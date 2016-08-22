#include <boost/property_tree/ptree.hpp>

using namespace std;
using boost::property_tree::ptree;

const int getModelIndexFromID(vector<Model> submodels, const string id);
const int getModelIndexFromIDRef(vector<Model> submodels, const string idRef);
const int getCoupledModelIndex(vector<ptree> allModels, const string modelName);
