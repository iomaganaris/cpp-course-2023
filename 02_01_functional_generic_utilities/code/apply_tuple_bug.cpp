#include <cstddef>
#include <functional>
#include <iostream>
#include <tuple>
#include <typeinfo>
#include <utility>

template <typename... Ts>
std::size_t f(Ts... ts) {
    constexpr auto g = [](auto... ts) {
        // Print the types in ts...
        std::cout << '\n';
        (std::cout << ... << typeid(ts).name()) << '\n';
        return sizeof...(ts); };

    // Are the following lines equivalent? Why? Why not?  What does class
    // template argument deduction do? See
    // https://eel.is/c++draft/over.match.class.deduct#1.3.
    // In std::invoke ts.. is a tuple the second when f(std::tuple(42, 3.14, std::string("hello"))) is called and then the tuple is passed to the invoke
    // and g is going to be called with a tuple
    // return std::invoke(g, ts...);
    // In std::apply ts.. is a tuple the second when f(std::tuple(42, 3.14, std::string("hello"))) is called and because of the CTAD, the copy constructor of
    // std::tuple is called thus passing in the end an std::tuple to std::apply, which is what is expects. So the g() is going to be called with the arguments
    // without being wrapped in a tuple.
    return std::apply(g, std::tuple(ts...));
}

int main() {
    std::cout << "int: " << f(42) << '\n';
    std::cout << "int, double: " << f(42, 3.14) << '\n';
    std::cout << "int, double, std::string: " << f(42, 3.14, std::string("hello")) << '\n';
    std::cout << "std::tuple<int, double, std::string>: " << f(std::tuple(42, 3.14, std::string("hello"))) << '\n';
}
