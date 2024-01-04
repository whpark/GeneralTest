#include <catch2/catch_template_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <fmt/format.h>
#include <iostream>

namespace {
    struct {
        void* pThis;
        void* pX;
        void* pY;
        void* pZ;
    } g{};

    class Rng
    {
        int _min;
        int _max;
        // invariant: _min <= _max

    public:
        Rng(int lo, int hi) 
            // precondition: lo <= hi
            : _min(lo), _max(hi)
        {
            g.pThis = this;
            fmt::print("addr this: {}\n", (void*)this);
        }

        int min() const { return _min; }
        int max() const { return _max; }

        void set(int lo, int hi)
            // precondition: lo <= hi
        {
            _min = lo;
            _max = hi;
        }
    };

    const Rng foo()
    {
        const Rng x {1, 2};
        g.pX = const_cast<Rng*>(&x);
        fmt::print("addr X: {}\n", (void*)&x);
        return x;
    }

    Rng bar()
    {
        Rng y = foo();
        g.pY = const_cast<Rng*>(&y);
        fmt::print("addr Y: {}\n", (void*)&y);
        y.set(3, 4);
        return y;
    }

    TEST_CASE("TheDoubleLifeOfObjects", "[TheDoubleLifeOfObjects]") {
        return;
   	    const Rng z = bar();
        g.pZ = const_cast<Rng*>(&z);
	    fmt::print("addr Z: {}\n", (void*)&z);
        REQUIRE(g.pThis == g.pX);
        REQUIRE(g.pThis == g.pY);
        REQUIRE(g.pThis == g.pZ);
    }

    TEST_CASE("Manipulate Const objects", "[TheDoubleLifeOfObjects]") {
        return;
        const int i = 9;
        int& j = const_cast<int&>(i);
        j = 4;
        std::cout << "cout prints i : " << i << std::endl; // prints 9
        std::cout << "cout prints j : " << j << std::endl; // prints 4
        REQUIRE(i != j);
        if (i == j) {
            std::cout << "i == j" << std::endl;
	    } else {
		    std::cout << "i != j" << std::endl;
        }

        fmt::print("fmt::print prints i : {}\n", i); // prints 4
        fmt::print("fmt::print prints j : {}\n", j); // prints 4

        REQUIRE(i != j);
        if (i == j) {
            std::cout << "i == j" << std::endl;
        } else {
            std::cout << "i != j" << std::endl;
        }
    }

}
