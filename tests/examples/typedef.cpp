#include <functional>
#include <memory>

typedef enum { A, B, C } AEnum;
using int_type = unsigned long;

typedef struct {
    int a;
} TypeDefAst;

typedef struct StAst {
    int a;
} NamedTypeDefAst;

typedef union {
    AEnum a;
    TypeDefAst b;
} AUnion;

class EnumClass {
    AEnum a_enum;
};

class StClass {
    NamedTypeDefAst ast;
};

class StStClass {
    struct StAst ast;
};

union StUnionClass {
    char arr[10];
    NamedTypeDefAst ast;
};

class ConstClass {
    const TypeDefAst ast;
};

class UnionClass {
    AUnion a;
};

class UsingClass {
    int_type a;
};

template <typename T>
class UsingTypenameClass {
    using my = typename T::int_t;
    my a;
};

namespace abc {

template <class T>
class Template {
    T t;
};

template <typename CharType>
using UN = Template<CharType>;

template <typename U>
class UsingTemplateTypenameClass {
    using UN = abc::UN<U>;
    UN t;
};
};  // namespace abc

class UseUsingTemplateTypenameClass {
    abc::UsingTemplateTypenameClass<EnumClass> a;
};
