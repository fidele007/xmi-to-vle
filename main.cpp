#include "VPZStructure.hpp"
#include "ReadXMI.hpp"
#include "WriteVPZ.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 3) {
        cerr << "Usage: " 
                  << argv[0] 
                  << " SOURCE_FILE" 
                  << " DESTINATION_FILE" 
             << endl;

        return 1;
    }

    VLEProject vle_project = read(argv[1], true);

    write(vle_project, argv[2]);

    cout << "Conversion finished." << endl;

    return 0;
}
