namespace abc {
class Ns {
public:
    int a;
    class Inner {
        int b;
    };
    struct InSt {
        int c;
    };
    Inner i;
    InSt s;
};
enum NsEnum { A, B, C };
};  // namespace abc

class Outer {
public:
    class In {
        int b;
    };
    In i;
};

class Outer2 {
public:
    class In2;
    static In2 i;
};
class Outer2::In2 {
public:
    In2() : b(0) {}

private:
    int b;
};
Outer2::In2 Outer2::i;

class NsClass {
    abc::Ns m;
};

class NsEnumClass {
    abc::NsEnum e;
};

class NsInnerClass {
    abc::Ns::Inner i;
};

class NsInnerStruct {
    abc::Ns::InSt s;
};
