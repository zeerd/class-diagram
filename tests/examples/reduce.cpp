#include <mutex>
#include <vector>

typedef void (*func_t)(void);

class Basic {
    int a;
};

class Std {
    std::vector<int> a;
};

template <class T>
class Template {
    T t;
};

class NonStd {
    Template<int> a;
};

class Func {
    func_t a;
};

class StdMutex {
    std::mutex l;
};

class StdVector {
    std::vector<Basic*> l;
};
