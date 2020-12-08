#include "CMDParser.h"

std::map<size_t, std::function<void(void*&, char*)>> CMDParser::convertFromString;

/// CMDParser default constructor
CMDParser::CMDParser() {
    m_params.resize(0);
    bindVar<bool>("-h", help, 0, "Displays the help page");
    generate_help = true;
    custom_help = "";
}

/// Operator== overload to help with searching for parameter strings
bool operator==(CMDParser::_container A, std::string B) { return A._flag == B; }

/** Parses command line arguments
 *  Takes values from the command line and stores them
 *  in their respective bound variables
 *
 *  @param argc The number of cstrings being passed
 *  @param argv The array of cstrings to be parsed
 *
 *  @return true if all arguments passed are expeccted, and formatted validly
 *
 *  @note Returns early if an invalid parameter is met and prints the help
 * message
 */
bool CMDParser::parse(int argc, char* argv[]) {
    std::sort(m_params.begin(), m_params.end(),
              [](auto A, auto B) { return A._flag < B._flag; });
    for (int i = 1; i < argc; i++) {
        auto pos =
            std::find(m_params.begin(), m_params.end(), std::string(argv[i]));
        if (pos == std::end(m_params)) {
            printf("The provided flag (%s) is invalid\n", argv[i]);
            printHelp();
            return false;
        } else if (pos->_flag == "-h") {
            printHelp();
            return false;
        } else if (!convertFromString.contains(pos->_typeHash)) {
            throw std::runtime_error(std::string("CMDParser: A conversion function was not provided for the type: ") + std::string(pos->_typeName));
        }

        if (pos->_lenType == DEFINED_LENGTH) {
            try {
                *(pos->_newLen) = std::stoi(argv[++i]);
            } catch (std::exception& e) {
                printf(
                    "An invalid parameter (%s) was passed to %s resulting in "
                    "the following exception:\n%s\n",
                    argv[i], pos->_flag.c_str(), e.what());
                printHelp();
                return false;
            }
            pos->_size = *(pos->_newLen);
        } else if (pos->_lenType == VARIABLE_LENGTH) {
            pos->_size = argc - i - 1;
            *(pos->_newLen) = pos->_size;
        }

        try {
            if (pos->_lenType != STATIC_LENGTH) {
                pos->_allocate(pos->_size);
            }
            if (pos->_typeHash == typeid(bool).hash_code() && pos->_size == 0) {
                //bool flags that don't want input :P
                bool* val = new bool[1];
                val[0] = true;
                pos->_storeOffset(static_cast<void*>(val), 0);
                delete[] val;
            } else {
                for (size_t j = 0; j < pos->_size; j++) {
                    void* val = nullptr;
                    convertFromString[pos->_typeHash](val, argv[++i]);
                    pos->_storeOffset(val, j);
                    if (val != nullptr) {
                        pos->_deAllocate(val);
                    }
                }
            }
        } catch (std::exception& e) {
            printf(
                "An invalid parameter (%s) was passed to %s resulting in the "
                "following exception:\n%s\n",
                argv[i], pos->_flag.c_str(), e.what());
            printHelp();
            return false;
        }
    }
    return true;
}

/** Sets a custom help message for the parser
 *  @note Does not disable the generated help message
 */
void CMDParser::setHelpMessage(std::string msg) { custom_help = msg; }

/// Sets whether the parser should generate a help message
void CMDParser::generateHelp(bool val) { generate_help = val; }

/** Prints out the help menu
 *  The help menu will be generated from the provided descriptions,
 *  or it can be bypassed with the generateHelp() function
 */
void CMDParser::printHelp() {
    if (custom_help.length() != 0) {
        printf("%s\n", custom_help.c_str());
    }

    if (generate_help) {
        if (custom_help.length() == 0) {
            printf("Displaying Help Page:\n\n");
        }
        int flagPadding = 0;
        for (auto param : m_params) {
            if (flagPadding < param._flag.size()) {
                flagPadding = param._flag.size();
            }
        }
        flagPadding++;
        flagPadding *= -1;
        std::string format1 = std::string("% ") + std::to_string(flagPadding) +
                              std::string("s|\t%s (takes %s argument%s)\n");
        std::string format2 = std::string("% ") + std::to_string(flagPadding) +
                              std::string("s|\t%s %s\n");
        for (auto param : m_params) {
            if (param._lenType == STATIC_LENGTH) {
                printf(format1.c_str(), param._flag.c_str(),
                       param._desc.c_str(),
                       (param._size == 0 ? "no"
                                         : std::to_string(param._size).c_str()),
                       (param._size == 1 ? "" : "s"));
            } else {
                printf(format2.c_str(), param._flag.c_str(),
                       param._desc.c_str(),
                       param._lenType == VARIABLE_LENGTH
                           ? "(Will process the rest of the input)"
                           : "(Must define number of elements before list)");
            }
        }
        printf("\n");
    }
}
