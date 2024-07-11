#include <functional>

typedef void (*func_t)(void);
typedef std::function<void()> stdFunc_t;

class FuncClass {
    func_t fun;
};

class StdFuncClass {
    stdFunc_t fun;
};

class Clz;
class ClzFuncClass {
public:
    void (Clz::*func)(void);
};
