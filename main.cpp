#include "VPZStructure.hpp"
#include "ReadXMI.hpp"
#include "WriteVPZ.hpp"
#include "WriteCPP.hpp"
#include <boost/filesystem.hpp>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 3) {
        cerr << "Usage: " 
                  << argv[0] 
                  << " SOURCE_FILE" 
                  << " PROJECT_DIRECTORY" 
             << endl;

        return 1;
    }

    using boost::filesystem::path;

    path sourcePath = path(argv[1]);
    if (is_directory(sourcePath)) {
        cerr << argv[1] << " is not a file. Abort." << endl;
        return 1;
    }

    path projectPath = path(argv[2]);
    if (!is_directory(projectPath)) {
        cerr << argv[2] << " is not a directory. Abort." << endl;
        return 1;
    }

    VLEProject vle_project = readXMI(argv[1]);
    writeVPZ(vle_project, argv[2]);

    writeCPP(vle_project, argv[2]);
    
    cout << "Conversion finished." << endl;
    return 0;
}
