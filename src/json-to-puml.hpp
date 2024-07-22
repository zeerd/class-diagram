#ifndef JSON_TO_PUML_HPP
#define JSON_TO_PUML_HPP

#include <fstream>
#include <nlohmann/json.hpp>
#include <set>

using json = nlohmann::json;

class JsonToPUml {
private:
    json &classes;
    bool verbose;
    bool basic;
    bool hide;
    bool reversal;
    std::ofstream uml;
    std::set<std::string> written;

private:
    static std::string getLastName(const std::string &clz);

    void drawBases(json &cls);
    void drawFields(json &cls);
    void drawClass(const json &cls, bool drawInComplete = false);
    void drawNested(const json &cls);

    void drawTypedef(const std::string &to);

    static std::string wrap(const std::string &str, size_t width);
    void write(std::string s)
    {
        uml.write(s.c_str(), s.length());
    }

public:
    JsonToPUml(json &clz, bool verb, bool bsc, bool hide, bool reversal)
        : classes(clz),
          verbose(verb),
          basic(bsc),
          hide(hide),
          reversal(reversal)
    {
    }
    void save(std::string output);
};

#endif /* JSON_TO_PUML_HPP */
