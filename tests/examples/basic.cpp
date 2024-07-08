#include <stdint.h>

#include <map>

class Foo {
public:
    int a_int;
};

class Bar {
    Foo foo;
};

class Child : public Bar {
};

template <class T>
class Templ {
    T t;
};

class UseTempl {
    Templ<uint8_t> templ;
};

class UseStd {
    std::map<uint8_t, std::pair<Foo, bool>> a_map;
};
