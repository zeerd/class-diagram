#ifndef CLASS_PARSER_HPP
#define CLASS_PARSER_HPP

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using json   = nlohmann::json;

class ClassParser {
public:
    bool isHeader(const fs::path &path);
    bool isExclude(std::string path_str);
    bool detectCmplCmdJson(void);
    void grab(std::string input, bool src, bool inc);
    void createCmplCmdJson();
    void makeFakeArgs(int argc, const char **argv);
    void makeDefaultOutput();
    void parseProject(int argc, const char **argv);

    bool verbose();
    std::string output();
    bool basic();
    std::string java();
    std::string plantuml();
    json &classes();

private:
    std::string input;
    std::vector<std::string> cpp_files;
    std::vector<std::string> inc_folders;
    std::vector<const char *> fakeArgv;
};

#endif /* CLASS_PARSER_HPP */
