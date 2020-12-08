#include <iostream>
#include <cstdlib>

#include "../include/CMDParser.h"

typedef struct {
    int a;
    int b;
    int c;
    int x;
    int y;
    int z;
} wierdStruct;

int main(int argc, char* argv[]) {
    int doot = 0;
    double potatoes[3] = {0, 0, 0};
    std::string leek = "<no name given>";
    bool ack = false;

    std::string* someStuff = nullptr;
    size_t stuffLen = 0;

    bool* boolList = nullptr;
    size_t boolLen = 0;

    wierdStruct fooBar{0, 0, 0, 0, 0, 0};

    CMDParser parser;
    parser.registerConversion<wierdStruct>([](void* &dest, char* src) -> void {
        wierdStruct* temp = new wierdStruct[1];
        temp->a = std::stoi(src);
        temp->z = temp->y = temp->x = temp->c = temp->b = temp->a;
        dest = static_cast<void*>(temp);
    });
    parser.bindVar("-p", potatoes, 3, "Sets the ph of the 3 potatoes");
    parser.bindVar("-d", doot, 1, "Sets the doot");
    parser.bindVar("-l", leek, 1, "names the leek");
    parser.bindVar("-a", ack, 0, "Ack??");
    parser.bindVar("-s", someStuff, "Just a list of stuff", CMDParser::VARIABLE_LENGTH, stuffLen);
    parser.bindVar("-b", boolList, "Just a list of booleans", CMDParser::DEFINED_LENGTH, boolLen);
    parser.bindVar("-w", fooBar, 1, "Takes one int and makes 5 copies");
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
    std::cout << "fooBar: " << fooBar.a << " " << fooBar.b << " " << fooBar.c << " " << fooBar.x << " " << fooBar.y << " " << fooBar.z << std::endl;

    delete[] someStuff;
    someStuff = nullptr;
    delete[] boolList;
    boolList = nullptr;

    return 0;
}
