#include <iostream>
#include <fstream>
#include <vector>

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

struct Experiment {};

struct VLEProject
{
    float version;
    std::string date;
    std::string author;
    Model model;
    Experiment experiment;
};