#include <pthread.h>

enum AEnum { A, B, C };

struct Ast {
    int a;
};

union AUnion {
    AEnum a;
    Ast b;
};

class EnumClass {
    AEnum a_enum;
};

class StClass {
    struct Ast ast;
};

class ConstClass {
    const Ast ast;
};

class UnionClass {
    AUnion a;
};

class ThreadClass {
    pthread_mutex_t a;
};
