#include <iostream>
#include <cstdlib>

#include "../include/CMDParser.h"

int main(int argc, char* argv[]) {
    int doot = 0;
    double potatoes[3] = {0, 0, 0};
    std::string leek = "<no name given>";
    bool ack = false;

    std::string* someStuff = nullptr;
    size_t stuffLen = 0;

    bool* boolList = nullptr;
    size_t boolLen = 0;

    CMDParser parser;
    parser.bindVar<double[3]>("-p", potatoes, 3, "Sets the ph of the 3 potatoes");
    parser.bindVar<int>("-d", doot, 1, "Sets the doot");
    parser.bindVar<std::string>("-l", leek, 1, "names the leek");
    parser.bindVar<bool>("-a", ack, 0, "Ack??");
    parser.bindVar<std::string*>("-s", someStuff, "Just a list of stuff", CMDParser::VARIABLE_LENGTH, stuffLen);
    parser.bindVar<bool*>("-b", boolList, "Just a list of booleans", CMDParser::DEFINED_LENGTH, boolLen);
    if (!parser.parse(argc, argv)) {
        return -1;
    }

    std::cout << "Potatoes: " << potatoes[0] << ", " << potatoes[1] << ", " << potatoes[2] << std::endl;
    std::cout << "Doot: " << doot << std::endl;
    std::cout << "Leek's name: " << leek << std::endl;
    std::cout << "Ack? " << (ack ? "yes." : "no") << std::endl;
    std::cout << stuffLen << " stuffs were received" << std::endl;
    for (int i = 0; i < stuffLen; i++) {
        std::cout << "stuff[" << i << "] = " << someStuff[i] << std::endl;
    }
    std::cout << boolLen << " bools were received" << std::endl;
    for (int i = 0; i < boolLen; i++) {
        std::cout << "bool[" << i << "] = " << (boolList[i] == true ? "true" : "false") << std::endl;
    }

    delete[] someStuff;
    someStuff = nullptr;
    delete[] boolList;
    boolList = nullptr;

    return 0;
}
