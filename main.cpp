#include "VPZStructure.hpp"
#include "ModelGetter.hpp"
#include "ReadXMI.hpp"
#include "WriteVPZ.hpp"

int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cerr << "Usage: " 
                  << argv[0] 
                  << " SOURCE_FILE" 
                  << " DESTINATION_FILE" 
                  << std::endl;

        return 1;
    }

    std::ifstream input(argv[1]);
    VLEProject vle_project = read(input);

    write(vle_project, argv[2]);

    std::cout << "Conversion succeeded." << std::endl;

    return 0;
}
