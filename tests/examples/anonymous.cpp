
struct AnonymousUnion {
    union {
        char a;
        int b;
    };
};

struct AnonymousStruct {
    struct {
        char a;
        int b;
    };
};
