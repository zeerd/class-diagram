#ifndef CLASS_DIAGRAM_HPP
#define CLASS_DIAGRAM_HPP

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <vector>

using json = nlohmann::json;
using namespace clang;
using namespace clang::tooling;
namespace fs = std::filesystem;

class FindNamedClassVisitor
    : public RecursiveASTVisitor<FindNamedClassVisitor> {
private:
    bool isBasic(QualType &type);
    std::string getClassName(CXXRecordDecl *Declaration);
    void grabBaseClasses(CXXRecordDecl *Declaration, std::string name);
    void grabFields(CXXRecordDecl *Declaration, std::string name);
    void grabType(json &fld, QualType type);
    void grabTemplateType(json &fld, const TemplateSpecializationType *type);
    void storeMember(std::string name, clang::ValueDecl *var, bool flag);

public:
    explicit FindNamedClassVisitor(ASTContext *Context, json &classes,
                                   bool verb,
                                   llvm::cl::list<std::string> &excludeList)
        : Context(Context),
          classes(classes),
          verbose(verb),
          excludeList(excludeList)
    {
    }

    bool VisitCXXRecordDecl(CXXRecordDecl *Declaration);

private:
    ASTContext *Context;
    json &classes;
    bool verbose;
    llvm::cl::list<std::string> &excludeList;
};

class FindNamedClassConsumer : public clang::ASTConsumer {
public:
    explicit FindNamedClassConsumer(ASTContext *Context, json &classes,
                                    bool verb,
                                    llvm::cl::list<std::string> &excludeList)
        : Visitor(
              new FindNamedClassVisitor(Context, classes, verb, excludeList))
    {
    }

    virtual void HandleTranslationUnit(clang::ASTContext &Context)
    {
        Visitor->TraverseDecl(Context.getTranslationUnitDecl());
    }

private:
    FindNamedClassVisitor *Visitor;
};

class FindNamedClassAction : public clang::ASTFrontendAction {
public:
    FindNamedClassAction(json &classes, bool verb,
                         llvm::cl::list<std::string> &excludeList)
        : classes(classes), verbose(verb), excludeList(excludeList)
    {
    }
    virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance &Compiler, llvm::StringRef InFile)
    {
        return std::unique_ptr<clang::ASTConsumer>(new FindNamedClassConsumer(
            &Compiler.getASTContext(), classes, verbose, excludeList));
    }

private:
    json &classes;
    bool verbose;
    llvm::cl::list<std::string> &excludeList;
};

class FindNamedClassActionFactory
    : public clang::tooling::FrontendActionFactory {
public:
    FindNamedClassActionFactory(json &classes, bool verb,
                                llvm::cl::list<std::string> &excludeList)
        : classes(classes), verbose(verb), excludeList(excludeList)
    {
    }

    virtual std::unique_ptr<FrontendAction> create() override
    {
        return std::make_unique<FindNamedClassAction>(classes, verbose,
                                                      excludeList);
    }

private:
    json &classes;
    bool verbose;
    llvm::cl::list<std::string> &excludeList;
};

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

#endif /* CLASS_DIAGRAM_HPP */
