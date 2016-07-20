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
    std::string model;
    std::string port;
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

    ptree& submodelNode = mainModelNode.put("submodels", "");
    BOOST_FOREACH(Model submodel, mainModel.submodels) {
        ptree& modelNode = submodelNode.add("model", "");
        modelNode.put("<xmlattr>.name", submodel.name);

        ptree& inNode = modelNode.put("in", "");
        ptree& inPortNode = inNode.add("port", "");
        inPortNode.put("<xmlattr>.name", "in");
        
        ptree& outNode = modelNode.put("out", "");
        ptree& outPortNode = outNode.add("port", "");
        outPortNode.put("<xmlattr>.name", "out");
    }

    ptree& connectionsNode = mainModelNode.put("connections", "");
    BOOST_FOREACH(Connection con, mainModel.connections) {
        ptree& connectionNode = connectionsNode.add("connection", "");
        connectionNode.put("<xmlattr>.type", con.type);

        ptree& originNode = connectionNode.put("origin", "");
        originNode.put("<xmlattr>.model", con.origin.model);
        originNode.put("<xmlattr>.port", con.origin.port);

        ptree& destNode = connectionNode.put("destination", "");
        destNode.put("<xmlattr>.model", con.destination.model);
        destNode.put("<xmlattr>.port", con.destination.port);
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

const std::string getModelNameFromID(std::vector<Model> submodels, 
                                     const std::string id) 
{
    BOOST_FOREACH(Model m, submodels) {
        if (id == m.id) {
            return m.name;
        } else if (!m.submodels.empty()) {
            std::string res = getModelNameFromID(m.submodels, id);
            if (res != "")
                return res;
            else
                continue;
        }
    }

    return "";
}

VLEProject read(std::istream& is)
{
    using boost::property_tree::ptree;
    ptree pt;
    read_xml(is, pt);
    
    VLEProject vle_project;

    BOOST_FOREACH(ptree::value_type const& v, pt.get_child("XMI")) {
        if (v.first == "<xmlattr>") {
            const ptree& root = pt.get_child("XMI");
            vle_project.date = root.get<std::string>("<xmlattr>.timestamp");
        } else if (v.first == "XMI.content") {
            vle_project.model.name = 
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
            
            BOOST_FOREACH(ptree::value_type const& role, roleTree) {
                if (role.first == "UML:ClassifierRole") {
                    string idPath = "<xmlattr>/xmi.id";
                    Model submodel;
                    submodel.name = role.second.get("<xmlattr>.name", "");
                    submodel.id = role.second.get(path(idPath, '/'), "");
                    vle_project.model.submodels.push_back(submodel);
                }
            }

            string mesPath = "UML:Collaboration.interaction/UML:Interaction/"
                             "UML:Interaction.message";
            const ptree& mesTree = 
                elt.second.get_child(path(mesPath, '/'));
            BOOST_FOREACH(ptree::value_type const& mes, mesTree) {
                if (mes.first == "UML:Message") {
                    Connection con;
                    con.name = mes.second.get("<xmlattr>.name", "");
                    con.type = "";

                    string senderPath = "UML:Message.sender/UML:ClassifierRole/"
                                        "<xmlattr>/xmi.idref";
                    string origId = mes.second.get(path(senderPath, '/'), "");
                    con.origin.model = 
                        getModelNameFromID(vle_project.model.submodels, origId);
                    con.origin.port = "out";

                    string rcvPath = "UML:Message.receiver/UML:ClassifierRole/"
                                     "<xmlattr>/xmi.idref";
                    string destID = mes.second.get(path(rcvPath, '/'), "");
                    con.destination.model = 
                        getModelNameFromID(vle_project.model.submodels, destID);
                    con.destination.port = "in";

                    vle_project.model.connections.push_back(con);
                }
            }
        }
    }

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

