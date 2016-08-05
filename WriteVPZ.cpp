#include "VPZStructure.hpp"
#include "WriteVPZ.hpp"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>

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

    // Dynamics
    mainModelNode.put("dynamics", "");

    // Connections
    ptree& connectionsNode = mainModelNode.put("connections", "");
    BOOST_FOREACH(Connection con, mainModel.connections) {
        ptree& connectionNode = connectionsNode.add("connection", "");
        std::string connectionType;
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
