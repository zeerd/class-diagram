#include "class-parser.hpp"

#include <iostream>

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

static llvm::cl::list<std::string> argsReduceList(
    "reduce",
    llvm::cl::desc("Exclude location for the output(could use several times)"),
    llvm::cl::cat(category));
static llvm::cl::list<std::string> argsExcludeList(
    "exclude",
    llvm::cl::desc("Exclude location for the input(could use several times)"),
    llvm::cl::cat(category));
static llvm::cl::list<std::string> argsReduceNsList(
    "reduce-ns",
    llvm::cl::desc("Exclude namespace for the output(could use several times)"),
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
static llvm::cl::opt<bool> argsReversal(
    "reversal", llvm::cl::desc("Rotate 90 degrees(false as default)"),
    llvm::cl::init(false), llvm::cl::cat(category));

static llvm::cl::list<std::string> argsInput(
    "input", llvm::cl::desc("input a folder of a project or a single file"),
    llvm::cl::cat(category));
static llvm::cl::opt<std::string> argsOutput(
    "output",
    llvm::cl::desc(
        "Specify output basename(use the last part of input if not given)"),
    llvm::cl::init(""), llvm::cl::cat(category));

bool ClassParser::verbose()
{
    return argsVerbose;
}

std::string ClassParser::output()
{
    return argsOutput;
}

bool ClassParser::all()
{
    return argsShowAll;
}

bool ClassParser::hide()
{
    return !argsUnlinked;
}

bool ClassParser::reversal()
{
    return argsReversal;
}

std::string ClassParser::java()
{
    return argsJava;
}

std::string ClassParser::plantUML()
{
    return argsPlantUML;
}

std::string ClassParser::theme()
{
    return argsTheme;
}

json &ClassParser::classes()
{
    return jClasses;
}

std::vector<std::string> ClassParser::reduceList()
{
    std::vector<std::string> stdVector;
    stdVector.reserve(argsReduceList.size());
    std::copy(argsReduceList.begin(), argsReduceList.end(),
              std::back_inserter(stdVector));
    return stdVector;
}

std::vector<std::string> ClassParser::reduceNsList()
{
    std::vector<std::string> stdVector;
    stdVector.reserve(argsReduceNsList.size());
    std::copy(argsReduceNsList.begin(), argsReduceNsList.end(),
              std::back_inserter(stdVector));
    return stdVector;
}

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
}

bool ClassParser::detectCmplCmdJson(void)
{
    bool found = false;

    std::string in = argsInput.front();
    fs::path path  = fs::canonical(in);
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

    if (argsVerbose) {
        std::cout << "\n";
    }
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
                std::cout << "Grab header files from '" << in << "'.\n";
            }
        }

        if (inc) {
            inc_folders.push_back(fs::canonical(in).string());
        }

        for (const auto &entry :
             fs::recursive_directory_iterator(fs::canonical(in))) {
            if (isExclude(entry.path().string())) {
                continue;
            }
            if (entry.is_directory() && inc) {
                bool hasHeader = std::any_of(
                    fs::directory_iterator(entry), fs::directory_iterator{},
                    [this](const auto &file) { return isHeader(file.path()); });

                if (hasHeader) {
                    inc_folders.push_back(entry.path().string());
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
        if (argsVerbose) {
            std::cout << "Create fake compile_commands.json in "
                      << new_folder.string() << "\n\n";
        }
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
            /** c++17 is good, c++20 will cause namespace range issue. */
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

    // LCOV_EXCL_START
    if (argsModVersion) {
        std::cout << "\n"
                  << program_description << " Ver 0.0.1-" << BUILD_ID << "\n\n";
        exit(0);
    }
    // LCOV_EXCL_STOP

    for (int i = 0; i < argc; ++i) {
        fakeArgv.push_back(argv[i]);
    }

    if (argsInput.size() == 1
        && fs::path(argsInput.front()).filename() == "compile_commands.json") {
        std::ifstream file(argsInput.front());
        if (!file) {
            std::cerr << "Cannot open file: " << argsInput.front() << std::endl;
            return;
        }

        nlohmann::json json;
        file >> json;

        for (const auto &item : json) {
            if (item.contains("file")) {
                // std::cout << item["file"] << std::endl;
                cpp_files.push_back(item["file"]);
            }
        }
    }
    else {
        for (auto in : argsInput) {
            grab(in, true, false);
        }

        bool found = detectCmplCmdJson();
        if (!found) {
            createCmplCmdJson();
        }
    }

    std::transform(cpp_files.begin(), cpp_files.end(),
                   std::back_inserter(fakeArgv),
                   [](const std::string &cpp) { return strdup(cpp.c_str()); });

    if (argsVerbose) {
        std::cout << "Generated arguments:\n";
        for (auto &v : fakeArgv) {
            std::cout << v << "\n";
        }
        std::cout << "\n";
    }
}

void ClassParser::makeDefaultOutput()
{
    std::string in = argsInput.front();
    fs::path inputPath(fs::canonical(in));
    argsOutput = inputPath.filename().string();
    if (argsVerbose) {
        std::cout << "Use input name: " << inputPath << "\n";
        std::cout << "Use output name: " << argsOutput << "\n"
                  << "\n";
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
        new FindNamedClassActionFactory());
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    Tool.run(actionFactory.get());

    removeCommonPrefix();
}

void ClassParser::removeCommonPrefix()
{
    std::vector<std::string> locations;
    for (auto &clz : classes()) {
        if (!clz["isReduced"]
            && clz["location"].get<std::string>().length() > 0) {
            locations.push_back(clz["location"]);
        }
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
        pos = common_prefix.rfind('/');
        if (pos != std::string::npos) {
            common_prefix = common_prefix.substr(0, pos);
        }
    }

    for (auto &clz : classes()) {
        std::string location = clz["location"];
        if (location.substr(0, common_prefix.size()) == common_prefix) {
            clz["location"] = location.substr(
                common_prefix.size() + 1);  // +1 to remove the leading '/'
        }
    }

    if (argsVerbose) {
        std::cout << "\nRemove common prefix '" << common_prefix
                  << "' from location\n"
                  << "\n";
    }
}
