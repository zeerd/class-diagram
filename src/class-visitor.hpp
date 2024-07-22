#ifndef CLASS_VISITOR_HPP
#define CLASS_VISITOR_HPP

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace clang;
using namespace clang::tooling;

class FindNamedClassVisitor
    : public RecursiveASTVisitor<FindNamedClassVisitor> {
private:
    void newClass(std::string name);
    std::string visibility(clang::ValueDecl *var);

    void configReduced(json &obj, QualType &type, bool deep = false);
    void configClassReduced(json &obj);
    std::pair<bool, std::string> checkReduce(QualType &type, bool deep = false);
    bool isNamespaceReduced(QualType &type, std::string &reason);
    bool isNamespaceReduced(std::string name, std::string &reason);
    bool isInner(const RecordType *recordType);
    bool isStdTemplate(const TemplateSpecializationType *type);
    bool isExcludedType(QualType &type, std::string &reason);
    bool isBuiltinArrayType(QualType &type);
    bool isBuiltinPointerType(QualType &type);
    bool isFunctionType(QualType &type);
    bool isTemplateParmType(QualType type);
    std::string getAnonymousType(const RecordType *recordType);

    bool isExcluded(std::string loc, std::string &reason);
    std::string trimType(const std::string &name);

    std::string getNamespace(const Decl *declaration);
    void grabBaseClasses(CXXRecordDecl *declaration, std::string clz);

    void grabFields(CXXRecordDecl *declaration, const std::string &name);
    void grabType(json &fld, QualType type);
    void grabFinalType(json &fld, QualType type);
    void storeFields(std::string name, clang::ValueDecl *var, bool flag);

    void grabTemplates(CXXRecordDecl *declaration, const std::string &name);
    void grabTemplateType(json &fld, const TemplateSpecializationType *type);
    void grabTemplateFields(json &fld, const TemplateSpecializationType *type);
    const TemplateSpecializationType *grabDeepTemplateType(
        const TemplateSpecializationType *type);

    bool VisitEnumDecl(EnumDecl *enumDecl, const std::string &location,
                       const std::string &ns, const std::string &name);

public:
    explicit FindNamedClassVisitor(ASTContext *Context);

    bool VisitCXXRecordDecl(CXXRecordDecl *declaration);
    bool VisitTypedefDecl(TypedefDecl *typedefDecl);
    bool VisitEnumDecl(EnumDecl *enumDecl);

private:
    ASTContext *Context;
    json &classes;
    bool verbose;
    std::vector<std::string> reduceList;
    std::vector<std::string> reduceNsList;
};

class FindNamedClassConsumer : public clang::ASTConsumer {
public:
    explicit FindNamedClassConsumer(ASTContext *Context)
        : Visitor(new FindNamedClassVisitor(Context))
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
    FindNamedClassAction() {}
    virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance &Compiler, llvm::StringRef InFile)
    {
        return std::unique_ptr<clang::ASTConsumer>(
            new FindNamedClassConsumer(&Compiler.getASTContext()));
    }
};

class FindNamedClassActionFactory
    : public clang::tooling::FrontendActionFactory {
public:
    FindNamedClassActionFactory() {}

    virtual std::unique_ptr<FrontendAction> create() override
    {
        return std::make_unique<FindNamedClassAction>();
    }
};

#endif /* CLASS_VISITOR_HPP */
