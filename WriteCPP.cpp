#include "VPZStructure.hpp"
#include "WriteCPP.hpp"
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/replace.hpp>

using namespace std;
using boost::filesystem::path;

static void writeModelToCPP(string fileContent, 
                            const Model model, 
                            const bool isMainModel,
                            const path srcPath)
{
    if (!isMainModel) {
        path coupledModelPath = srcPath;
        coupledModelPath = coupledModelPath.append(model.name + ".cpp");
        ofstream coupledOut(coupledModelPath.string());
        string modelContent = boost::replace_all_copy(fileContent,
                                                      "Simple",
                                                      model.name);
        coupledOut << modelContent;
        coupledOut.close();
    }

    BOOST_FOREACH(Model submodel, model.submodels) {
        path submodelPath = srcPath;
        submodelPath = submodelPath.append(submodel.name + ".cpp");
        std::ofstream out(submodelPath.string());
        string submodelContent = boost::replace_all_copy(fileContent,
                                                         "Simple",
                                                         submodel.name);
        out << submodelContent;
        out.close();

        if (!submodel.submodels.empty()) {
            writeModelToCPP(fileContent, submodel, false, srcPath);
        }
    }
}

void writeCPP(VLEProject project, const string &dirName)
{
    ifstream infile { "Simple.cpp" };
    string fileContent { istreambuf_iterator<char>(infile), 
                         istreambuf_iterator<char>() };

    path srcPath = path(dirName).append("src");
    if (!is_directory(srcPath))
        create_directory(srcPath);

    writeModelToCPP(fileContent, project.model, true, srcPath);
}
