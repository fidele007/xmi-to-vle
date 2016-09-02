#include "VPZStructure.hpp"
#include "WriteCPP.hpp"
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <iterator>

using namespace std;
using boost::filesystem::path;

static void writeModelToCPP(string fileContent, 
                            const Model model, 
                            const path srcPath)
{
    BOOST_FOREACH(Model submodel, model.submodels) {
        path submodelPath = srcPath;
        submodelPath = submodelPath.append(submodel.name + ".cpp");
        std::ofstream out(submodelPath.string());
        string submodelContent = boost::replace_all_copy(fileContent,
                                                         "Simple",
                                                         submodel.name);
        string sigmaFunc;
        map<string, string> taskMap = submodel.taskDuration;
        if (!taskMap.empty()) {
            sigmaFunc.append("if (phase_is(\"");
            sigmaFunc.append(taskMap.begin()->first);
            sigmaFunc.append("\") { return ");
            sigmaFunc.append(taskMap.begin()->second);
            sigmaFunc.append("; } ");

            map<string, string>::iterator it;
            for (it = std::next(taskMap.begin()); it != taskMap.end(); ++it) {
                sigmaFunc.append("else if (phase_is(\"");
                sigmaFunc.append(it->first);
                sigmaFunc.append("\") { return ");
                sigmaFunc.append(it->second);
                sigmaFunc.append("; }");
            }
            cout << submodel.name << ":\n" << sigmaFunc << endl;
        }

        out << submodelContent;
        out.close();

        if (!submodel.submodels.empty()) {
            writeModelToCPP(fileContent, submodel, srcPath);
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

    writeModelToCPP(fileContent, project.model, srcPath);
}
