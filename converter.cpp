// converter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
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
    rootNode.put("<xmlattr>.author", "");
    rootNode.put("<xmlattr>.date", "");
    rootNode.put("<xmlattr>.version", "");
    
    // Structures    
    ptree& structureNode = rootNode.put("structures", "");
    ptree& modelNode = structureNode.put("model", "");
    modelNode.put("<xmlattr>.name", "");
    modelNode.put("<xmlattr>.type", "");
    modelNode.put("<xmlattr>.dynamics", "");
    
    ptree& inNode = modelNode.put("in", "");
    for (int i = 0; i < 5; i++) {
        ptree& portNode = inNode.add("port", "");
        std::string portName = "in";
        portNode.put("<xmlattr>.name", portName);
    }
    
    ptree& outNode = modelNode.put("out", "");
    for (int i = 0; i < 5; i++) {
        ptree& portNode = outNode.add("port", "");
        std::string portName = "out";
        portNode.put("<xmlattr>.name", portName);
    }

    // Experiment
    ptree& experimentNode = rootNode.put("experiment", "");
    experimentNode.put("<xmlattr>.name", "");
    experimentNode.put("<xmlattr>.seed", "");

    ptree& conditionsNode = experimentNode.put("conditions", "");
    ptree& conditionNode = conditionsNode.put("condition", "");
    conditionNode.put("<xmlattr>.name", "");
    
    for (int i = 0; i < 5; i++) {
        ptree& portNode = conditionNode.add("port", "");
        std::string portName = "port";
        portNode.put("<xmlattr>.name", portName);
        portNode.put("double", i);
    }
    
    experimentNode.add("views", "");

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

