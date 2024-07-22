#include <string>

namespace ExNS {
class ExNSClz {};
};  // namespace ExNS

class ExNsClass {
    ExNS::ExNSClz m;
};

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

namespace a {
namespace b {
class c {
    class d {};
};
};  // namespace b
};  // namespace a

namespace def {
typedef struct {
    int i;
} NsTypeDefSt;

};  // namespace def

class Outer {
public:
    class In {
        int b;
    };
    In i;
};

class StOuter {
public:
    struct StIn {
        int b;
    } si;
};

class Outer2 {
private:
    class In2;
    static In2 i;
};
class Outer2::In2 {
public:
    explicit In2(int i) : b(i) {}

private:
    int b;
};
Outer2::In2 Outer2::i(0);

class StInClass {
    typedef struct {
        int a;
    } InClassSt;

    InClassSt ast;
};

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

class NsInnerTypedef {
    def::NsTypeDefSt t;
};

class MyString : public std::string {};
