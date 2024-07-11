class Foo {
public:
    Foo() {}
    int a_int;
};

class Array {
    Foo foo[10];
};

class BasicPointer {
    int *foo;
};

class ClzPointer {
    Foo *foo;
};

class PointerArray {
    Foo *foo[10];
};

class Reference {
    Foo &foo;

public:
    Reference(Foo &foo) : foo(foo) {}
};
