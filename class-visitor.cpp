#include "class-diagram.hpp"

bool FindNamedClassVisitor::isBasic(QualType &type)
{
    bool is             = false;
    const Type *typePtr = type.getTypePtr();
    if (typePtr->isBuiltinType()) {
        is = true;
    }

    if (typePtr->isArrayType()) {
        const ArrayType *arrayType = dyn_cast<ArrayType>(typePtr);
        if(arrayType != NULL) {
            QualType elementType       = arrayType->getElementType();

            if (elementType->isBuiltinType()) {
                is = true;
            }
        }
    }
    return is;
}

std::string FindNamedClassVisitor::getClassName(CXXRecordDecl *Declaration)
{
    std::string name;
    if (Declaration->isAnonymousStructOrUnion()) {
        if (const auto *typedefName
            = Context->getTypedefNameForUnnamedTagDecl(Declaration)) {
            name = typedefName->getNameAsString();
        }
        else {
            name = "(anonymous)";
        }
    }
    else {
        name = Declaration->getQualifiedNameAsString();
    }
    return name;
}

void FindNamedClassVisitor::grabBaseClasses(CXXRecordDecl *Declaration,
                                            std::string name)
{
    if (Declaration->hasDefinition()) {
        for (const auto &base : Declaration->bases()) {
            auto baseType = base.getType();
            if (auto baseClassDecl = baseType->getAsCXXRecordDecl()) {
                classes[name]["bases"].push_back(
                    baseClassDecl->getQualifiedNameAsString());
                if (verbose) {
                    llvm::outs()
                        << "  Base class: "
                        << baseClassDecl->getQualifiedNameAsString() << "\n";
                }
            }
        }
    }
}

void FindNamedClassVisitor::grabTemplateType(
    json &fld, const TemplateSpecializationType *type)
{
    std::string templateName
        = type->getTemplateName().getAsTemplateDecl()->getNameAsString();

    const auto *namespaceDecl = type->getTemplateName()
                                    .getAsTemplateDecl()
                                    ->getDeclContext()
                                    ->getEnclosingNamespaceContext();

    json jtempl = json::object();
    if ((namespaceDecl && namespaceDecl->isStdNamespace())
        || templateName.rfind("std::", 0) == 0) {
        jtempl["isBasic"] = true;
        if (verbose) {
            llvm::outs() << "    Template (defined in C++ "
                            "standard library): ";
        }
    }
    else {
        jtempl["isBasic"] = false;
        if (verbose) {
            llvm::outs() << "    Template (defined in "
                            "third-party library): ";
        }
    }
    jtempl["name"] = templateName;
    fld["templates"].push_back(jtempl);

    if (verbose) {
        llvm::outs() << templateName << "\n";
    }

    for (const auto &arg : type->template_arguments()) {
        if (arg.getKind() == TemplateArgument::Type) {
            if (verbose) {
                llvm::outs() << "    Template argument: ";
            }
            grabType(fld, arg.getAsType());
        }
    }
}

void FindNamedClassVisitor::grabType(json &fld, QualType type)
{
    QualType desugaredType = type.getDesugaredType(*Context);

    const TemplateSpecializationType *specializedType;
    if (specializedType = type->getAs<TemplateSpecializationType>()) {
        grabTemplateType(fld, specializedType);
    }
    else if (const auto *elaboratedType
             = desugaredType->getAs<ElaboratedType>()) {
        grabType(fld, elaboratedType->getNamedType());
    }
    else if (const auto *typedefType = desugaredType->getAs<TypedefType>()) {
        grabType(fld, typedefType->desugar());
    }
    else if (const auto *pointerType = desugaredType->getAs<PointerType>()) {
        grabType(fld, pointerType->getPointeeType());
    }
    else {
        std::string typeName = type.getAsString();

        auto removePrefix = [&](std::string prefix) {
            size_t pos = typeName.find(prefix);
            if (pos != std::string::npos) {
                typeName.replace(pos, prefix.length(), "");
            }
        };
        removePrefix("struct ");
        removePrefix("class ");
        removePrefix("const ");

        typeName.erase(std::remove(typeName.begin(), typeName.end(), ' '),
                       typeName.end());

        json jtype;
        jtype["isBasic"] = isBasic(type);
        jtype["name"]    = typeName;
        fld["types"].push_back(jtype);
        if (verbose) {
            llvm::outs() << "    Type: " << type.getAsString() << "\n";
        }
    }
}

void FindNamedClassVisitor::storeMember(std::string name, clang::ValueDecl *var,
                                        bool flag)
{
    json fld    = json::object();
    fld["name"] = var->getNameAsString();
    if (verbose) {
        llvm::outs() << "  Field: " << var->getQualifiedNameAsString() << "\n";
    }

    QualType fieldType = var->getType();
    fld["type"]        = fieldType.getAsString();
    fld["isBasic"]     = isBasic(fieldType);

    const Type *typePtr = fieldType.getTypePtr();
    if (typePtr->isPointerType() || typePtr->isReferenceType()) {
        fld["is_aggregate"] = true;
    }
    else {
        fld["is_aggregate"] = false;
    }
    fld["is_static"] = flag;

    grabType(fld, fieldType);
    classes[name]["fields"].push_back(fld);
};

void FindNamedClassVisitor::grabFields(CXXRecordDecl *Declaration,
                                       std::string name)
{
    std::function<void(json &, QualType)> grabType;

    for (auto field : Declaration->fields()) {
        storeMember(name, field, false);
    }

    for (auto it = Declaration->decls_begin(); it != Declaration->decls_end();
         ++it) {
        if (clang::VarDecl *varDecl = clang::dyn_cast<clang::VarDecl>(*it)) {
            if (varDecl->isStaticDataMember()) {
                storeMember(name, varDecl, true);
            }
        }
    }
}

bool FindNamedClassVisitor::VisitCXXRecordDecl(CXXRecordDecl *Declaration)
{
    if (!Declaration->isCompleteDefinition()) {
        return true;
    }
    std::string location
        = Declaration->getLocation().printToString(Context->getSourceManager());
    if (Declaration->hasNameForLinkage()) {
        bool match = false;
        for (const auto &exclude : excludeList) {
            if (location.find(exclude) != std::string::npos) {
                match = true;
                break;
            }
        }
        if (!match) {
            std::string name = getClassName(Declaration);
            if (name == "(anonymous)") {
                return true;  // FIXME
            }
            if (classes.find(name) != classes.end()) {
                return true;
            }

            classes[name]             = json::object();
            classes[name]["name"]     = name;
            classes[name]["location"] = location;

            if (verbose) {
                llvm::outs() << "Found class: "
                             << Declaration->getQualifiedNameAsString() << "\n";
                llvm::outs() << location << "\n";
            }

            grabBaseClasses(Declaration, name);
            grabFields(Declaration, name);
        }
    }
    return true;
}
