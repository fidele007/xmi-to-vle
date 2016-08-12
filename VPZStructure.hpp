#include <iostream>
#include <fstream>
#include <vector>

enum ModelType {
    MT_atomic,
    MT_coupled
};

enum ConnectionType {
    CT_input,
    CT_output,
    CT_internal
};

enum OutputFormat {
    OF_local,
    OF_distant
};

enum ViewType {
    VT_timed,
    VT_event,
    VT_internal,
    VT_external,
    VT_confluent,
    VT_output,
    VT_finish
};

enum ExperimentCombination {
    EC_linear,
    EC_total
};

struct AttachedView 
{
    std::string name;
};

struct Port
{
    std::string name;
    std::vector<AttachedView> attachedViews;
};

// Common struct for origin and destination
struct Point
{
    std::string modelName;
    std::string portName;
};

struct Connection
{
    ConnectionType type;
    std::string name;
    Point origin;
    Point destination;
};

struct View
{
    std::string name;
    ViewType type;
    std::string output;
    std::string timestep;
};

struct Observable
{
    std::string observable;
};

struct Output
{
    std::string name;
    OutputFormat format;
    std::string location;
    std::string package;
    std::string plugin;
};

struct Views
{
    std::vector<Output> outputs;
    std::vector<Observable> observables;
    std::vector<View> subviews;
};

struct Condition
{
    std::string name;
    std::vector<Port> ports;
};

struct Experiment 
{
    std::string name;
    ExperimentCombination combination;
    std::vector<Condition> conditions;
    Views views;
};

struct Model
{
    std::string id;
    std::vector<std::string> idRef;
    std::string name;
    ModelType type;
    std::string dynamics;
    std::string conditions;
    std::string observables;
    std::vector<Port> inPorts;
    std::vector<Port> outPorts;
    std::vector<Model> submodels;
    std::vector<Connection> connections;
};

struct Class
{
    std::string name;
    Model model;
};

struct Dynamic
{
    std::string name;
    std::string library;
    std::string package;
    std::string location;
    std::string type;
    std::string language;
};

struct VLEProject
{
    std::string date;
    std::string version;
    std::string instance;
    std::string replica;
    std::string author;
    Model model;
    std::vector<Dynamic> dynamics;
    std::vector<Class> classes;
    Experiment experiment;
};
