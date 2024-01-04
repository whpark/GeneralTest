#include <catch2/catch_template_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <fmt/format.h>
#include <iostream>

// https://www.sandordargo.com/blog/2024/01/03/DR20-equality-operator

namespace {

    class MyClass {
    public:
        int value;

        bool operator==(const MyClass& other) const {
            std::cout << "Using operator==\n";
            return value == other.value;
        }
    };

    // This function is not visible before operator== is called
    bool operator!=(const MyClass& a, const MyClass& b) {
        std::cout << "Using operator!=\n";
        return !(a == b);
    }


    TEST_CASE("EqualityOperator") {
        return;
        MyClass a{42};
        MyClass b{42};

        if (a == b) {
            std::cout << "a is equal to b\n";
        }

        if (a != b) {
            std::cout << "a is not equal to b\n";
        }
    }

}
