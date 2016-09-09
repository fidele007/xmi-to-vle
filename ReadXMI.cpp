#include "VPZStructure.hpp"
#include "ModelGetter.hpp"
#include "ReadXMI.hpp"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using boost::property_tree::ptree;

static vector<string> split(string data, string token)
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

static void getStatesForModel(const ptree &modelTree, Model &model)
{
    vector<Model> &submodels = model.submodels;
    BOOST_FOREACH(const ptree::value_type &child, modelTree) {
        if (child.first != "fragment")
            continue;

        string fragType = child.second.get("<xmlattr>.xmi:type", "");
        if (fragType == "uml:CombinedFragment") {
            BOOST_FOREACH(const ptree::value_type &op, child.second) {
                if (op.first == "operand")
                    getStatesForModel(op.second, model);
            }
        } else if (fragType == "uml:BehaviorExecutionSpecification") {
            string stateID = child.second.get("<xmlattr>.xmi:id", "");
            string start = child.second.get("<xmlattr>.start", "");
            string finish = child.second.get("<xmlattr>.finish", "");
            BOOST_FOREACH(const ptree::value_type &com, child.second) {
                if (com.first != "ownedComment")
                    continue;

                string stateComment = com.second.get("body", "");
                if (stateComment.empty())
                    continue;

                string modelID = child.second.get<string>("<xmlattr>.covered");
                int modelInd = getModelIndexFromID(submodels, modelID);
                if (modelInd == -1) {
                    cerr << "WARNING: Model with ID "
                         << modelID
                         << " not found. State is skipped."
                         << endl;

                    continue;
                }

                vector<State> &states = submodels[modelInd].states;
                vector<string> stateString = split(stateComment, "/time=");

                State aState;
                aState.id = stateID;
                aState.name = stateString[0];
                aState.duration = stateString[1];
                aState.start = start;
                aState.finish = finish;

                states.push_back(aState);
            }
        }
    }
}

static void getPortsForStates(Model &model)
{
    if (model.submodels.empty())
        return;

    BOOST_FOREACH(Model &submodel, model.submodels) {
        BOOST_FOREACH(State &state, submodel.states) {
            BOOST_FOREACH(Connection con, model.connections) {
                if (state.start == con.origin.id ||
                    state.finish == con.origin.id)
                {
                    state.outPort = con.origin;
                }
                else if (state.start == con.destination.id ||
                         state.finish == con.destination.id)
                {
                    state.inPort = con.destination;
                }
            }
        }
    }
}

static void getGuardsForModel(const ptree &modelTree, Model &model)
{
    vector<Guard> &guards = model.guards;
    BOOST_FOREACH(const ptree::value_type &child, modelTree) {
        if (child.first != "fragment")
            continue;

        string fragType = child.second.get("<xmlattr>.xmi:type", "");

        if (fragType != "uml:CombinedFragment")
            continue;

        string combinedFragType =
            child.second.get<string>("<xmlattr>.interactionOperator");

        BOOST_FOREACH(const ptree::value_type &frag, child.second) {
            if (frag.first != "operand")
                continue;

            const ptree &guardSpec =
                frag.second.get_child("guard.specification");
            string value = guardSpec.get<string>("<xmlattr>.value");
            // If no guard value is specified, set it to always true
            if (value.empty())
                value = "1";

            Guard aGuard;
            aGuard.value = value;
            aGuard.type = combinedFragType;

            BOOST_FOREACH(const ptree::value_type &subFrag, frag.second) {
                if (subFrag.first != "fragment")
                    continue;

                string subFragType =
                    subFrag.second.get<string>("<xmlattr>.xmi:type");

                if (subFragType == "uml:CombinedFragment") {
                    getGuardsForModel(frag.second, model);
                } else if (subFragType == "uml:MessageOccurrenceSpecification") {
                    string associatedId =
                        subFrag.second.get<string>("<xmlattr>.message");

                    aGuard.idList.push_back(associatedId);
                } else if (subFragType == "uml:BehaviorExecutionSpecification") {
                    string associatedId =
                        subFrag.second.get<string>("<xmlattr>.xmi:id");

                    aGuard.idList.push_back(associatedId);
                } else {
                    continue;
                }
            }

            guards.push_back(aGuard);
        }
    }
}

static void getConnectionsForGuards(Model &model) {
    if (model.guards.empty() || model.connections.empty())
        return;

    BOOST_FOREACH(Connection con, model.connections) {
        BOOST_FOREACH(Guard &guard, model.guards) {
            BOOST_FOREACH(string id, guard.idList) {
                if (con.id == id) {
                    guard.connections.push_back(con);
                    break;
                }
            }
        }
    }
}

static Model readModel(const ptree &modelTree, 
                       const vector<ptree> allModels,
                       const bool isMainModel)
{
    Model model;
    model.name = modelTree.get<string>("<xmlattr>.name");
    model.type = MT_coupled;

    const string diagramType = modelTree.get<string>("<xmlattr>.xmi:type");
    if (diagramType != "uml:Interaction") {
        if (isMainModel) {
            cerr << "ERROR: Model " 
                 << model.name 
                 << " is not of a sequence diagram."
                 << " Abort."
                 << endl;
            exit (EXIT_FAILURE);
        } else {
            cerr << "WARNING: " 
                 << model.name
                 << " is not of a sequence diagram."
                 << " File is skipped." 
                 << endl;
            return model;
        }
    }
    
    // This is for automatically naming connections with no name
    int conCount = 0; 

    BOOST_FOREACH(const ptree::value_type &child, modelTree) {
        if (child.first == "lifeline") {
            Model submodel;
            string fullModelName = child.second.get<string>("<xmlattr>.name");
            size_t colonPos = fullModelName.find(":");
            if (colonPos == string::npos) {
                cerr << "WARNING: Type of model "
                     << fullModelName 
                     << " is not specified. Model's name in UML must follow"
                     << " the form MODEL_TYPE:MODEL_NAME. Model is skipped."
                     << endl;
                continue;                
            }

            string modelName = fullModelName.substr(colonPos + 1);
            string modelType = fullModelName.substr(0, colonPos);
            if (boost::iequals(modelType, "atomic")) {
                submodel.type = MT_atomic;
            } else if (boost::iequals(modelType, "coupled")) {
                submodel.type = MT_coupled;
                int submodelIndex = getCoupledModelIndex(allModels, modelName);
                if (submodelIndex == -1) {
                    cerr << "WARNING: Submodels of coupled model "
                         << modelName
                         << " not found and are therefore skipped."
                         << endl;
                } else {
                    submodel = readModel(allModels[submodelIndex], 
                                         allModels, 
                                         false);
                }
            } else {
                cerr << "WARNING: Type of model "
                     << modelName
                     << " is incorrect. Model's name in UML must follow"
                     << " the form MODEL_TYPE:MODEL_NAME. Model's type is set"
                     << " to \"atomic\" by default."
                     << endl;

                submodel.type = MT_atomic;
            }

            submodel.name = modelName;
            submodel.id = child.second.get<string>("<xmlattr>.xmi:id");
            string idRef = child.second.get<string>("<xmlattr>.coveredBy");
            submodel.idRef = split(idRef, " ");

            model.submodels.push_back(submodel);
        } else if (child.first == "message") {
            Connection con;
            con.id = child.second.get<string>("<xmlattr>.xmi:id");
            con.name = child.second.get("<xmlattr>.name", "");
            if (con.name.empty()) {
                // TODO: Automically add connection names
                conCount++;
                con.name = "p0rt" + to_string(conCount);
            }
            con.type = CT_internal;

            string origID = child.second.get<string>("<xmlattr>.sendEvent");
            int origIndex = getModelIndexFromIDRef(model.submodels, origID);
            if (origIndex == -1) {
                cerr << "WARNING: Model with reference ID " 
                     << origID 
                     << " not found. Origin model is skipped." 
                     << endl;

                continue;
            }

            Model origModel = model.submodels[origIndex];
            con.origin.id = origID;
            con.origin.modelName = origModel.name;
            con.origin.name = con.name + ".out";
            origModel.outPorts.push_back(con.origin);
            model.submodels[origIndex] = origModel;

            string destID = child.second.get<string>("<xmlattr>.receiveEvent");
            int destIndex = getModelIndexFromIDRef(model.submodels, destID);
            if (destIndex == -1) {
                cerr << "WARNING: Model with reference ID " 
                     << destID 
                     << " not found. Destination model is skipped."
                     << endl;

                continue;
            }

            Model destModel = model.submodels[destIndex];
            con.destination.id = destID;
            con.destination.modelName = destModel.name;
            con.destination.name = con.name + ".in";
            destModel.inPorts.push_back(con.destination);
            model.submodels[destIndex] = destModel;

            model.connections.push_back(con);
        }
    }

    getStatesForModel(modelTree, model);
    getPortsForStates(model);

    getGuardsForModel(modelTree, model);
    getConnectionsForGuards(model);

    return model;
}

VLEProject readXMI(const string file)
{
    VLEProject mainProject;

    ifstream input(file.c_str());
    if(input.fail()) {
        cerr << "ERROR: Can't access " 
            << file 
            << ". Abort." 
            << endl;
        exit (EXIT_FAILURE);
    }

    using boost::filesystem::path;
    path filePath = path(file).parent_path();

    ptree pt;
    read_xml(input, pt);

    const ptree &root = pt.get_child("uml:Model");
    vector<ptree> coupledModels;
    BOOST_FOREACH(const ptree::value_type &rootNode, root) {
        if (rootNode.first == "packagedElement") {
            coupledModels.push_back(rootNode.second);
        }
    }

    mainProject.model = readModel(coupledModels[0], coupledModels, true);
    return mainProject;
}
