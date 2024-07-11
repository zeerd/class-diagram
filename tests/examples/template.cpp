#include <stdint.h>

#include <map>

class Foo {
public:
    int a_int;
};

template <class T>
class Template {
    T t;
};

class UseTemplate {
    Template<uint8_t> templ;
};

class UseStd {
    std::map<uint8_t, std::pair<Foo, bool>> a_map;
};
