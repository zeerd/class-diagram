#ifndef CLASS_VISITOR_HPP
#define CLASS_VISITOR_HPP

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

class FindNamedClassVisitor
    : public RecursiveASTVisitor<FindNamedClassVisitor> {
private:
    bool isBasic(QualType &type);
    std::string getClassName(CXXRecordDecl *Declaration);
    void grabBaseClasses(CXXRecordDecl *Declaration, std::string name);
    void grabFields(CXXRecordDecl *Declaration, std::string name);
    void grabType(json &fld, QualType type);
    std::string trimType(std::string name);
    void grabTemplateType(json &fld, const TemplateSpecializationType *type);
    void storeFields(std::string name, clang::ValueDecl *var, bool flag);

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
    bool VisitTypedefDecl(TypedefDecl *typedefDecl);
    bool VisitEnumDecl(EnumDecl *enumDecl);
    bool VisitEnumDecl(EnumDecl *enumDecl, std::string name);

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

#endif /* CLASS_VISITOR_HPP */