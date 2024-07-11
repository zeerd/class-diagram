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
    std::string theme;

private:
    std::string replaceHomeDirectory(std::string path);
    std::string makeCmd(std::string input);

public:
    PUmlToSvg(bool verb, std::string java, std::string jar, std::string theme)
        : verbose(verb), java(java), jar(jar), theme(theme)
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
    bool hide;
    std::ofstream uml;

private:
    std::string getLoc(json &cls);
    std::string getLastName(std::string clz);
    void drawBases(json &cls);
    void drawFields(json &cls);
    void drawClass(json &cls);
    void drawNested(json &cls);

public:
    JsonToPUml(json &clz, bool verb, bool bsc, bool hide)
        : classes(clz), verbose(verb), basic(bsc), hide(hide)
    {
    }
    void save(std::string output);
};

#endif /* CLASS_DIAGRAM_HPP */
