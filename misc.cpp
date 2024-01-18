#include <vector>
#include <algorithm>
#include <iostream>

#include <catch.hpp>

namespace {

    TEST_CASE("misc", "[nth_element]") {
        std::vector<int> data{8, 1, 9, 2, 7, 3, 6, 4, 5};
        std::nth_element(data.begin(), data.begin() + 4, data.end());
        REQUIRE(data[4] == 5);
        //std::cout << "data[4] == " << data[4] << "\n";

        std::nth_element(data.begin(), data.begin() + 7, data.end(),
                         std::greater<>());
        REQUIRE(data[7] == 2);
        //std::cout << "data[7] == " << data[7] << "\n";
    }

}