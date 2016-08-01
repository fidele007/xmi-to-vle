#include <iostream>
#include <fstream>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>

struct Port
{
    std::string name;
};

struct Point
{
    std::string modelName;
    std::string portName;
};

struct Connection
{
    std::string type;
    std::string name;
    Point origin;
    Point destination;
};

struct Model
{
    std::string id;
    std::string name;
    std::string type;
    std::string conditions;
    std::string dynamics;
    std::vector<Model> submodels;
    std::vector<Port> inPorts;
    std::vector<Port> outPorts;
    std::vector<Connection> connections;
};

struct Experiment
{

};

struct VLEProject
{
    float version;
    std::string date;
    std::string author;
    Model model;
    Experiment experiment;
};

void write(VLEProject vle_project, const std::string &filename)
{
    using boost::property_tree::ptree;
    ptree pt;
    Model mainModel = vle_project.model;

    // Root
    ptree& rootNode = pt.add("vle_project", "");
    rootNode.put("<xmlattr>.author", "");
    rootNode.put("<xmlattr>.date", vle_project.date);
    rootNode.put("<xmlattr>.version", "1.0");

    // Structures
    ptree& structureNode = rootNode.put("structures", "");
    ptree& mainModelNode = structureNode.put("model", "");
    mainModelNode.put("<xmlattr>.name", mainModel.name);
    mainModelNode.put("<xmlattr>.type", "");
    mainModelNode.put("<xmlattr>.dynamics", "");

    // Submodels & Port list
    ptree& submodelNode = mainModelNode.put("submodels", "");
    BOOST_FOREACH(Model submodel, mainModel.submodels) {
        ptree& modelNode = submodelNode.add("model", "");
        modelNode.put("<xmlattr>.name", submodel.name);

        ptree& inNode = modelNode.put("in", "");
        BOOST_FOREACH(Port inPort, submodel.inPorts) {
            ptree& inPortNode = inNode.add("port", "");
            inPortNode.put("<xmlattr>.name", inPort.name);
        }

        ptree& outNode = modelNode.put("out", "");
        BOOST_FOREACH(Port outPort, submodel.outPorts) {
            ptree& outPortNode = outNode.add("port", "");
            outPortNode.put("<xmlattr>.name", outPort.name);
        }
    }

    // Connections
    ptree& connectionsNode = mainModelNode.put("connections", "");
    BOOST_FOREACH(Connection con, mainModel.connections) {
        ptree& connectionNode = connectionsNode.add("connection", "");
        connectionNode.put("<xmlattr>.type", con.type);

        ptree& originNode = connectionNode.put("origin", "");
        originNode.put("<xmlattr>.model", con.origin.modelName);
        originNode.put("<xmlattr>.port", con.origin.portName);

        ptree& destNode = connectionNode.put("destination", "");
        destNode.put("<xmlattr>.model", con.destination.modelName);
        destNode.put("<xmlattr>.port", con.destination.portName);
    }

    // Experiment
    ptree& experimentNode = rootNode.put("experiment", "");
    experimentNode.put("<xmlattr>.name", "test");
    experimentNode.put("<xmlattr>.seed", "123456789");

    ptree& conditionsNode = experimentNode.put("conditions", "");
    ptree& conditionNode = conditionsNode.put("condition", "");
    conditionNode.put("<xmlattr>.name", "simulation_engine");

    ptree& conditionPortNode = conditionNode.add("port", "");
    conditionPortNode.put("<xmlattr>.name", "begin");
    conditionPortNode.put("double", 0.0);

    ptree& anotherConditionPortNode = conditionNode.add("port", "");
    anotherConditionPortNode.put("<xmlattr>.name", "duration");
    anotherConditionPortNode.put("double", 1.0);

    ptree& viewsNode = experimentNode.add("views.outputs", "");
    ptree& outputNode = viewsNode.add("output", "");
    outputNode.put("<xmlattr>.format", "local");
    outputNode.put("<xmlattr>.location", "");
    outputNode.put("<xmlattr>.name", "view");
    outputNode.put("<xmlattr>.package", "vle.output");
    outputNode.put("<xmlattr>.plugin", "storage");

    ptree& mapNode = outputNode.put("map", "");
    ptree& keyNodeColumns = mapNode.add("key", "");
    keyNodeColumns.put("<xmlattr>.name", "columns");
    keyNodeColumns.add("integer", 15);

    ptree& keyNodeHeader = mapNode.add("key", "");
    keyNodeHeader.put("<xmlattr>.name", "header");
    keyNodeHeader.put("string", "top");

    ptree& keyNodeIncColumns = mapNode.add("key", "");
    keyNodeIncColumns.put("<xmlattr>.name", "inc_columns");
    keyNodeIncColumns.put("integer", 10);

    ptree& keyNodeIncRows = mapNode.add("key", "");
    keyNodeIncRows.put("<xmlattr>.name", "inc_rows");
    keyNodeIncRows.put("integer", 10);

    ptree& keyNodeRows = mapNode.add("key", "");
    keyNodeRows.put("<xmlattr>.name", "rows");
    keyNodeRows.put("integer", 15);

    ptree& viewNode = viewsNode.add("view", "");
    viewNode.put("<xmlattr>.name", "view");
    viewNode.put("<xmlattr>.output", "view");
    viewNode.put("<xmlattr>.timestep", 1);
    viewNode.put("<xmlattr>.type", "timed");

    viewsNode.add("observables", "");

    write_xml(filename, pt);
}

bool isModelID(const std::string& id, const Model& model) {
    return model.id == id;
}

int getModelIndexFromID(std::vector<Model> submodels, const std::string id) {
    std::vector<Model>::iterator it = find_if(submodels.begin(), submodels.end(),
                        boost::bind(&isModelID, id, boost::placeholders::_1));
    if (it != submodels.end()) {
        return std::distance(submodels.begin(), it);
    }

    return -1;
}

VLEProject read(std::istream& is)
{
    using boost::property_tree::ptree;
    ptree pt;
    read_xml(is, pt);

    VLEProject vle_project;
    Model mainModel = vle_project.model;

    BOOST_FOREACH(ptree::value_type const& v, pt.get_child("XMI")) {
        if (v.first == "<xmlattr>") {
            const ptree& root = pt.get_child("XMI");
            vle_project.date = root.get<std::string>("<xmlattr>.timestamp");
        } else if (v.first == "XMI.content") {
            mainModel.name =
                v.second.get<std::string>("UML:Model.<xmlattr>.name");
        }
    }

    typedef ptree::path_type path;
    using namespace std;
    string eltPath = "XMI/XMI.content/UML:Model/UML:Namespace.ownedElement";
    const ptree& eltTree = pt.get_child(path(eltPath, '/'));

    BOOST_FOREACH(ptree::value_type const& elt, eltTree) {
        if (elt.first == "UML:Collaboration") {
            // in sequence or collaboration diagram
            const ptree& roleTree =
                elt.second.get_child(path("UML:Namespace.ownedElement", '/'));

            // Get submodels
            BOOST_FOREACH(ptree::value_type const& role, roleTree) {
                if (role.first == "UML:ClassifierRole") {
                    string idPath = "<xmlattr>/xmi.id";
                    Model submodel;
                    submodel.name = role.second.get("<xmlattr>.name", "");
                    submodel.id = role.second.get(path(idPath, '/'), "");
                    mainModel.submodels.push_back(submodel);
                }
            }

            // Get messages and connections
            string mesPath = "UML:Collaboration.interaction/UML:Interaction/"
                             "UML:Interaction.message";
            const ptree& mesTree =
                elt.second.get_child(path(mesPath, '/'));
            Model submodels;
            BOOST_FOREACH(ptree::value_type const& mes, mesTree) {
                if (mes.first == "UML:Message") {
                    Connection con;
                    con.name = mes.second.get("<xmlattr>.name", "");
                    con.type = "internal";

                    string senderPath = "UML:Message.sender/UML:ClassifierRole/"
                                        "<xmlattr>/xmi.idref";
                    string origID = mes.second.get(path(senderPath, '/'), "");
                    int origIndex = getModelIndexFromID(mainModel.submodels, origID);
                    if (origIndex == -1) {
                        cout << "Model ID " << origID << " not found" << endl;
                        break;
                    }
                    Model origModel = mainModel.submodels[origIndex];
                    con.origin.modelName = origModel.name;
                    con.origin.portName = con.name + ".out";
                    Port outPort;
                    outPort.name = con.origin.portName;
                    origModel.outPorts.push_back(outPort);
                    mainModel.submodels[origIndex] = origModel;

                    string rcvPath = "UML:Message.receiver/UML:ClassifierRole/"
                                     "<xmlattr>/xmi.idref";
                    string destID = mes.second.get(path(rcvPath, '/'), "");
                    int destIndex = getModelIndexFromID(mainModel.submodels, destID);
                    if (destIndex == -1) {
                        cout << "Model ID " << destID << " not found" << endl;
                        break;
                    }
                    Model destModel = mainModel.submodels[destIndex];
                    con.destination.modelName = destModel.name;
                    con.destination.portName = con.name + ".in";
                    Port inPort;
                    inPort.name = con.destination.portName;
                    destModel.inPorts.push_back(inPort);
                    mainModel.submodels[destIndex] = destModel;

                    mainModel.connections.push_back(con);
                }
            }
        }
    }

    vle_project.model = mainModel;
    return vle_project;
}

int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " SOURCE_FILE" <<
            " DESTINATION_FILE" << std::endl;

        return 1;
    }

    std::ifstream input(argv[1]);
    VLEProject vle_project = read(input);

    write(vle_project, argv[2]);

    std::cout << "Conversion succeeded." << std::endl;

    return 0;
}
