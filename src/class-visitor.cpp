#include "class-visitor.hpp"

#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool FindNamedClassVisitor::isBasic(QualType &type)
{
    bool is             = false;
    const Type *typePtr = type.getTypePtr();
    if (typePtr->isBuiltinType()) {
        is = true;
    }

    if (typePtr->isArrayType()) {
        const ArrayType *arrayType = dyn_cast<ArrayType>(typePtr);
        if (arrayType != NULL) {
            QualType elementType = arrayType->getElementType();

            if (elementType->isBuiltinType()) {
                is = true;
            }
        }
    }
    return is;
}

std::string FindNamedClassVisitor::getClassName(CXXRecordDecl *declaration)
{
    std::string name;
    if (declaration->isAnonymousStructOrUnion()) {
        if (const auto *typedefName
            = Context->getTypedefNameForUnnamedTagDecl(declaration)) {
            name = typedefName->getNameAsString();
        }
        else {
            name = "(anonymous)";
        }
    }
    else {
        name = declaration->getQualifiedNameAsString();
    }
    return name;
}

void FindNamedClassVisitor::grabBaseClasses(CXXRecordDecl *declaration,
                                            std::string name)
{
    if (declaration->hasDefinition()) {
        for (const auto &base : declaration->bases()) {
            auto baseType = base.getType();
            if (auto baseClassDecl = baseType->getAsCXXRecordDecl()) {
                classes[name]["bases"].push_back(
                    baseClassDecl->getQualifiedNameAsString());
                if (verbose) {
                    std::cout << "  Base class: "
                              << baseClassDecl->getQualifiedNameAsString()
                              << "\n";
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
            std::cout << "    Template (defined in C++ "
                         "standard library): ";
        }
    }
    else {
        jtempl["isBasic"] = false;
        if (verbose) {
            std::cout << "    Template (defined in "
                         "third-party library): ";
        }
    }
    jtempl["name"] = templateName;
    fld["templates"].push_back(jtempl);

    if (verbose) {
        std::cout << templateName << "\n";
    }

    for (const auto &arg : type->template_arguments()) {
        if (arg.getKind() == TemplateArgument::Type) {
            if (verbose) {
                std::cout << "    Template argument: ";
            }
            grabType(fld, arg.getAsType());
        }
    }
}

std::string FindNamedClassVisitor::trimType(std::string name)
{
    std::string ret   = name;
    auto removePrefix = [&](std::string prefix) {
        size_t pos = ret.find(prefix);
        if (pos != std::string::npos) {
            ret.replace(pos, prefix.length(), "");
        }
    };

    removePrefix("struct ");
    removePrefix("class ");
    removePrefix("enum ");
    removePrefix("union ");
    removePrefix("const ");

    return ret;
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
    else if (const auto *arrayType = desugaredType->getAsArrayTypeUnsafe()) {
        grabType(fld, arrayType->getElementType());
    }
    else {
        std::string typeName = type.getAsString();
        std::string kindName = "";

        const RecordType *recordType = NULL;
        const Type *typePtr          = type.getTypePtr();
        if (typePtr->isArrayType()) {
            const ArrayType *arrayType = dyn_cast<ArrayType>(typePtr);
            if (arrayType != NULL) {
                QualType elementType = arrayType->getElementType();
                recordType           = dyn_cast<RecordType>(elementType);
            }
        }
        else {
            recordType = dyn_cast<RecordType>(typePtr);
        }

        if (recordType != NULL) {
            RecordDecl *recordDecl = recordType->getDecl();
            kindName               = recordDecl->getKindName().data();
            typeName               = recordDecl->getNameAsString();
        }
        else {
            typeName = type.getAsString();
        }

        typeName = trimType(typeName);

        typeName.erase(std::remove(typeName.begin(), typeName.end(), ' '),
                       typeName.end());

        json jtype;
        jtype["isBasic"] = isBasic(type);
        jtype["name"]    = typeName;
        jtype["kind"]    = kindName;
        fld["types"].push_back(jtype);
        if (verbose) {
            std::cout << "    Type: " << type.getAsString() << "\n";
        }
    }
}

void FindNamedClassVisitor::storeFields(std::string name, clang::ValueDecl *var,
                                        bool flag)
{
    json fld    = json::object();
    fld["name"] = var->getNameAsString();
    if (verbose) {
        std::cout << "  Field: " << var->getQualifiedNameAsString() << "\n";
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

void FindNamedClassVisitor::grabFields(CXXRecordDecl *declaration,
                                       std::string name)
{
    std::function<void(json &, QualType)> grabType;

    for (auto field : declaration->fields()) {
        storeFields(name, field, false);
    }

    for (auto it = declaration->decls_begin(); it != declaration->decls_end();
         ++it) {
        if (clang::VarDecl *varDecl = clang::dyn_cast<clang::VarDecl>(*it)) {
            if (varDecl->isStaticDataMember()) {
                storeFields(name, varDecl, true);
            }
        }
    }
}

bool FindNamedClassVisitor::VisitCXXRecordDecl(CXXRecordDecl *declaration)
{
    if (!declaration->isCompleteDefinition()) {
        return true;
    }
    std::string location
        = declaration->getLocation().printToString(Context->getSourceManager());
    if (declaration->hasNameForLinkage()) {
        bool match = false;
        for (const auto &exclude : excludeList) {
            if (location.find(exclude) != std::string::npos) {
                match = true;
                break;
            }
        }
        if (!match) {
            std::string name = getClassName(declaration);
            if (name == "(anonymous)") {
                if (verbose) {
                    std::cout << "Ignore " << name << " at " << location
                              << "\n";
                }
                return true;
            }
            if (classes.find(name) != classes.end()) {
                return true;
            }

            classes[name]             = json::object();
            classes[name]["name"]     = name;
            classes[name]["kind"]     = declaration->getKindName().data();
            classes[name]["location"] = location;

            if (verbose) {
                std::cout << "Found class: "
                          << declaration->getQualifiedNameAsString() << "\n";
                std::cout << location << "\n";
            }

            grabBaseClasses(declaration, name);
            grabFields(declaration, name);
        }
    }
    return true;
}

bool FindNamedClassVisitor::VisitTypedefDecl(TypedefDecl *typedefDecl)
{
    std::string location
        = typedefDecl->getLocation().printToString(Context->getSourceManager());

    bool match = false;
    for (const auto &exclude : excludeList) {
        if (location.find(exclude) != std::string::npos) {
            match = true;
            break;
        }
    }
    if (!match) {
        QualType underlyingType = typedefDecl->getUnderlyingType();
        std::string name        = trimType(underlyingType.getAsString());

        if (const EnumType *enumType = underlyingType->getAs<EnumType>()) {
            std::string name   = typedefDecl->getNameAsString();
            EnumDecl *enumDecl = enumType->getDecl();
            VisitEnumDecl(enumDecl, name);
        }
        else {
            const RecordType *recordType = underlyingType->getAs<RecordType>();
            if (recordType != NULL) {
                RecordDecl *recordDecl = recordType->getDecl();
                if (recordDecl != NULL) {
                    std::string kind = recordDecl->getKindName().data();
                    if (kind == "struct" || kind == "union" || kind == "enum") {
                        classes[name]             = json::object();
                        classes[name]["name"]     = name;
                        classes[name]["kind"]     = kind;
                        classes[name]["location"] = location;
                        for (auto field = recordDecl->field_begin();
                             field != recordDecl->field_end(); ++field) {
                            storeFields(name, *field, false);
                        }
                    }
                }
            }
        }
    }

    return true;
}

bool FindNamedClassVisitor::VisitEnumDecl(EnumDecl *enumDecl)
{
    return VisitEnumDecl(enumDecl, "");
}

bool FindNamedClassVisitor::VisitEnumDecl(EnumDecl *enumDecl, std::string def)
{
    std::string location
        = enumDecl->getLocation().printToString(Context->getSourceManager());
    bool match = false;
    for (const auto &exclude : excludeList) {
        if (location.find(exclude) != std::string::npos) {
            match = true;
            break;
        }
    }
    if (!match) {
        std::string name = enumDecl->getNameAsString();
        if (name == "" && def != "") {
            name = def;
        }
        if (name.length() > 0) {
            classes[name]             = json::object();
            classes[name]["name"]     = name;
            classes[name]["kind"]     = "enum";
            classes[name]["location"] = location;
        }
    }

    return true;
}
