#include <stdint.h>

#include <map>
#include <string>
#include <vector>

class Foo {
public:
    int a_int;
};

class UseStd {
    std::map<uint8_t, std::pair<Foo, bool>> a_map;
};

template <class T>
class Template {
    T t;
};

class UseTemplate {
    Template<uint8_t> templ;
};

template <typename Type, typename SubType = int>
class TypenameTemplate {
    std::map<Type, SubType> t;
};

class UseTypenameTemplate {
    TypenameTemplate<uint8_t, bool> templ;
};

namespace Value {
template <template <typename U, typename... Args> class ArrayType = std::vector,
          class StringType                                        = std::string>
class ValueBase {
    using array_t  = ArrayType<ValueBase>;
    using string_t = StringType;
    union value {
        array_t *a;
        string_t *s;
    };
    value v;
};
using ValueClass = ValueBase<>;

template <typename T>
class MyVector : public std::vector<T> {
    MyVector() : std::vector<T>() {}
};

class UseValueClass {
    ValueBase<MyVector, Foo> sc;
};

};  // namespace Value
