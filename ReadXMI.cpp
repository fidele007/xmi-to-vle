#include "VPZStructure.hpp"
#include "ModelGetter.hpp"
#include "ReadXMI.hpp"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

vector<string> split(string data, string token)
{
    vector<string> output;
    size_t pos = string::npos;
    do
    {
        pos = data.find(token);
        output.push_back(data.substr(0, pos));
        if (string::npos != pos)
            data = data.substr(pos + token.size());
    } while (string::npos != pos);

    return output;
}

VLEProject read(std::istream& is)
{
    using boost::property_tree::ptree;

    ptree pt;
    read_xml(is, pt);

    VLEProject vle_project;
    Model mainModel = vle_project.model;
    
    const ptree& root = pt.get_child("uml:Model");
    mainModel.name = root.get<string>("<xmlattr>.name");
    mainModel.type = MT_coupled;

    const ptree& eltTree = root.get_child("packagedElement");
    string diagramType = eltTree.get<string>("<xmlattr>.xmi:type");
    if (diagramType != "uml:Interaction") {
        cout << "XMI is not of a sequence a diagram. Abort." << endl;
        exit (EXIT_FAILURE);
    }

    BOOST_FOREACH(ptree::value_type const& child, eltTree) {
        if (child.first == "lifeline") {
            Model submodel;
            string fullModelName = child.second.get<string>("<xmlattr>.name");
            size_t colonPos = fullModelName.find(":");
            if (colonPos == string::npos)
                continue;

            string modelType = fullModelName.substr(0, colonPos);
            if (boost::iequals(modelType, "atomic"))
                submodel.type = MT_atomic;
            else if (boost::iequals(modelType, "coupled"))
                submodel.type = MT_coupled; //TODO: Go to another XMI
            else
                continue;

            submodel.name = fullModelName.substr(colonPos + 1);
            submodel.id = child.second.get<string>("<xmlattr>.xmi:id");
            string idRef = child.second.get<string>("<xmlattr>.coveredBy");
            submodel.idRef = split(idRef, " ");

            mainModel.submodels.push_back(submodel);
        } else if (child.first == "message") {
            Connection con;
            con.name = child.second.get("<xmlattr>.name", "");
            con.type = CT_internal;

            string origID = child.second.get<string>("<xmlattr>.sendEvent");
            int origIndex = getModelIndexFromIDRef(mainModel.submodels, origID);
            if (origIndex == -1) {
                cout << "Model with IDRef " << origID << " not found" << endl;
                continue;
            }
            Model origModel = mainModel.submodels[origIndex];
            con.origin.modelName = origModel.name;
            con.origin.portName = con.name + ".out";
            Port outPort;
            outPort.name = con.origin.portName;
            origModel.outPorts.push_back(outPort);
            mainModel.submodels[origIndex] = origModel;

            string destID = child.second.get<string>("<xmlattr>.receiveEvent");
            int destIndex = getModelIndexFromIDRef(mainModel.submodels, destID);
            if (destIndex == -1) {
                cout << "Model with IDRef " << destID << " not found" << endl;
                continue;
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

    vle_project.model = mainModel;
    return vle_project;
}
