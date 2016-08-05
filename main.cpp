#include "VPZStructure.hpp"
#include "ModelGetter.hpp"
#include "ReadXMI.hpp"
#include "WriteVPZ.hpp"

using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 3) {
        cerr << "Usage: " 
                  << argv[0] 
                  << " SOURCE_FILE" 
                  << " DESTINATION_FILE" 
             << endl;

        return 1;
    }

    ifstream input(argv[1]);
    VLEProject vle_project = read(input);

    write(vle_project, argv[2]);

    cout << "Conversion succeeded." << endl;

    return 0;
}
