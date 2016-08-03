#include "VPZStructure.hpp"
#include "ModelGetter.hpp"
#include "ReadXMI.hpp"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>

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
                    con.type = CT_internal;

                    string senderPath = "UML:Message.sender/UML:ClassifierRole/"
                                        "<xmlattr>/xmi.idref";
                    string origID = mes.second.get(path(senderPath, '/'), "");
                    int origIndex = getModelIndexFromID(mainModel.submodels, 
                                                        origID);
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
                    int destIndex = getModelIndexFromID(mainModel.submodels, 
                                                        destID);
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
