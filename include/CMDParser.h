#pragma once

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <string>
#include <type_traits>
#include <vector>
#include <any>
#include <functional>
#include <map>
#include <charconv>
#include <iostream>

/** CMDParser class for parsing command line
 *  @class CMDParser
 *
 *  @note Address sanitizer may throw a fit with the _deAlloc functions,
 *        just set the environment variable ASAN_OPTIONS to include 
 *        alloc_dealloc_mismatch=0 and you should be good to go
 *  @note Don't even try with char* or char** unless u want a bad time ;)
 */
class CMDParser {
public:
    CMDParser();

    /// Enum to define listed argument length type
    typedef enum {
        STATIC_LENGTH,  // only the previously defined number of arguments are
                        // accepted
        VARIABLE_LENGTH,  // any length of extra arguments may be read in (can
                          // only be one, must be final parameter entered)
        DEFINED_LENGTH  // accepts an integer to define the number of variables
    } argType;

    /** Function to bind variables to flags
     *  @param flag The string that identifies this variable on the command
     * line.
     *  @param destination The variable to store the value from the command
     * line.
     *  @param numEntries The number of values expected to be stored in
     * destination.
     *  @param description The description to be used in the help menu.
     *
     *  @note This function is templated in order to properly
     *        typecast from void* to the correct datatype of destination
     */
    template <class T>
    void bindVar(std::string flag, T &destination, size_t numEntries,
                 std::string description) {

        using Type = std::remove_pointer_t<std::remove_all_extents_t<T>>;
        using TypePtr = Type*;

        //auto[store_to_dest_offset, allocate_dest, de_allo_temp] = generateMemFuncs<Type>(dest);

        auto store_to_dest_offset = [&](void* input, size_t offset) -> void {
            //cstrings are really iffy, cuz they aren't going to come in as
            //a separated array like everything else
            TypePtr dest;
            if constexpr (std::is_pointer_v<T> || std::is_array_v<T>) {
                dest = destination;
            } else {
                dest = &destination;
            }
            dest[offset] = *static_cast<TypePtr>(input);
        };
        auto allocate_dest = [&](size_t size) -> void {
            TypePtr dest;
            if constexpr (std::is_pointer_v<T> || std::is_array_v<T>) {
                dest = destination;
            } else {
                dest = &destination;
            }

            if (dest != nullptr) {
                delete[] dest;
            }
            dest = new Type[size];
        };
        auto de_allo_temp = [&](void* &ptr) {
            delete static_cast<TypePtr>(ptr);
        };

        TypePtr dest;
        if constexpr (std::is_pointer_v<T> || std::is_array_v<T>) {
            dest = destination;
        } else {
            dest = &destination;
        }

        m_params.push_back({dest, numEntries, description,
                            flag, STATIC_LENGTH, nullptr,
                            store_to_dest_offset,
                            allocate_dest, de_allo_temp,
                            typeid(Type).hash_code(), 
                            typeid(Type).name()});

       appendConversionIfAvailable<Type>();
    }

    /** Function to bind variables to flags (intended for non-predetermined
     * length parameters)
     *  @param flag The string that identifies this variable on the command
     * line.
     *  @param destination The variable to store the value from the command
     * line.
     *  @param description The description to be used in the help menu.
     *  @param lengthType Determines the intended behaviour for readin in a list
     * of values.
     *  @param numEntries The number of values expected to be stored in
     * destination.
     *
     *  @note This function is templated in order to properly
     *        typecast from void* to the correct datatype of destination
     *  @note Another variation of this function should be added for writing
     *        results into an std::vector<T>
     */
    template <class T>
    void bindVar(std::string flag, T &destination, std::string description,
                 argType lengthType, size_t& numEntries) {
        static_assert(std::is_pointer_v<T>, "destination must be a pointer or array");
        if(lengthType == STATIC_LENGTH) {
           throw std::runtime_error(std::string("lengthType must either be DEFINED_LENGTH or VARIABLE_LENGTH"));
        }
        numEntries = 0;
        using Type = std::remove_pointer_t<std::remove_all_extents_t<T>>;
        using TypePtr = Type*;
        
        //auto[store_to_dest_offset, allocate_dest, de_allo_temp] = generateMemFuncs<Type>(destination);

        auto store_to_dest_offset = [&](void* input, size_t offset) -> void {
            //cstrings are really iffy, cuz they aren't going to come in as
            //a separated array like everything else
            destination[offset] = *static_cast<TypePtr>(input);
        };
        auto allocate_dest = [&](size_t size) -> void {
            if (destination != nullptr) {
                delete[] destination;
            }
            destination = new Type[size];
        };
        auto de_allo_temp = [&](void* &ptr) {
            delete static_cast<TypePtr>(ptr);
        };

        m_params.push_back({static_cast<void*>(destination), 0, description, flag,
                            lengthType, &numEntries, 
                            store_to_dest_offset, allocate_dest, 
                            de_allo_temp, typeid(Type).hash_code(),
                            typeid(Type).name()});

        appendConversionIfAvailable<Type>();
    }

    template <class T>
    void registerConversion(std::function<void(void*&, char*)> conversionFunc) {
        convertFromString.insert({typeid(T).hash_code(), conversionFunc});
    }

    bool parse(int argc, char* argv[]);

    // the default is for CMDParser to generate the help menu
    void setHelpMessage(std::string msg = "");
    void generateHelp(bool val = true);

    void printHelp();

private:
    // Struct for storing an argument
    typedef struct {
        void* _dest;
        size_t _size;
        std::string _desc;
        std::string _flag;
        argType _lenType;
        size_t* _newLen;
        std::function<void(void*, size_t)> _storeOffset;
        std::function<void(size_t)> _allocate;
        std::function<void(void*&)> _deAllocate;
        size_t _typeHash;
        const char* _typeName;
    } _container;

    template <typename T>
    struct fail_assertion : std::false_type {};

    template <typename ValType>
    static void string_to_val(void* &dest, char* source) {
        if constexpr (std::is_same_v<ValType, bool>) {
            ValType* boolean = new ValType[1];
            if (strcmp(source, "true") == 0 ||
                strcmp(source, "1") == 0 ||
                strcmp(source, "t") == 0) {
                boolean[0] = true;
            } else {
                boolean[0] = false;
            }
            dest = static_cast<void*>(boolean);
        } else if constexpr (std::is_same_v<ValType, std::string>) {
            //std::string
            std::string* str = new std::string(source);
            dest = static_cast<void*>(str);
        } else if constexpr (std::is_integral_v<ValType>) {
            //Integer Thing
            ValType* X = new ValType[1];
            size_t end = std::strlen(source);
            std::from_chars(static_cast<const char*>(source), 
                            static_cast<const char*>(source) + end,
                            X[0], 10);
            dest = static_cast<void*>(X);
        } else if constexpr (std::is_arithmetic_v<ValType>) {
            //float thing
            ValType* X = new ValType[1];
            *X = std::stof(source);
            dest = static_cast<void*>(X);
        } else {
            static_assert(fail_assertion<ValType>::value, "Using string_to_val for non-supported type!");
        }
    }

    /*
    template <class Type>
    auto generateMemFuncs(Type* &dest) {
        using TypePtr = Type*;
        auto store_to_dest_offset = [&](void* input, size_t offset) -> void {
            //cstrings are really iffy, cuz they aren't going to come in as
            //a separated array like everything else
            dest[offset] = *static_cast<TypePtr>(input);
        };
        auto allocate_dest = [&](size_t size) -> void {
            if (dest != nullptr) {
                delete[] dest;
            }
            dest = new Type[size];
        };
        auto de_allo_temp = [&](void* &ptr) {
            delete static_cast<TypePtr>(ptr);
        };
        return std::make_tuple(store_to_dest_offset, allocate_dest, de_allo_temp);
    }
    */

    template <class Type>
    void appendConversionIfAvailable() {
        if (!convertFromString.contains(typeid(Type).hash_code())) {
            if constexpr (std::is_arithmetic_v<Type> || 
                          std::is_same_v<Type, bool> ||
                          std::is_same_v<Type, std::string>) {
                convertFromString.insert({typeid(Type).hash_code(), string_to_val<Type>});
            } else {
                // this could be bad if the user doesn't provide a conversion before parsing
                std::cerr << "Warning (CMDParser): A conversion function has not been provided for type: " << typeid(Type).name() << std::endl;
            }
        }
    }

    static std::map<size_t, std::function<void(void*&, char*)>> convertFromString;

    friend bool operator==(_container A, std::string B);

    std::vector<_container> m_params;

    bool help;
    bool generate_help;
    std::string custom_help;
};
