class Foo {
public:
    int a_int;
};

class Bar {
protected:
    Foo foo;
};

class Child : public Bar {
private:
    Foo myFoo;
};

class Two : public Foo, public Bar {};
