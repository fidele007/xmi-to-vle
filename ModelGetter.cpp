#include "VPZStructure.hpp"
#include "ModelGetter.hpp"
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>

static const bool isModelID(const string &id, const Model &model) 
{
    return model.id == id;
}

const int getModelIndexFromID(vector<Model> submodels, const string id)
{
    vector<Model>::iterator it = find_if(submodels.begin(), submodels.end(),
                                         boost::bind(&isModelID, id, _1));
    if (it != submodels.end())
        return distance(submodels.begin(), it);

    return -1;
}

static const bool isModelIDRef(const string &idRef, const Model &model) 
{
    BOOST_FOREACH(const string &id, model.idRef) {
        if (id == idRef)
            return true;
    }

    return false;
}

const int getModelIndexFromIDRef(vector<Model> submodels, const string idRef) 
{
    vector<Model>::iterator it = find_if(submodels.begin(), submodels.end(),
                                         boost::bind(&isModelIDRef, idRef, _1));
    if (it != submodels.end())
        return distance(submodels.begin(), it);

    return -1;
}

static const bool isModelName(const string &modelName, const ptree &model)
{
    return model.get("<xmlattr>.name", "") == modelName;
}

const int getCoupledModelIndex(vector<ptree> allModels, const string modelName)
{
    vector<ptree>::iterator it = find_if(allModels.begin(), allModels.end(),
                                         boost::bind(&isModelName, modelName, _1));
    if (it != allModels.end())
        return distance(allModels.begin(), it);

    return -1; 
}
