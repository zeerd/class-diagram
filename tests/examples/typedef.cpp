typedef enum { A, B, C } AEnum;

typedef struct {
    int a;
} Ast;

typedef union {
    AEnum a;
    Ast b;
} AUnion;

class EnumClass {
    AEnum a_enum;
};

class StClass {
    Ast ast;
};

class ConstClass {
    const Ast ast;
};

class UnionClass {
    AUnion a;
};
