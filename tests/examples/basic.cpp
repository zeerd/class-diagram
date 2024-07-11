class Foo {
public:
    int a_int;
};

class Bar {
    Foo foo;
};

class Child : public Bar {
};
