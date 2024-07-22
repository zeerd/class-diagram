#include "class-visitor.hpp"

#include <iostream>

#include "class-parser.hpp"

using json = nlohmann::json;

FindNamedClassVisitor::FindNamedClassVisitor(ASTContext *Context)
    : Context(Context),
      classes(ClassParser::classes()),
      verbose(ClassParser::verbose()),
      reduceList(ClassParser::reduceList()),
      reduceNsList(ClassParser::reduceNsList())
{
}

std::string FindNamedClassVisitor::visibility(clang::ValueDecl *var)
{
    clang::AccessSpecifier access = var->getAccess();
    std::string visibility;
    switch (access) {
        case clang::AS_public:
            visibility = "public";
            break;
        case clang::AS_protected:
            visibility = "protected";
            break;
        case clang::AS_private:
            visibility = "private";
            break;
        // LCOV_EXCL_START
        default:
            visibility = "unknown";
            break;
            // LCOV_EXCL_STOP
    }
    return visibility;
}

void FindNamedClassVisitor::newClass(std::string name)
{
    classes[name]               = json::object();
    classes[name]["name"]       = name;
    classes[name]["typedef"]    = "";
    classes[name]["kind"]       = "";
    classes[name]["location"]   = "";
    classes[name]["namespace"]  = "";
    classes[name]["isInner"]    = false;
    classes[name]["isComplete"] = true;
    classes[name]["isReduced"]  = false;
}

bool FindNamedClassVisitor::isInner(const RecordType *recordType)
{
    bool is = false;
    if (recordType != NULL) {
        RecordDecl *recordDecl = recordType->getDecl();
        if (recordDecl != NULL && recordDecl->getDeclContext()->isRecord()) {
            /* the context of this is a record(means:class/struct/union).
               so, it's a inner-xxx */
            if (verbose) {
                std::cout << "(inner)";
            }
            is = true;
        }
    }
    return is;
}

std::string FindNamedClassVisitor::getAnonymousType(
    const RecordType *recordType)
{
    std::string type = "";
    if (recordType) {
        RecordDecl *recordDecl = recordType->getDecl();
        if (recordDecl->isAnonymousStructOrUnion()) {
            if (recordDecl->isUnion()) {
                type = "union";
            }
            else if (recordDecl->isStruct()) {
                type = "struct";
            }
        }
    }
    return type;
}

bool FindNamedClassVisitor::isExcluded(std::string loc, std::string &reason)
{
    bool match = false;
    for (const auto &exclude : reduceList) {
        if (loc.find(exclude) != std::string::npos) {
            match  = true;
            reason = "(ReduceList)";
            break;
        }
    }
    return match;
}

bool FindNamedClassVisitor::isExcludedType(QualType &type, std::string &reason)
{
    bool is = false;
    if (const clang::TypeDecl *typeDecl = type->getAsTagDecl()) {
        if (const clang::RecordDecl *recordDecl
            = clang::dyn_cast<clang::RecordDecl>(typeDecl)) {
            if (const clang::RecordDecl *definition
                = recordDecl->getDefinition()) {
                std::string location = definition->getLocation().printToString(
                    Context->getSourceManager());
                is = isExcluded(location, reason);
            }
        }
    }
    return is;
}

bool FindNamedClassVisitor::isBuiltinArrayType(QualType &type)
{
    bool is             = false;
    const Type *typePtr = type.getTypePtr();
    if (typePtr->isArrayType()) {
        const ArrayType *arrayType = dyn_cast<ArrayType>(typePtr);
        if (arrayType != NULL) {
            QualType elementType = arrayType->getElementType();
            is                   = elementType->isBuiltinType();
        }
    }
    return is;
}
bool FindNamedClassVisitor::isBuiltinPointerType(QualType &type)
{
    bool is             = false;
    const Type *typePtr = type.getTypePtr();
    if (const auto *pointerType = typePtr->getAs<PointerType>()) {
        QualType pointeeType = pointerType->getPointeeType();
        is                   = pointeeType->isBuiltinType();
    }
    return is;
}

bool FindNamedClassVisitor::isFunctionType(QualType &type)
{
    bool is                = false;
    QualType desugaredType = type.getDesugaredType(*Context);
    if (const auto *pointerType = desugaredType->getAs<PointerType>()) {
        if (pointerType->getPointeeType()->isFunctionType()) {
            is = true;
        }
    }
    if (!is) {
        // class function pointer
        if (desugaredType->getAs<FunctionProtoType>()) {
            is = true;
        }
    }
    return is;
}

bool FindNamedClassVisitor::isNamespaceReduced(std::string name,
                                               std::string &reason)
{
    bool is    = false;
    size_t pos = name.rfind("::");
    if (pos != std::string::npos) {
        std::string ns = name.substr(0, pos);
        is = (std::find(reduceNsList.begin(), reduceNsList.end(), ns)
              != reduceNsList.end());
        if (is) {
            reason = "(ReduceNsList)";
        }
    }
    return is;
}

bool FindNamedClassVisitor::isNamespaceReduced(QualType &type,
                                               std::string &reason)
{
    bool is                = false;
    QualType desugaredType = type.getDesugaredType(*Context);
    if (const auto *recordType = desugaredType->getAs<RecordType>()) {
        RecordDecl *recordDecl    = recordType->getDecl();
        std::string qualifiedName = recordDecl->getQualifiedNameAsString();
        size_t pos                = qualifiedName.rfind("::");
        if (pos != std::string::npos) {
            std::string namespaceName = qualifiedName.substr(0, pos);
            if (namespaceName == "std") {
                if (!isa<ClassTemplateSpecializationDecl>(recordDecl)) {
                    is     = true;
                    reason = "(NS)(!Template)";
                }
            }
            else {
                is = isNamespaceReduced(qualifiedName, reason);
            }
        }
    }
    return is;
}

bool FindNamedClassVisitor::isTemplateParmType(QualType type)
{
    bool is                = false;
    QualType desugaredType = type.getDesugaredType(*Context);
    if (desugaredType->getAs<TemplateTypeParmType>()) {
        is = true;
    }
    return is;
}

std::pair<bool, std::string> FindNamedClassVisitor::checkReduce(QualType &type,
                                                                bool deep)
{
    if (verbose) {
        std::cout << " Reduce{";
    }

    bool is             = true;
    std::string reason  = "";
    const Type *typePtr = type.getTypePtr();
    do {
        if (typePtr->isBuiltinType()) {
            reason = "(Builtin)";
            break;
        }
        if (llvm::isa<clang::AutoType>(typePtr)) {
            reason = "(AutoType)";
            break;
        }
        if (deep && isExcludedType(type, reason)) {
            break;
        }
        if (isBuiltinArrayType(type)) {
            reason = "(BuiltinArrayType)";
            break;
        }
        if (isBuiltinPointerType(type)) {
            reason = "(BuiltinPointerType)";
            break;
        }
        if (isFunctionType(type)) {
            reason = "(FunctionType)";
            break;
        }
        if (isNamespaceReduced(type, reason)) {
            break;
        }
        if (deep && isTemplateParmType(type)) {
            reason = "(TemplateParmType)";
            break;
        }
        if (type->getAs<DependentNameType>()) {
            reason = "(DependentNameType)";
            break;
        }

        is = false;
    } while (0);

    if (verbose) {
        std::cout << reason << "}";
    }

    return std::make_pair(is, reason);
}

void FindNamedClassVisitor::configReduced(json &obj, QualType &type, bool deep)
{
    std::pair<bool, std::string> ret = checkReduce(type, deep);
    obj["isReduced"]                 = ret.first;
    obj["utils"]["ReducedReason"]    = ret.second;
}

void FindNamedClassVisitor::grabBaseClasses(CXXRecordDecl *declaration,
                                            std::string clz)
{
    if (declaration->hasDefinition()) {
        for (const auto &base : declaration->bases()) {
            auto baseType = base.getType();
            if (auto baseClassDecl = baseType->getAsCXXRecordDecl()) {
                std::string name = baseClassDecl->getQualifiedNameAsString();
                classes[clz]["bases"].push_back(name);
                if (verbose) {
                    std::cout << "  Base class: " << name << "\n";
                }
            }
        }
    }
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

const TemplateSpecializationType *FindNamedClassVisitor::grabDeepTemplateType(
    const TemplateSpecializationType *type)
{
    QualType desugaredType = type->desugar();
    if (type->isSugared()) {
        if (const TemplateSpecializationType *specializedType
            = desugaredType->getAs<TemplateSpecializationType>()) {
            return grabDeepTemplateType(specializedType);
        }
        else {
            return type;
        }
    }
    else {
        return type;
    }
}

void FindNamedClassVisitor::grabTemplateFields(
    json &fld, const TemplateSpecializationType *type)
{
    for (const auto &arg : type->template_arguments()) {
        if (verbose) {
            std::cout << "        Template argument:";
        }
        if (arg.getKind() == TemplateArgument::Type) {
            grabType(fld, arg.getAsType());
        }
        else if (arg.getKind() == TemplateArgument::Template) {
            TemplateDecl *td = const_cast<TemplateDecl *>(
                arg.getAsTemplate().getAsTemplateDecl());
            if (td != nullptr) {
                ClassTemplateDecl *classTemplateDecl
                    = dyn_cast<ClassTemplateDecl>(td);
                if (classTemplateDecl != nullptr) {
                    QualType instantiatedType
                        = classTemplateDecl
                              ->getInjectedClassNameSpecialization();
                    grabType(fld, instantiatedType);
                }
            }
        }
        else {
            // LCOV_EXCL_START
            if (verbose) {
                std::cout << " kind=" << arg.getKind() << "\n";
            }
            // LCOV_EXCL_STOP
        }
    }
}

void FindNamedClassVisitor::grabTemplateType(
    json &fld, const TemplateSpecializationType *type)
{
    const TemplateSpecializationType *templ = grabDeepTemplateType(type);
    TemplateDecl *templateDecl = templ->getTemplateName().getAsTemplateDecl();

    json jtempl              = json::object();
    std::string templateName = templateDecl->getNameAsString();
    if (isStdTemplate(templ)) {
        if (verbose) {
            std::cout << " Template{standard}: ";
        }
        jtempl["isReduced"]              = true;
        jtempl["utils"]["ReducedReason"] = "Template{standard}";
    }
    else {
        if (verbose) {
            std::cout << " Template{third-party}: ";
        }
        if (isa<TemplateTemplateParmDecl>(templateDecl)) {
            jtempl["isReduced"]              = true;
            jtempl["utils"]["ReducedReason"] = "(TemplateTemplateParm)";
        }
        else {
            jtempl["isReduced"] = false;
        }
    }
    jtempl["type"]      = templateName;
    jtempl["namespace"] = "";
    fld["templates"].push_back(jtempl);

    if (verbose) {
        std::cout << templateName << "\n";
    }

    grabTemplateFields(fld, templ);
}

std::string FindNamedClassVisitor::trimType(const std::string &name)
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
    removePrefix("typename ");

    auto start = std::find_if_not(ret.begin(), ret.end(), [](unsigned char c) {
        return std::isspace(c);
    });
    auto end = std::find_if_not(ret.rbegin(), ret.rend(), [](unsigned char c) {
                   return std::isspace(c);
               }).base();

    ret = std::string(start, end);

    return ret;
}

void FindNamedClassVisitor::grabFinalType(json &fld, QualType type)
{
    std::string typeName      = "";
    std::string kindName      = "";
    std::string namespaceName = "";

    if (verbose) {
        std::cout << " ";
    }

    const RecordType *recordType = dyn_cast<RecordType>(type.getTypePtr());
    if (recordType != NULL) {
        if (verbose) {
            std::cout << "(RecordType)";
        }
        RecordDecl *recordDecl = recordType->getDecl();
        namespaceName          = getNamespace(recordDecl);
        if (isInner(recordType)) {
            kindName = getAnonymousType(recordType);
            if (kindName == "") {
                kindName = "class";
                typeName = type.getAsString();
            }
            else {
                if (verbose) {
                    std::cout << "(anonymous)";
                }
                typeName = "";
            }
        }
        else {
            kindName = recordDecl->getKindName().data();
            typeName = recordDecl->getNameAsString();
            if (typeName == "") {
                typeName = type.getAsString();
            }
        }
    }
    else if (const auto *enumType = type->getAs<EnumType>()) {
        if (verbose) {
            std::cout << " (EnumType) ";
        }
        kindName           = "enum";
        EnumDecl *enumDecl = enumType->getDecl();
        if (enumDecl) {
            namespaceName = getNamespace(enumDecl);
            typeName      = enumDecl->getQualifiedNameAsString();
        }
        if (typeName == "" || typeName == "(anonymous)") {
            QualType desugaredType = type.getDesugaredType(*Context);
            typeName               = desugaredType.getAsString();
        }
    }
    else {
        QualType desugaredType = type.getDesugaredType(*Context);
        typeName               = desugaredType.getAsString();
        // Type::TypeClass typeClass =
        // desugaredType.getTypePtr()->getTypeClass(); kindName =
        // std::to_string(typeClass);
    }

    typeName = trimType(typeName);

    json jtype;
    jtype["type"]      = typeName;
    jtype["kind"]      = kindName;
    jtype["namespace"] = namespaceName;
    configReduced(jtype, type, true);
    fld["types"].push_back(jtype);
    if (verbose) {
        std::cout << " '" << typeName << "'\n";
    }
}

void FindNamedClassVisitor::grabType(json &fld, QualType type)
{
    QualType desugaredType = type.getDesugaredType(*Context);
    if (verbose) {
        Type::TypeClass typeClass = type.getTypePtr()->getTypeClass();
        std::cout << "(" << typeClass << ":" << type.getAsString() << ")";
        // std::cout << "(" << Type::TemplateSpecialization << ")";
    }

    do {
        if (const auto *elaboratedType = type->getAs<ElaboratedType>()) {
            if (verbose) {
                std::cout << " (ElaboratedType) ";
            }
            grabType(fld, elaboratedType->getNamedType());
            break;
        }
        if (const auto *typedefType = type->getAs<TypedefType>()) {
            if (verbose) {
                std::cout << " (TypedefType) ";
            }
            if (type->isBuiltinType()) {
                grabFinalType(fld, type);
            }
            else {
                QualType underlyingType
                    = typedefType->getDecl()->getUnderlyingType();
                grabType(fld, underlyingType);
            }
            break;
        }
        if (const auto *decltypeType = type->getAs<DecltypeType>()) {
            if (verbose) {
                std::cout << " (DecltypeType ";
            }
            QualType underlyingType = decltypeType->getUnderlyingType();
            if (decltypeType->isSugared()) {
                grabType(fld, decltypeType->desugar());
            }
            else {
                grabType(fld, underlyingType);
            }
            break;
        }
        if (type->getAs<AutoType>()) {
            if (verbose) {
                std::cout << " (AutoType) ";
            }
            grabFinalType(fld, type);
            break;
        }
        if (const auto *pointerType = desugaredType->getAs<PointerType>()) {
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
            break;
        }
        if (const auto *arrayType = desugaredType->getAsArrayTypeUnsafe()) {
            if (verbose) {
                std::cout << " (ArrayType) ";
            }
            grabType(fld, arrayType->getElementType());
            break;
        }
        if (const auto *referenceType = desugaredType->getAs<ReferenceType>()) {
            QualType referredType = referenceType->getPointeeType();
            if (verbose) {
                std::cout << " (ReferenceType) ";
            }
            grabType(fld, referredType);
            break;
        }
        if (const auto *memberPointerType
            = desugaredType->getAs<MemberPointerType>()) {
            if (memberPointerType->getPointeeType()->isFunctionType()) {
                // This is a pointer to a member function
                const Type *classType  = memberPointerType->getClass();
                QualType qualClassType = QualType::getFromOpaquePtr(classType);

                if (verbose) {
                    std::cout << " (MemberPointerType) ";
                }
                grabType(fld, qualClassType);
                break;
            }
        }
        if (const InjectedClassNameType *injectedClassNameType
            = type->getAs<InjectedClassNameType>()) {
            if (verbose) {
                std::cout << " (InjectedClassNameType) ";
            }
            grabFinalType(
                fld, injectedClassNameType->getInjectedSpecializationType());
            break;
        }
        if (const TemplateSpecializationType *specializedType
            = type->getAs<TemplateSpecializationType>()) {
            grabTemplateType(fld, specializedType);
            break;
        }

        grabFinalType(fld, type);
    } while (0);
}

void FindNamedClassVisitor::storeFields(std::string name, clang::ValueDecl *var,
                                        bool flag)
{
    QualType fieldType           = var->getType();
    const RecordType *recordType = fieldType->getAs<RecordType>();

    json fld          = json::object();
    fld["name"]       = var->getNameAsString();
    fld["visibility"] = visibility(var);
    if (verbose) {
        std::cout << "  Field: " << var->getQualifiedNameAsString() << "\n";
        std::cout << "    Type: ";
    }

    bool inner       = isInner(recordType);
    std::string type = getAnonymousType(recordType);
    if (type == "") {
        type = fieldType.getAsString();
        if (recordType != NULL) {
            RecordDecl *recordDecl = recordType->getDecl();
            if (recordDecl != NULL && recordDecl->getDeclContext()->isRecord()
                && recordDecl->getQualifiedNameAsString().find("(anonymous)")
                       == std::string::npos) {
                type = std::string(recordDecl->getKindName().data()) + " "
                       + recordDecl->getQualifiedNameAsString();
            }
        }
        fld["type"] = type;
    }
    else {
        fld["type"] = type;
        fld["name"] = "(anonymous)";
    }
    fld["isStatic"] = flag;
    fld["isInner"]  = inner;
    configReduced(fld, fieldType);

    if (verbose) {
        std::cout << " '" << fld["type"].get<std::string>() << "'\n";
        std::cout << "      Sub-Types: ";
    }

    if (inner) {
        // Keep full name with namespace and outer-class
        // need them to draw inner-class
        if (const auto *elaboratedType = fieldType->getAs<ElaboratedType>()) {
            if (verbose) {
                std::cout << " (ElaboratedType) ";
            }
            QualType qualType = elaboratedType->desugar();
            grabFinalType(fld, qualType);
        }
        else {
            grabFinalType(fld, fieldType);
        }
    }
    else {
        grabType(fld, fieldType);
    }
    classes[name]["fields"].push_back(fld);
};

void FindNamedClassVisitor::grabFields(CXXRecordDecl *declaration,
                                       const std::string &name)
{
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

void FindNamedClassVisitor::grabTemplates(CXXRecordDecl *declaration,
                                          const std::string &name)
{
    if (const auto *recordDecl
        = llvm::dyn_cast<clang::CXXRecordDecl>(declaration)) {
        if (const auto *templateDecl
            = recordDecl->getDescribedClassTemplate()) {
            const auto *templateParmList
                = templateDecl->getTemplateParameters();
            for (auto it = templateParmList->begin();
                 it != templateParmList->end(); ++it) {
                if (const auto *parm = llvm::dyn_cast<clang::NamedDecl>(*it)) {
                    std::string templ = parm->getNameAsString();
                    if (templ != "") {
                        classes[name]["templates"].push_back(templ);
                        if (verbose) {
                            std::cout << "  Template : " << templ << "\n";
                        }
                    }
                }
            }
        }
    }
}

std::string FindNamedClassVisitor::getNamespace(const Decl *declaration)
{
    std::vector<std::string> namespaces;
    const DeclContext *ctx = declaration->getDeclContext();
    while (ctx != nullptr && !ctx->isTranslationUnit()) {
        if (const NamespaceDecl *ns = dyn_cast<NamespaceDecl>(ctx)) {
            std::string nsName
                = ns->isAnonymousNamespace() ? "" : ns->getNameAsString();
            if (!nsName.empty()) {
                namespaces.push_back(nsName);
            }
        }
        ctx = ctx->getParent();
    }

    std::string namespaceName;
    for (auto it = namespaces.rbegin(); it != namespaces.rend(); ++it) {
        if (it != namespaces.rbegin()) {
            namespaceName += "::";
        }
        namespaceName += *it;
    }

    if (verbose && !namespaceName.empty()) {
        std::cout << "(" << namespaceName << ")";
    }

    return namespaceName;
}

void FindNamedClassVisitor::configClassReduced(json &obj)
{
    std::string reduceReason;
    std::string name = obj["name"];
    std::string loc  = obj["location"];

    bool isReduced = isNamespaceReduced(name, reduceReason)
                     || isExcluded(loc, reduceReason);
    obj["isReduced"]              = isReduced;
    obj["utils"]["ReducedReason"] = reduceReason;
}

bool FindNamedClassVisitor::VisitCXXRecordDecl(CXXRecordDecl *declaration)
{
    std::string name;
    if (declaration->isAnonymousStructOrUnion()) {
        name = "(anonymous)";
    }
    else {
        name = declaration->getQualifiedNameAsString();
    }

    if (classes.contains(name) && (classes[name]["isComplete"])) {
        /* When a header file was included in multi-source-files,
         * the VisitCXXRecordDecl() would be triggered serval times.
         */
        return true;
    }

    std::string kind = declaration->getKindName().data();
    bool inner;
    DeclContext *context = declaration->getDeclContext();
    if (context && context->isRecord()) {
        inner = true;
    }
    else {
        inner = false;
    }

    if (verbose) {
        std::cout << "\nFound class: "
                  << declaration->getQualifiedNameAsString() << "\n";
        std::cout << "  ";
    }
    std::string namespaceName = getNamespace(declaration);
    std::string location
        = declaration->getLocation().printToString(Context->getSourceManager());
    if (verbose) {
        std::cout << location << "\n";
    }

    if (declaration->hasNameForLinkage()) {
        auto set = [&](bool complete) {
            classes[name]["kind"]       = kind;
            classes[name]["location"]   = location;
            classes[name]["namespace"]  = namespaceName;
            classes[name]["isComplete"] = complete;
            classes[name]["isInner"]    = inner;
            configClassReduced(classes[name]);
        };

        if (declaration->isCompleteDefinition()) {
            if (name.find("(anonymous)") != std::string::npos) {
                return true;
            }

            if (!classes.contains(name)) {
                newClass(name);
            }
            set(true);

            if (!classes[name]["isReduced"]) {
                grabBaseClasses(declaration, name);
                grabTemplates(declaration, name);
                grabFields(declaration, name);
            }
        }
        else {
            if (!classes.contains(name)) {
                newClass(name);
                set(false);
            }
        }
    }

    return true;
}

bool FindNamedClassVisitor::VisitTypedefDecl(TypedefDecl *typedefDecl)
{
    std::string name = "";
    std::string location
        = typedefDecl->getLocation().printToString(Context->getSourceManager());

    QualType underlyingType = typedefDecl->getUnderlyingType();
    if (const NamespaceDecl *namespaceDecl
        = dyn_cast<NamespaceDecl>(typedefDecl->getDeclContext())) {
        std::string contextName = namespaceDecl->getQualifiedNameAsString();
        std::string typeName    = underlyingType.getAsString();
        name                    = contextName + "::" + trimType(typeName);
    }
    else if (const CXXRecordDecl *recordDecl
             = dyn_cast<CXXRecordDecl>(typedefDecl->getDeclContext())) {
        std::string contextName = recordDecl->getQualifiedNameAsString();
        std::string typeName    = underlyingType.getAsString();
        name                    = contextName + "::" + trimType(typeName);
    }
    else {
        name = trimType(typedefDecl->getNameAsString());
    }

    if (verbose) {
        std::cout << "\nFound typedef: " << name << "\n";
        std::cout << "  ";
    }
    std::string namespaceName = getNamespace(typedefDecl);
    if (verbose) {
        std::cout << location << "\n";
    }

    if (const EnumType *enumType = underlyingType->getAs<EnumType>()) {
        name               = typedefDecl->getNameAsString();
        EnumDecl *enumDecl = enumType->getDecl();
        VisitEnumDecl(enumDecl, location, namespaceName, name);
    }
    else {
        const RecordType *recordType = underlyingType->getAs<RecordType>();
        if (recordType != NULL) {
            RecordDecl *recordDecl = recordType->getDecl();
            if (recordDecl != NULL) {
                std::string kind = recordDecl->getKindName().data();
                if (kind == "struct" || kind == "union" || kind == "enum") {
                    QualType desugaredType
                        = underlyingType.getDesugaredType(*Context);
                    std::string alias = trimType(desugaredType.getAsString());
                    newClass(name);
                    classes[name]["kind"]      = kind;
                    classes[name]["location"]  = location;
                    classes[name]["namespace"] = namespaceName;
                    classes[name]["isInner"]   = isInner(recordType);
                    configClassReduced(classes[name]);
                    if (alias != name) {
                        classes[name]["typedef"] = alias;
                    }
                    if (!classes[name]["isReduced"]) {
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
    std::string location
        = enumDecl->getLocation().printToString(Context->getSourceManager());
    std::string name = enumDecl->getQualifiedNameAsString();
    if (verbose) {
        std::cout << "\nFound enum: " << name << "\n";
        std::cout << "  ";
    }
    std::string namespaceName = getNamespace(enumDecl);
    if (verbose) {
        std::cout << location << "\n";
    }
    return VisitEnumDecl(enumDecl, location, namespaceName, name);
}

bool FindNamedClassVisitor::VisitEnumDecl(EnumDecl *enumDecl,
                                          const std::string &location,
                                          const std::string &ns,
                                          const std::string &name)
{
    if (name.length() > 0) {
        newClass(name);
        classes[name]["kind"]      = "enum";
        classes[name]["location"]  = location;
        classes[name]["namespace"] = ns;
        configClassReduced(classes[name]);
    }

    return true;
}
