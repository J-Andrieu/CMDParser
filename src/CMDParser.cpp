#ifndef CMDPARSER_CPP
#define CMDPARSER_CPP

#include "CMDParser.h"

CMDParser::CMDParser() {
    m_params.resize (0);
    bindVar<bool> ("-h", &help, 0, "Displays the help page");
}

bool operator== (CMDParser::_container A, std::string B) {
    return A._flag == B;
}

bool CMDParser::parse (int argc, char* argv[]) {
    std::sort (m_params.begin(), m_params.end(), [] (auto A, auto B) {
        return A._flag < B._flag;
    });
    for (int i = 1; i < argc; i++) {
        auto pos = std::find (m_params.begin(), m_params.end(), std::string (argv[i]));
        if (pos == std::end (m_params)) {
            printf ("The provided flag (%s) is invalid\n", argv[i]);
            printHelp();
            return false;
        } else if (pos->_flag == "-h") {
            printHelp();
            return false;
        }

        try {
            switch (pos->_type) {
                case INT: {
                        int* dest = (int*) pos->_dest;
                        for (size_t j = 0; j < pos->_size; j++) {
                            dest[j] = std::stoi (argv[++i]);
                        }
                    }
                    break;
                case DOUBLE: {
                        double* dest = (double*) pos->_dest;
                        for (size_t j = 0; j < pos->_size; j++) {
                            dest[j] = std::stof (argv[++i]);
                        }
                    }
                    break;
                case STRING: {
                        std::string* dest = (std::string*) pos->_dest;
                        for (size_t j = 0; j < pos->_size; j++) {
                            dest[j] = std::string (argv[++i]);
                        }
                    }
                    break;
                case BOOL: {
                        bool* dest = (bool*) pos->_dest;
                        if (pos->_size > 0) {
                            for (size_t j = 0; j < pos->_size; j++) {
                                i++;
                                if (strcmp(argv[i], "true") == 0 || strcmp(argv[i], "1") == 0) {
                                    dest[j] = true;
                                } else {
                                    dest[j] = false;
                                }
                            }
                        } else {
                            dest[0] = true;
                        }
                    }
                    break;
            }
        } catch (std::exception& e) {
            printf("An invalid parameter was passed to %s resulting in the following exception:\n%s\n", pos->_flag.c_str(), e.what());
            printHelp();
            return false;
        }
    }
    return true;
}

void CMDParser::printHelp() {
    printf ("Displaying Help Page:\n\n");
    for (auto param : m_params) {
        printf ("%s\t|\t%s (takes %s argument%s)\n", param._flag.c_str(), param._desc.c_str(), (param._size == 0 ? "no" : std::to_string (param._size).c_str()), (param._size == 1 ? "" : "s"));
    }
    printf ("\n");
}
#endif // CMDPARSER_CPP

