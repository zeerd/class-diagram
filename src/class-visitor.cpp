#include "class-visitor.hpp"

#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool FindNamedClassVisitor::isInner(const RecordType *recordType)
{
    if (recordType != NULL) {
        RecordDecl *recordDecl = recordType->getDecl();
        if (recordDecl->isThisDeclarationADefinition()
            && recordDecl->getDeclContext()->isRecord()) {
            return true;
        }
    }
    return false;
}

std::string FindNamedClassVisitor::getAnonymousType(
    const RecordType *recordType)
{
    if (recordType) {
        RecordDecl *recordDecl = recordType->getDecl();
        if (recordDecl->isAnonymousStructOrUnion()) {
            if (recordDecl->isUnion()) {
                return "union";
            }
            else if (recordDecl->isStruct()) {
                return "struct";
            }
        }
    }
    return "";
}

bool FindNamedClassVisitor::isReduced(QualType &type)
{
    if (verbose) {
        std::cout << "Reduce{";
    }

    bool is             = false;
    const Type *typePtr = type.getTypePtr();
    if (typePtr->isBuiltinType()) {
        if (verbose) {
            std::cout << "(Builtin)";
        }
        is = true;
    }
    if (!is) {
        QualType desugaredType = type.getDesugaredType(*Context);
        if (const auto *recordType = desugaredType->getAs<RecordType>()) {
            RecordDecl *recordDecl    = recordType->getDecl();
            std::string qualifiedName = recordDecl->getQualifiedNameAsString();
            size_t pos                = qualifiedName.rfind("::");
            if (pos != std::string::npos) {
                if (verbose) {
                    std::cout << "(NS)";
                }
                std::string namespaceName = qualifiedName.substr(0, pos);
                if (namespaceName == "std") {
                    if (!isa<ClassTemplateSpecializationDecl>(recordDecl)) {
                        if (verbose) {
                            std::cout << "(!Template)";
                        }
                        is = true;
                    }
                }
                else if (std::find(excludeNsList.begin(), excludeNsList.end(),
                                   namespaceName)
                         != excludeNsList.end()) {
                    if (verbose) {
                        std::cout << "(excludeNs)";
                    }
                    is = true;
                }
            }
        }
    }
    if (!is && typePtr->isArrayType()) {
        const ArrayType *arrayType = dyn_cast<ArrayType>(typePtr);
        if (arrayType != NULL) {
            QualType elementType = arrayType->getElementType();

            if (elementType->isBuiltinType()) {
                if (verbose) {
                    std::cout << "(ArrayType)";
                }
                is = true;
            }
        }
    }
    if (!is) {
        if (const auto *pointerType = typePtr->getAs<PointerType>()) {
            QualType pointeeType = pointerType->getPointeeType();
            if (pointeeType->isBuiltinType()) {
                if (verbose) {
                    std::cout << "(PointerType)";
                }
                is = true;
            }
        }
    }
    if (!is) {
        QualType desugaredType = type.getDesugaredType(*Context);
        if (const auto *pointerType = desugaredType->getAs<PointerType>()) {
            if (pointerType->getPointeeType()->isFunctionType()) {
                if (verbose) {
                    std::cout << "(FunctionPointerType)";
                }
                is = true;
            }
        }
        if (!is) {
            // class function pointer
            if (const auto *functionProtoType
                = desugaredType->getAs<FunctionProtoType>()) {
                if (verbose) {
                    std::cout << "(FunctionProtoType)";
                }
                is = true;
            }
        }
    }

    if (verbose) {
        std::cout << "}(" << (is ? "true" : "false") << ") ";
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

bool FindNamedClassVisitor::isStdTemplate(QualType type)
{
    return isStdTemplate(type->getAs<TemplateSpecializationType>());
}

bool FindNamedClassVisitor::isStdTemplate(
    const TemplateSpecializationType *type)
{
    bool ret = false;
    if (type) {
        auto templateDecl        = type->getTemplateName().getAsTemplateDecl();
        std::string templateName = templateDecl->getNameAsString();
        const auto *namespaceDecl
            = templateDecl->getDeclContext()->getEnclosingNamespaceContext();

        if ((namespaceDecl && namespaceDecl->isStdNamespace())
            || templateName.rfind("std::", 0) == 0) {
            ret = true;
        }
    }
    return ret;
}

void FindNamedClassVisitor::grabTemplateType(
    json &fld, const TemplateSpecializationType *type)
{
    std::string templateName
        = type->getTemplateName().getAsTemplateDecl()->getNameAsString();

    json jtempl = json::object();
    if (isStdTemplate(type)) {
        jtempl["isReduced"] = true;
        if (verbose) {
            std::cout << "    Template (standard): ";
        }
    }
    else {
        jtempl["isReduced"] = false;
        if (verbose) {
            std::cout << "    Template (third-party): ";
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
    removePrefix("volatile ");

    return ret;
}

void FindNamedClassVisitor::grabFinalType(json &fld, QualType type)
{
    std::string typeName = "";
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
        if (isInner(recordType)) {
            if (verbose) {
                std::cout << " (inner) ";
            }
            kindName = getAnonymousType(recordType);
            if (kindName == "") {
                kindName = "class";
                typeName = type.getAsString();
            }
            else {
                if (verbose) {
                    std::cout << " (anonymous) ";
                }
                typeName = "";
            }
        }
        else {
            RecordDecl *recordDecl = recordType->getDecl();
            kindName               = recordDecl->getKindName().data();
            typeName               = recordDecl->getNameAsString();
        }
    }
    else {
        typeName = type.getAsString();
    }

    typeName = trimType(typeName);

    typeName.erase(std::remove(typeName.begin(), typeName.end(), ' '),
                   typeName.end());

    if (verbose) {
        std::cout << type.getAsString() << "\n";
    }

    if (verbose) {
        std::cout << "      Sub-Types: ";
    }

    json jtype;
    jtype["isReduced"] = isReduced(type);
    jtype["name"]      = typeName;
    jtype["kind"]      = kindName;
    fld["types"].push_back(jtype);
    if (verbose) {
        std::cout << typeName << "\n";
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
        if (verbose) {
            std::cout << " (ElaboratedType) ";
        }
        grabType(fld, elaboratedType->getNamedType());
    }
    else if (const auto *typedefType = desugaredType->getAs<TypedefType>()) {
        if (verbose) {
            std::cout << " (e) ";
        }
        grabType(fld, typedefType->desugar());
    }
    else if (const auto *pointerType = desugaredType->getAs<PointerType>()) {
        QualType pointeeType = pointerType->getPointeeType();
        if (pointeeType->isFunctionType()) {
            grabFinalType(fld, type);
        }
        else {
            if (verbose) {
                std::cout << " (PointerType) ";
            }
            grabType(fld, pointeeType);
        }
    }
    else if (const auto *arrayType = desugaredType->getAsArrayTypeUnsafe()) {
        if (verbose) {
            std::cout << " (ArrayType) ";
        }
        grabType(fld, arrayType->getElementType());
    }
    else if (const auto *referenceType
             = desugaredType->getAs<ReferenceType>()) {
        QualType referredType = referenceType->getPointeeType();
        if (verbose) {
            std::cout << " (ReferenceType) ";
        }
        grabType(fld, referredType);
    }
    else if (const auto *memberPointerType
             = desugaredType->getAs<MemberPointerType>()) {
        if (memberPointerType->getPointeeType()->isFunctionType()) {
            // This is a pointer to a member function
            const Type *classType  = memberPointerType->getClass();
            QualType qualClassType = QualType::getFromOpaquePtr(classType);

            if (verbose) {
                std::cout << " (MemberPointerType) ";
            }
            grabType(fld, qualClassType);
        }
    }
    else {
        grabFinalType(fld, type);
    }
}

void FindNamedClassVisitor::storeFields(std::string name, clang::ValueDecl *var,
                                        bool flag)
{
    QualType fieldType           = var->getType();
    const RecordType *recordType = fieldType->getAs<RecordType>();

    json fld    = json::object();
    fld["name"] = var->getNameAsString();
    if (verbose) {
        std::cout << "  Field: " << var->getQualifiedNameAsString() << "\n";
        std::cout << "    Type: ";
    }

    std::string anonymous = getAnonymousType(recordType);
    if (anonymous == "") {
        fld["type"] = fieldType.getAsString();
    }
    else {
        fld["type"] = anonymous;
        fld["name"] = "(anonymous)";
    }
    fld["isReduced"] = isReduced(fieldType);

    const Type *typePtr = fieldType.getTypePtr();
    if (typePtr->isPointerType() || typePtr->isReferenceType()) {
        fld["is_aggregate"] = true;
    }
    else {
        fld["is_aggregate"] = false;
    }
    fld["is_static"] = flag;

    if (isInner(recordType)) {
        // Keep full name with namespace and outer-class
        // because PlantUML has no inner-class
        grabFinalType(fld, fieldType);
    }
    else {
        grabType(fld, fieldType);
    }
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

            DeclContext *context = declaration->getDeclContext();
            if (context && context->isRecord()) {
                classes[name]["isInner"] = true;
            }
            else {
                classes[name]["isInner"] = false;
            }

            if (verbose) {
                std::cout << "Found class: "
                          << declaration->getQualifiedNameAsString() << "\n";
                std::cout << "  " << location << "\n";
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
                        classes[name]["isInner"]  = false;
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
        std::string name;
        if (def != "") {
            name = def;
        }
        else {
            name = enumDecl->getQualifiedNameAsString();
        }
        if (name.length() > 0) {
            classes[name]             = json::object();
            classes[name]["name"]     = name;
            classes[name]["kind"]     = "enum";
            classes[name]["location"] = location;
            classes[name]["isInner"]  = false;
        }
    }

    return true;
}
