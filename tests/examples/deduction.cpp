
#include <stdint.h>

#include <array>  // std::array
#include <map>

template <typename A, typename B, typename C>
class AutoClass {
    std::map<A, std::pair<B, C>> t;
    static auto key   = std::map<A, std::pair<B, C>>::key_type();
    static auto value = std::map<A, std::pair<B, C>>::value_type();
};

class DecltypeClass {
    const decltype(1) a = 1;
};

template <typename T>
class DependentTypeClass {
    T a;
    decltype(a) b;
};
