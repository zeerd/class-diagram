#ifndef CLASS_PARSER_HPP
#define CLASS_PARSER_HPP

#include <filesystem>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json   = nlohmann::json;

class ClassParser {
private:
    static bool isHeader(const fs::path &path);
    static bool isExclude(std::string path_str);
    static bool detectCmplCmdJson(void);
    void grab(std::string in, bool src, bool inc);
    void createCmplCmdJson();
    void makeFakeArgs(int argc, const char **argv);
    static void makeDefaultOutput();
    static void removeCommonPrefix();

public:
    void parseProject(int argc, const char **argv);

    static bool verbose();
    static std::string output();
    static bool all();
    static bool hide();
    static bool reversal();
    static std::string java();
    static std::string plantUML();
    static std::string theme();
    static json &classes();
    static std::vector<std::string> reduceList();
    static std::vector<std::string> reduceNsList();

private:
    std::vector<std::string> cpp_files;
    std::vector<std::string> inc_folders;
    std::vector<const char *> fakeArgv;
};

#endif /* CLASS_PARSER_HPP */
