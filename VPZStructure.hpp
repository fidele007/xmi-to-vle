#include <iostream>
#include <fstream>
#include <vector>
#include <map>

using namespace std;

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
    string name;
};

struct Port
{
    string id;
    string modelName;
    string name;
};

struct Connection
{
    ConnectionType type;
    string name;
    Port origin;
    Port destination;
    string id;
};

struct State
{
    string id;
    string name;
    string duration;
    string start;
    string finish;
    Port inPort;
    Port outPort;
};

struct Guard
{
    string type;
    string value;
    vector<string> idList;
    vector<Connection> connections;
    vector<State> states;
};

struct View
{
    string name;
    ViewType type;
    string output;
    string timestep;
};

struct Observable
{
    string observable;
};

struct Output
{
    string name;
    OutputFormat format;
    string location;
    string package;
    string plugin;
};

struct Views
{
    vector<Output> outputs;
    vector<Observable> observables;
    vector<View> subviews;
};

struct Condition
{
    string name;
    vector<Port> ports;
};

struct Experiment 
{
    string name;
    ExperimentCombination combination;
    vector<Condition> conditions;
    Views views;
};

struct Model
{
    string id;
    vector<string> idRef;
    string name;
    ModelType type;
    string dynamics;
    string conditions;
    string observables;
    vector<Port> inPorts;
    vector<Port> outPorts;
    vector<Model> submodels;
    vector<Connection> connections;
    vector<Guard> guards;
    vector<State> states;
};

struct Class
{
    string name;
    Model model;
};

struct Dynamic
{
    string name;
    string library;
    string package;
    string location;
    string type;
    string language;
};

struct VLEProject
{
    string date;
    string version;
    string instance;
    string replica;
    string author;
    Model model;
    vector<Dynamic> dynamics;
    vector<Class> classes;
    Experiment experiment;
};
