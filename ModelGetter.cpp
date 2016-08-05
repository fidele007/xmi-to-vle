#include "VPZStructure.hpp"
#include "ModelGetter.hpp"
#include <algorithm>
#include <boost/bind.hpp>

bool isModelID(const std::string& id, const Model& model) {
    return model.id == id;
}

int getModelIndexFromID(std::vector<Model> submodels, const std::string id) {
    std::vector<Model>::iterator it = find_if(submodels.begin(), submodels.end(),
                        boost::bind(&isModelID, id, _1));
    if (it != submodels.end()) {
        return std::distance(submodels.begin(), it);
    }

    return -1;
}
