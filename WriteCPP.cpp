#include "VPZStructure.hpp"
#include "WriteCPP.hpp"
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <iterator>

using namespace std;
using boost::filesystem::path;

static string writeSigmaFunction(map<string, string> stateMap)
{
    string sigmaFunc;
    if (!stateMap.empty()) {
        sigmaFunc = "    virtual vd::Time timeAdvance() const override\n"
                    "    {\n"
                    "        switch (modelPhase) {\n"
                    "        case ";
        sigmaFunc.append(stateMap.begin()->first);
        sigmaFunc.append(":\n            return ");
        sigmaFunc.append(stateMap.begin()->second);
        sigmaFunc.append(";\n");

        map<string, string>::iterator it;
        for (it = std::next(stateMap.begin()); it != stateMap.end(); ++it) {
            sigmaFunc.append("        case ");
            sigmaFunc.append(it->first);
            sigmaFunc.append(":\n            return ");
            sigmaFunc.append(it->second);
            sigmaFunc.append(";\n");
        }
        sigmaFunc.append("        default:\n");
        sigmaFunc.append("            return vd::infinity;\n");
        sigmaFunc.append("        };\n");
        sigmaFunc.append("    }");
    }

    return sigmaFunc;
}

static string writeStateEnum(map<string, string> stateMap)
{
    string stateEnum;
    if (!stateMap.empty()) {
        stateEnum.append("    enum PHASE {\n");
        stateEnum.append("        ");
        stateEnum.append(stateMap.begin()->first);

        if (stateMap.size() > 1)
            stateEnum.append(",");

        stateEnum.append("\n");

        map<string, string>::iterator it;
        for (it = std::next(stateMap.begin()); it != stateMap.end(); ++it) {
            stateEnum.append("        ");
            stateEnum.append(it->first);
            if (it != --stateMap.end())
                stateEnum.append(",");

            stateEnum.append("\n");
        }
        stateEnum.append("    };\n");
        stateEnum.append("    PHASE modelPhase;\n");
    }

    return stateEnum;
}

static string writeExternalTransition(const vector<Port> inPorts) {
    string extFunc;
    if (inPorts.empty())
        return extFunc;

    extFunc = "    virtual void externalTransition(\n"
              "            const vd::ExternalEventList& event,\n"
              "            vle::devs::Time /*time*/) override\n"
              "    {\n"
              "        vd::ExternalEventList::const_iterator it;\n"
              "        for (it = event.begin(); it != event.end(); ++it) {\n";
    vector<Port>::const_iterator it;
    for (it = inPorts.begin(); it != inPorts.end(); ++it) {
        if (it == inPorts.begin())
            extFunc.append("            if ((*it)->onPort(\"");
        else
            extFunc.append("            else if ((*it)->onPort(\"");

        extFunc.append(it->name);
        extFunc.append("\")) {\n");
        extFunc.append("                modelPhase = \n"); //TODO: to change
        extFunc.append("            }\n");
    }
    extFunc.append("        }\n");
    extFunc.append("    }");

    return extFunc;
}

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

        map<string, string> stateMap = submodel.stateDuration;
        string stateEnum = writeStateEnum(stateMap);
        string sigmaFunc = writeSigmaFunction(stateMap);
        string extFunc = writeExternalTransition(submodel.inPorts);

        if (!stateEnum.empty()) {
            string publicString = "public:";
            string privateVar = "private:\n";
            privateVar.append(stateEnum);
            privateVar.append("\n");
            privateVar.append(publicString);
            submodelContent = boost::replace_all_copy(submodelContent,
                                                      publicString,
                                                      privateVar);
        }

        if (!sigmaFunc.empty()) {
            string origTaFunc = 
                "    virtual vd::Time timeAdvance() const override\n"
                "    {\n"
                "        return vd::infinity;\n"
                "    }";

            submodelContent = boost::replace_all_copy(submodelContent,
                                                      origTaFunc,
                                                      sigmaFunc);
        }

        if (!extFunc.empty()) {
            string origExtFunc =
                "    virtual void externalTransition(\n"
                "            const vd::ExternalEventList& /*event*/,\n"
                "            vle::devs::Time /*time*/) override\n"
                "    {\n"
                "    }";

            submodelContent = boost::replace_all_copy(submodelContent,
                                                      origExtFunc,
                                                      extFunc);
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
