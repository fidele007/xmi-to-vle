#include "VPZStructure.hpp"
#include "ModelGetter.hpp"
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>

using namespace std;

bool isModelID(const string& id, const Model& model) 
{
    return model.id == id;
}

int getModelIndexFromID(vector<Model> submodels, const string id)
{
    vector<Model>::iterator it = find_if(submodels.begin(), submodels.end(),
                                         boost::bind(&isModelID, id, _1));
    if (it != submodels.end())
        return distance(submodels.begin(), it);

    return -1;
}

bool isModelIDRef(const string& idRef, const Model& model) 
{
    BOOST_FOREACH(const string& id, model.idRef) {
        if (id == idRef)
            return true;
    }

    return false;
}

int getModelIndexFromIDRef(vector<Model> submodels, const string idRef) 
{
    vector<Model>::iterator it = find_if(submodels.begin(), submodels.end(),
                                         boost::bind(&isModelIDRef, idRef, _1));
    if (it != submodels.end())
        return distance(submodels.begin(), it);

    return -1;
}
