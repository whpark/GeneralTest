#include <vector>
#include <algorithm>
#include <iostream>

#include <catch.hpp>

#include <glaze/glaze.hpp>
#include <glaze/core/macros.hpp>

namespace {

    struct sTest {
        int i{};
        int j{};
        std::string str;

        GLZ_LOCAL_META(sTest, i, j, str);
        //struct glaze {
        //    using T = sTest;
        //    static constexpr auto value = glz::object("i", &T::i, "j", &T::j, "str", &T::str);
        //};

    };

    TEST_CASE("json", "[glaze]") {
        std::string strJson1 = R"({"i":1,"str":"str","other":"@_@"})";
        std::string strJson2 = R"({"i":1,"str":"str"})";

        sTest t;
        constexpr glz::opts opts{.comments = true, .error_on_unknown_keys = false, .error_on_missing_keys = false };
        auto err1 = glz::read<opts>(t, strJson1);
        REQUIRE(!err1);

		auto err2 = glz::read<opts>(t, strJson2);
        REQUIRE(!err2);
    }

}