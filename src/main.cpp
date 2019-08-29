#include <iostream>
#include <cstdlib>

#include "../include/CMDParser.h"

int main(int argc, char* argv[]) {
    int doot = 0;
    double potatoes[3] = {0, 0, 0};
    std::string leek = "<no name given>";
    bool ack = false;
    CMDParser parser;

    parser.bindVar<double>("-p", potatoes, 3, "Sets the ph of the 3 potatoes");
    parser.bindVar<int>("-d", &doot, 1, "Sets the doot");
    parser.bindVar<std::string>("-l", &leek, 1, "names the leek");
    parser.bindVar<bool>("-a", &ack, 0, "Ack??");
    if (!parser.parse(argc, argv)) {
        return -1;
    }

    std::cout << "Potatoes: " << potatoes[0] << ", " << potatoes[1] << ", " << potatoes[2] << std::endl;
    std::cout << "Doot: " << doot << std::endl;
    std::cout << "Leek's name: " << leek << std::endl;
    std::cout << "Ack? " << (ack ? "yes." : "no") << std::endl;

    return 0;
}
