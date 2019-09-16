#ifndef CMDPARSER_H
#define CMDPARSER_H

#include <string>
#include <cstring>
#include <exception>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <cstdlib>

class CMDParser {
public:
    CMDParser();

    template <typename T>
    void bindVar (std::string flag, T* destination, size_t numEntries, std::string description) {
        datatype type;
        if (std::is_same<T, int>()) {
            type = INT;
        } else if (std::is_same<T, std::string>()) {
            type = STRING;
        } else if (std::is_same<T, double>()) {
            type = DOUBLE;
        } else if (std::is_same<T, bool>()) {
            type = BOOL;
        }
        m_params.push_back ({type, (void*) destination, numEntries, description, flag});
    }

    bool parse (int argc, char* argv[]);

    void printHelp();

private:
    typedef enum {
        INT,
        DOUBLE,
        STRING,
        BOOL
    } datatype;

    typedef struct {
        datatype _type;
        void* _dest;
        size_t _size;
        std::string _desc;
        std::string _flag;
    } _container;

    friend bool operator== (_container A, std::string B);

    std::vector<_container> m_params;

    bool help;
};
#endif // CMDPARSER_H

