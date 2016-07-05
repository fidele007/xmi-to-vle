#include <iostream>
#include <fstream>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>

struct Port
{
    std::string name;
};

struct In 
{
    Port port[50]; // to be modified
};

struct Out
{
    Port port[50]; // to be modified
};

struct Origin
{
    std::string model;
    std::string port;
};

struct Destination
{
    std::string model;
    std::string port;
};

struct Connection
{
    Origin origin;
    Destination destination;
};

struct Connections
{
    Connection connection[50];
};

struct Model 
{
    std::string name;
    std::string type;
    std::string conditions;
    std::string dynamics;
    struct Model *submodel;
    In in;
    Out out;
    Connections connections;
};

struct Structures
{
    Model model;
};

struct Experiment
{

};

struct VleProject
{
    float version;
    std::string date;
    std::string author;
    Structures structures;
    Experiment experiment;
};

typedef std::vector<VleProject> Vle;

void write(Vle vle_project, const std::string &filename)
{
    using boost::property_tree::ptree;
    ptree pt;

    // Root
    ptree& rootNode = pt.add("vle_project", "");
    rootNode.put("<xmlattr>.author", "Gauthier Quesnel");
    rootNode.put("<xmlattr>.date", "");
    rootNode.put("<xmlattr>.version", "1.0");
    
    // Structures    
    ptree& structureNode = rootNode.put("structures", "");
    ptree& modelNode = structureNode.put("model", "");
    modelNode.put("<xmlattr>.name", "");
    modelNode.put("<xmlattr>.type", "");
    modelNode.put("<xmlattr>.dynamics", "");
    
    ptree& inNode = modelNode.put("in", "");
    ptree& inPortNode = inNode.add("port", "");
    inPortNode.put("<xmlattr>.name", "inPort");
    
    ptree& outNode = modelNode.put("out", "");
    ptree& outPortNode = outNode.add("port", "");
    outPortNode.put("<xmlattr>.name", "outPort");

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

Vle read(std::istream& is)
{
    using boost::property_tree::ptree;
    ptree pt;
    read_xml(is, pt);
    Vle res;

    // read xmi file here

    return res;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " PATH_TO_FILE" << std::endl;
        return 1;
    }
    std::ifstream input("test.xml");
    Vle res = read(input);

    write(res, "output.vpz");

    std::cout << "Conversion succeeded." << std::endl;

    return 0;
}

