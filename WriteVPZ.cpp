#include "VPZStructure.hpp"
#include "WriteVPZ.hpp"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>
#include <ctime>

using namespace std;
using boost::property_tree::ptree;

static void addModels(Model model, ptree& modelNode)
{
    ptree& submodelNode = modelNode.put("submodels", "");
    BOOST_FOREACH(Model submodel, model.submodels) {
        ptree& aModel = submodelNode.add("model", "");
        aModel.put("<xmlattr>.name", submodel.name);
        string modelType;
        switch (submodel.type) {
            case MT_atomic:
                modelType = "atomic";
                break;
            case MT_coupled:
                modelType = "coupled";
                break;
        }
        aModel.put("<xmlattr>.type", modelType);

        ptree& inNode = aModel.put("in", "");
        BOOST_FOREACH(Port inPort, submodel.inPorts) {
            ptree& inPortNode = inNode.add("port", "");
            inPortNode.put("<xmlattr>.name", inPort.name);
        }

        ptree& outNode = aModel.put("out", "");
        BOOST_FOREACH(Port outPort, submodel.outPorts) {
            ptree& outPortNode = outNode.add("port", "");
            outPortNode.put("<xmlattr>.name", outPort.name);
        }

        if (!submodel.submodels.empty())
            addModels(submodel, aModel);
    }
}

static void addConnections(Model model, ptree& connectionsNode)
{
    BOOST_FOREACH(Connection con, model.connections) {
        ptree& connectionNode = connectionsNode.add("connection", "");
        string connectionType;
        switch (con.type) {
            case CT_input:
                connectionType = "input";
                break;
            case CT_output:
                connectionType = "output";
                break;
            case CT_internal:
                connectionType = "internal";
                break;
        }
        connectionNode.put("<xmlattr>.type", connectionType);

        ptree& originNode = connectionNode.put("origin", "");
        originNode.put("<xmlattr>.model", con.origin.modelName);
        originNode.put("<xmlattr>.port", con.origin.portName);

        ptree& destNode = connectionNode.put("destination", "");
        destNode.put("<xmlattr>.model", con.destination.modelName);
        destNode.put("<xmlattr>.port", con.destination.portName);
    }
    
    if (model.submodels.empty())
        return;

    BOOST_FOREACH(Model submodel, model.submodels) {
        addConnections(submodel, connectionsNode);
    }
}

void write(VLEProject vle_project, const string &filename)
{
    ptree pt;
    Model mainModel = vle_project.model;

    // Root
    ptree& rootNode = pt.add("vle_project", "");
    rootNode.put("<xmlattr>.author", "");
    time_t now = time(0);
    rootNode.put("<xmlattr>.date", ctime(&now));
    rootNode.put("<xmlattr>.version", "1.0");

    // Structures
    ptree& structureNode = rootNode.put("structures", "");
    ptree& mainModelNode = structureNode.put("model", "");
    mainModelNode.put("<xmlattr>.name", mainModel.name);
    mainModelNode.put("<xmlattr>.type", "coupled");
    mainModelNode.put("<xmlattr>.dynamics", "");

    // Submodels & Port list
    addModels(mainModel, mainModelNode);

    // Dynamics
    mainModelNode.put("dynamics", "");

    // Connections
    ptree& connectionsNode = mainModelNode.put("connections", "");
    addConnections(mainModel, connectionsNode);

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
