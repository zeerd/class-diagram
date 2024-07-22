#ifndef PUML_TO_SVG_HPP
#define PUML_TO_SVG_HPP

#include <string>
class PUmlToSvg {
private:
    bool verbose;
    const std::string java;
    const std::string jar;
    const std::string theme;

private:
    static std::string replaceHomeDirectory(std::string path);
    std::string makeCmd(const std::string &input);

public:
    PUmlToSvg(bool verb, const std::string &java, const std::string &jar,
              const std::string &theme)
        : verbose(verb), java(java), jar(jar), theme(theme)
    {
    }
    void save(const std::string &input, const std::string &output);
};

#endif /* PUML_TO_SVG_HPP */
