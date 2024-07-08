#ifndef CLASS_DIAGRAM_HPP
#define CLASS_DIAGRAM_HPP

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

using json = nlohmann::json;

class PUmlToSvg {
private:
    bool verbose;
    std::string java;
    std::string jar;

public:
    PUmlToSvg(bool verb, std::string java, std::string jar)
        : verbose(verb), java(java), jar(jar)
    {
    }
    void save(std::string input, std::string output);
};

class JsonToFile {
private:
    json &classes;
    bool verbose;

public:
    JsonToFile(json &clz, bool verb) : classes(clz), verbose(verb) {}
    void save(std::string output);
};

class SvgToHtml {
private:
    bool verbose;

public:
    SvgToHtml(bool verb) : verbose(verb) {}
    void save(std::string input, std::string output);
};

class JsonToPUml {
private:
    json &classes;
    bool verbose;
    bool basic;
    std::ofstream puml;

private:
    std::string getLoc(json &cls);
    void draw_bases(json &cls);
    void draw_fields(json &cls);
    void draw_class(json &cls);

public:
    JsonToPUml(json &clz, bool verb, bool bsc)
        : classes(clz), verbose(verb), basic(bsc)
    {
    }
    void save(std::string output);
};

#endif /* CLASS_DIAGRAM_HPP */
