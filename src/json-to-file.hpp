#ifndef JSON_TO_FILE_HPP
#define JSON_TO_FILE_HPP

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class JsonToFile {
private:
    json &classes;
    bool verbose;

public:
    JsonToFile(json &clz, bool verb) : classes(clz), verbose(verb) {}
    void save(std::string output);
};

#endif /* JSON_TO_FILE_HPP */
