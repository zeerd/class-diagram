#include "class-parser.hpp"

#include <iostream>
#include <nlohmann/json.hpp>

#include "class-visitor.hpp"

using json = nlohmann::json;
static json jClasses;

static const std::string program_description = "C++ Class Diagram Generator";
static llvm::cl::OptionCategory category(
    program_description.c_str(),
    "Generate Class Diagram from C++ source/header files.");
static llvm::cl::opt<bool> argsModVersion(
    "mod-version", llvm::cl::desc("Show the version of this"),
    llvm::cl::init(false), llvm::cl::cat(category));

static llvm::cl::list<std::string> argsExcludeList(
    "exclude",
    llvm::cl::desc("Exclude location from the output(could use several times)"),
    llvm::cl::cat(category));
static llvm::cl::list<std::string> argsExcludeNsList(
    "exclude-ns",
    llvm::cl::desc(
        "Exclude namespace from the output(could use several times)"),
    llvm::cl::cat(category));
static llvm::cl::list<std::string> argsIncludeList(
    "include",
    llvm::cl::desc(
        "Extra include location of header files(could use several times)"),
    llvm::cl::cat(category));

static llvm::cl::opt<std::string> argsPlantUML(
    "plantuml", llvm::cl::desc("Specify the path of plantuml.jar file"),
    llvm::cl::cat(category));
static llvm::cl::opt<std::string> argsTheme(
    "theme", llvm::cl::desc("Specify the path of theme file for plantuml.jar"),
    llvm::cl::cat(category));
static llvm::cl::opt<std::string> argsJava(
    "java",
    llvm::cl::desc("Specify the full path of java binary for plantuml.jar"),
    llvm::cl::init("java"), llvm::cl::cat(category));

static llvm::cl::opt<bool> argsShowAll(
    "all", llvm::cl::desc("Show all members(false as default)"),
    llvm::cl::init(false), llvm::cl::cat(category));
static llvm::cl::opt<bool> argsVerbose(
    "verbose", llvm::cl::desc("Show more logs(false as default)"),
    llvm::cl::init(false), llvm::cl::cat(category));
static llvm::cl::opt<bool> argsUnlinked(
    "unlinked", llvm::cl::desc("Show unlinked classes(false as default)"),
    llvm::cl::init(false), llvm::cl::cat(category));

static llvm::cl::list<std::string> argsInput(
    "input", llvm::cl::desc("input a folder of a project or a single file"),
    llvm::cl::cat(category));
static llvm::cl::opt<std::string> argsOutput(
    "output",
    llvm::cl::desc(
        "Specify output basename(use the last part of input if not given)"),
    llvm::cl::init(""), llvm::cl::cat(category));

bool ClassParser::verbose() { return argsVerbose; }
std::string ClassParser::output() { return argsOutput; }
bool ClassParser::all() { return argsShowAll; }
bool ClassParser::hide() { return !argsUnlinked; }
std::string ClassParser::java() { return argsJava; }
std::string ClassParser::plantUML() { return argsPlantUML; }
std::string ClassParser::theme() { return argsTheme; }
json &ClassParser::classes() { return jClasses; }

bool ClassParser::isHeader(const fs::path &path)
{
    if (path.extension() == ".h" || path.extension() == ".hpp"
        || path.extension() == ".hxx") {
        return true;
    }
    if (path.extension().empty() || path.extension() == "") {
        std::ifstream file(path);
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("#include") != std::string::npos
                || line.find("#pragma once") != std::string::npos
                || line.find("#ifndef") != std::string::npos
                || line.find("#define") != std::string::npos
                || line.find("#endif") != std::string::npos) {
                return true;
            }
        }
    }
    return false;
}

bool ClassParser::isExclude(std::string path_str)
{
    if (std::any_of(argsIncludeList.begin(), argsIncludeList.end(),
                    [&](const std::string &include) {
                        return path_str.find(include) != std::string::npos;
                    })) {
        return false;
    }

    if (std::any_of(argsExcludeList.begin(), argsExcludeList.end(),
                    [&](const std::string &exclude) {
                        return path_str.find(exclude) != std::string::npos;
                    })) {
        return true;
    }

    return false;
};

bool ClassParser::detectCmplCmdJson(void)
{
    bool found = false;

    std::string in = argsInput.front();
    fs::path path  = fs::absolute(in);
    if (fs::is_regular_file(path)) {
        path = path.parent_path();
    }

    do {
        if (argsVerbose) {
            std::cout << "Detecting " << (path / "compile_commands.json")
                      << "\n";
        }
        found = fs::exists(path / "compile_commands.json");
        if (path.string() == "/") {
            break;
        }
        path = path.parent_path();
    } while (!found);
    return found;
}

void ClassParser::grab(std::string in, bool src, bool inc)
{
    fs::path input_path(in);
    if (fs::is_regular_file(input_path)) {
        if (inc) {
            inc_folders.push_back(input_path.parent_path());
        }
        if (src) {
            cpp_files.push_back(in);
        }
    }
    else {
        if (argsVerbose) {
            if (inc) {
                std::cout << "Grab files from " << in << " .\n";
            }
        }

        for (const auto &entry :
             fs::recursive_directory_iterator(fs::absolute(in))) {
            if (isExclude(entry.path().string())) {
                continue;
            }
            if (entry.is_directory() && inc) {
                for (const auto &file : fs::directory_iterator(entry)) {
                    if (isHeader(file.path())) {
                        inc_folders.push_back(entry.path().string());
                        break;
                    }
                }
            }
            if (entry.is_regular_file() && src) {
                if (entry.path().extension() == ".cpp"
                    || entry.path().extension() == ".cc") {
                    cpp_files.push_back(entry.path().string());
                }
            }
        }
    }
}

void ClassParser::createCmplCmdJson()
{
    fs::path new_folder = fs::absolute(".class-diagram");
    fs::create_directory(new_folder);

    std::ofstream file(new_folder / "compile_commands.json");
    if (file.is_open()) {
        for (auto in : argsInput) {
            grab(in, false, true);
        }
        for (const auto &inc : argsIncludeList) {
            grab(inc, false, true);
        }

        json cmpl_cmd = json::array();
        std::ostringstream incs;
        for (auto inc : inc_folders) {
            incs << " -I" << inc;
        }
        for (auto cpp : cpp_files) {
            json jFile         = json::object();
            jFile["directory"] = new_folder.string();
            std::ostringstream ssCmd;
            ssCmd << "/usr/bin/c++" << incs.str() << " -std=c++17 -c " << cpp;
            jFile["command"] = ssCmd.str();
            jFile["file"]    = cpp;
            cmpl_cmd.push_back(jFile);
        }
        file << cmpl_cmd.dump(2);
        file.close();
    }

    char *argsPath = new char[new_folder.string().length() + strlen("-p=") + 1];
    std::strcpy(argsPath, "-p=");
    std::strcat(argsPath, new_folder.string().c_str());
    fakeArgv.push_back(argsPath);
}

void ClassParser::makeFakeArgs(int argc, const char **argv)
{
    llvm::cl::ParseCommandLineOptions(argc, argv, program_description.c_str());

    if (argsModVersion) {
        std::cout << "\n"
                  << program_description << " Ver 0.0.1-" << BUILD_ID << "\n\n";
        exit(0);
    }

    for (int i = 0; i < argc; ++i) {
        fakeArgv.push_back(argv[i]);
    }

    for (auto in : argsInput) {
        grab(in, true, false);
    }

    bool found = detectCmplCmdJson();
    if (!found) {
        createCmplCmdJson();
    }

    for (auto cpp : cpp_files) {
        char *c = new char[cpp.length() + 1];
        std::strcpy(c, cpp.c_str());
        fakeArgv.push_back(c);
    }

    if (argsVerbose) {
        std::cout << "Generated arguments:\n";
        for (auto &c : fakeArgv) {
            std::cout << c << "\n";
        }
    }
}

void ClassParser::makeDefaultOutput()
{
    std::string in = argsInput.front();
    fs::path inputPath(fs::canonical(in));
    std::cout << "Use input name: " << inputPath << "\n";
    argsOutput = inputPath.filename().string();
    if (argsVerbose) {
        std::cout << "Use output name: " << argsOutput << "\n";
    }
}

void ClassParser::parseProject(int argc, const char **argv)
{
    makeFakeArgs(argc, argv);

    int fakeArgc = fakeArgv.size();

#if LLVM_VERSION_MAJOR >= 13
    auto ExpectedParser
        = CommonOptionsParser::create(fakeArgc, fakeArgv.data(), category);
    if (!ExpectedParser) {
        std::cerr << "Error: " << llvm::toString(ExpectedParser.takeError())
                  << "\n";
        return;
    }
    CommonOptionsParser &op = ExpectedParser.get();
#else
    clang::tooling::CommonOptionsParser op(fakeArgc, fakeArgv.data(), category);
#endif
    if (argsOutput == "") {
        makeDefaultOutput();
    }

    std::unique_ptr<clang::tooling::FrontendActionFactory> actionFactory(
        new FindNamedClassActionFactory(classes(), argsVerbose, argsExcludeList,
                                        argsExcludeNsList));
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    Tool.run(actionFactory.get());

    removeCommonPrefix();
}

void ClassParser::removeCommonPrefix()
{
    std::vector<std::string> locations;
    for (auto &clz : classes()) {
        locations.push_back(clz["location"]);
    }

    std::sort(locations.begin(), locations.end());

    auto mismatch_pair
        = std::mismatch(locations.front().begin(), locations.front().end(),
                        locations.back().begin());
    std::string common_prefix(locations.front().begin(), mismatch_pair.first);

    size_t pos = common_prefix.find(':');
    if (pos != std::string::npos) {
        common_prefix = common_prefix.substr(0, pos);
    }
    fs::path commonPath(common_prefix);
    if (fs::is_regular_file(commonPath)) {
        common_prefix = commonPath.parent_path().string();
    }
    else {
        size_t pos = common_prefix.rfind('/');
        if (pos != std::string::npos) {
            common_prefix = common_prefix.substr(0, pos);
        }
    }
    if (argsVerbose) {
        std::cout << "Remove common prefix '" << common_prefix
                  << "' from location\n";
    }

    for (auto &clz : classes()) {
        std::string location = clz["location"];
        if (location.substr(0, common_prefix.size()) == common_prefix) {
            clz["location"] = location.substr(
                common_prefix.size() + 1);  // +1 to remove the leading '/'
        }
    }
}
