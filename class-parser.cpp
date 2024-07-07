#include "class-diagram.hpp"

static json jclasses;

static const std::string program_description = "C++ Class Diagram Generator";
static llvm::cl::OptionCategory category(
    program_description.c_str(),
    "Generate Class Diagram from C++ source/header files.");

static llvm::cl::list<std::string> argsExcludeList(
    "exclude",
    llvm::cl::desc("Exclude location from the output(could use several times)"),
    llvm::cl::cat(category));
static llvm::cl::list<std::string> argsIncludeList(
    "include",
    llvm::cl::desc(
        "Extra include location of header files(could use several times)"),
    llvm::cl::cat(category));

static llvm::cl::opt<std::string> argsPlantUML(
    "plantuml", llvm::cl::desc("Specify the full path of plantuml.jar file"),
    llvm::cl::cat(category));
static llvm::cl::opt<std::string> argsJava(
    "java",
    llvm::cl::desc("Specify the full path of java binary for plantuml.jar"),
    llvm::cl::init("java"), llvm::cl::cat(category));

static llvm::cl::opt<bool> argsShowBasicFields(
    "basic", llvm::cl::desc("Show basic members(false as default)"),
    llvm::cl::init(false), llvm::cl::cat(category));
static llvm::cl::opt<bool> argsVerbose(
    "verbose", llvm::cl::desc("Show more logs(false as default)"),
    llvm::cl::init(false), llvm::cl::cat(category));
static llvm::cl::opt<bool> argsModVersion(
    "modversion", llvm::cl::desc("Show the version of this"),
    llvm::cl::init(false), llvm::cl::cat(category));

static llvm::cl::opt<std::string> argsInput(
    "input", llvm::cl::desc("input a folder of a project or a single file"),
    llvm::cl::cat(category));
static llvm::cl::opt<std::string> argsOutput(
    "output",
    llvm::cl::desc(
        "Specify output basename(use the last part of input if not given)"),
    llvm::cl::init(""), llvm::cl::cat(category));


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

    fs::path path = fs::absolute(input);
    if (fs::is_regular_file(path)) {
        path = path.parent_path();
    }

    do {
        if (argsVerbose) {
            llvm::outs() << "Detecting " << (path / "compile_commands.json")
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

void ClassParser::grab(std::string input, bool src, bool inc)
{
    fs::path input_path(input);
    if (fs::is_regular_file(input_path)) {
        if (inc) {
            inc_folders.push_back(input_path.parent_path());
        }
        if (src) {
            cpp_files.push_back(input);
        }
    }
    else {
        if (argsVerbose) {
            if (inc) {
                llvm::outs() << "Grab files from " << input << " .\n";
            }
        }

        for (const auto &entry :
             fs::recursive_directory_iterator(fs::absolute(input))) {
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
        grab(input, false, true);
        for (const auto &inc : argsIncludeList) {
            grab(inc, false, true);
        }

        json cmpl_cmd = json::array();
        std::ostringstream incs;
        for (auto inc : inc_folders) {
            incs << " -I" << inc;
        }
        for (auto cpp : cpp_files) {
            json jfile         = json::object();
            jfile["directory"] = new_folder.string();
            std::ostringstream sscmd;
            sscmd << "/usr/bin/c++" << incs.str()
                  << " -std=c++17 -DDEBUG -O2 -c " << cpp;
            jfile["command"] = sscmd.str();
            jfile["file"]    = cpp;
            cmpl_cmd.push_back(jfile);
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

    for (int i = 0; i < argc; ++i) {
        fakeArgv.push_back(argv[i]);
    }

    input = argsInput;
    grab(input, true, false);

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
        llvm::outs() << "Generated arguments:\n";
        for (auto &c : fakeArgv) {
            llvm::outs() << c << "\n";
        }
    }
}

void ClassParser::makeDefaultOutput()
{
    fs::path inputPath(fs::canonical(input));
    llvm::outs() << "Use input name: " << inputPath << "\n";
    argsOutput = inputPath.filename().string();
    if (argsVerbose) {
        llvm::outs() << "Use output name: " << argsOutput << "\n";
    }
}

void ClassParser::parseProject(int argc, const char **argv)
{
    makeFakeArgs(argc, argv);

    int fakeArgc = fakeArgv.size();
    auto ExpectedParser = CommonOptionsParser::create(fakeArgc, fakeArgv.data(), category);
    if (!ExpectedParser) {
        llvm::errs() << "Error: " << llvm::toString(ExpectedParser.takeError()) << "\n";
        return;
    }
    CommonOptionsParser& op = ExpectedParser.get();

    if (argsOutput == "") {
        makeDefaultOutput();
    }

    std::unique_ptr<clang::tooling::FrontendActionFactory> actionFactory(
        new FindNamedClassActionFactory(classes(), argsVerbose,
                                        argsExcludeList));
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    Tool.run(actionFactory.get());
}

bool ClassParser::verbose() { return argsVerbose; }
std::string ClassParser::output() { return argsOutput; }
bool ClassParser::basic() { return argsShowBasicFields; }
std::string ClassParser::java() { return argsJava; }
std::string ClassParser::plantuml() { return argsPlantUML; }
json &ClassParser::classes() { return jclasses; }
