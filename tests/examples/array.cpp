class Foo {
public:
    int a_int;
};

class Array {
    Foo foo[10];
};

class Pointer {
    Foo *foo;
};

class PointerArray {
    Foo *foo[10];
};
