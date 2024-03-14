#include <vector>
#include <algorithm>
#include <fstream>
#include <string>
#include <filesystem>
#include <ranges>
#include <fmt/core.h>
#include <fmt/xchar.h>
#include <fmt/chrono.h>
#include <thread>

#include <catch.hpp>

#include <boost/endian.hpp>

using namespace std::literals;

namespace test {

	struct normal {
		uint8_t foo;
		uint32_t bar;
	};
	static_assert(sizeof(normal) == 8);

	struct packed {
		uint8_t foo;
		boost::endian::native_uint32_t bar;
	};
	static_assert(sizeof(packed) == 5);

	TEST_CASE("misc", "[nth_element]") {
		std::vector<int> data{ 8, 1, 9, 2, 7, 3, 6, 4, 5 };
		std::nth_element(data.begin(), data.begin() + 4, data.end());
		REQUIRE(data[4] == 5);
		//std::cout << "data[4] == " << data[4] << "\n";

		std::nth_element(data.begin(), data.begin() + 7, data.end(),
						 std::greater<>());
		REQUIRE(data[7] == 2);
		//std::cout << "data[7] == " << data[7] << "\n";
	}

	class xA {
	public:
		std::string str;

		xA(std::string str) : str(std::move(str)) {}
		//xA(xA const& other) : str(other.str) {}
		//xA(xA&& other) : str(std::move(other.str)) {}
		//xA& operator=(xA const& other) {
		//	str = other.str;
		//	return *this;
		//}
		//xA& operator=(xA&& other) {
		//	str = std::move(other.str);
		//	return *this;
		//}
	};

	template < size_t N >
	class xStringLiteral {
	public:
		char content[N];
		static const auto length = N-1;
		constexpr xStringLiteral(char const (&str)[N]) {
			for (auto [i, c] : std::ranges::enumerate_view(str))
				content[i] = c;
		}
	};

	template < xStringLiteral lit >
	constexpr auto operator "" _lit() { return lit; }

	template < typename tchar_to, xStringLiteral literal >
	struct TStringLiteral {
		tchar_to value[std::size(literal.content)];
		static const auto length = literal.length;
		constexpr TStringLiteral() {
			for (size_t i{}; i < std::size(literal.content); i++) {
				//static_assert(literal.str[i] > 0 and literal.str[i] < 127);
				if (literal.content[i] < 0 or literal.content[i] > 127)
					throw std::logic_error("invalid character");
				value[i] = static_cast<tchar_to>(literal.content[i]);
			}
		}
	};

	template < xStringLiteral source >
	constexpr auto operator "" _ToW() {
		static constexpr TStringLiteral<wchar_t, source> lit;
		return lit.value;
	}

	template < xStringLiteral source, typename tchar_to >
	constexpr auto ToString() {
		static constexpr TStringLiteral<tchar_to, source> lit;
		return lit.value;
	}

	TEST_CASE("misc", "[rule_of_five]") {
		xA a("hello");
		xA b = a;
		REQUIRE(b.str == "hello");
		xA c = std::move(a);
		REQUIRE(c.str == "hello");
		REQUIRE(a.str == "");

		constexpr xStringLiteral strA("hello, world!");
		constexpr auto strB = "hello, world!"_lit;

		//constexpr auto strW = TStringLiteral<wchar_t, strA>{};
		constexpr auto strW = TStringLiteral<wchar_t, "Hello World!"_lit>{};
		static std::wstring const str2 (L"Hello World!");
		REQUIRE(str2 == strW.value);
		REQUIRE(str2 == L"Hello World!");
		fmt::print(L"str: {}\n", strW.value);

		REQUIRE("ABC"_ToW == L"ABC"s);
		REQUIRE(ToString<"ABC"_lit, wchar_t>() == L"ABC"s);
	}

	TEST_CASE("misc", "[permutation]") {
		const std::vector<std::string> vec{"axe", "bow", "cat", "dog", "elk", "fox"};

		for (auto [i, s] : std::views::enumerate(vec)) {
			fmt::print("{}: {}\n", i, s);
		}

		auto ev = std::views::enumerate(vec);

		auto strictly_ascending = [](const auto& triple_idx_elem) {
			const auto& [ie0, ie1, ie2] = triple_idx_elem;
			const auto& idx0            = get<0>(ie0);
			const auto& idx1            = get<0>(ie1);
			const auto& idx2            = get<0>(ie2);
			return idx0 < idx1 && idx1 < idx2;
		};

		for (const auto& [ie0, ie1, ie2] : std::views::cartesian_product(ev, ev, ev) | std::views::filter(strictly_ascending)) {
			fmt::println("{} {} {}", get<1>(ie0), get<1>(ie1), get<1>(ie2));
		}

	}


	//=============================================================================================================================
	// https://www.sandordargo.com/blog/2024/03/06/std-filesystem-part2-iterate-over-directories
	void iterateOverDirectory(const std::filesystem::path& root) {
		if (!exists(root))
			return;
		std::array<std::string, 2> allowed_extensions {".h", ".cpp"};

		auto const end = std::filesystem::recursive_directory_iterator();
		for (auto entry = std::filesystem::recursive_directory_iterator(root); entry != end; ++entry) {
			const auto filename = entry->path().filename().string();
			const auto extension = entry->path().extension().string();
			const auto level = entry.depth();
			if (!entry->is_directory() && std::ranges::find(allowed_extensions, extension) == allowed_extensions.end()) {
				continue;
			}
			if (entry->is_directory() && filename == "build") {
				entry.disable_recursion_pending();
				continue;
			}
			if (entry->is_directory()) {
				fmt::print("{:{}} directory: {}\n", ' ', level * 3, filename);
			}
			else if (entry->is_regular_file()) {
				fmt::print("{:{}} file: {}\n", ' ', level * 3, filename);
			}
			else {
				fmt::print("{:{}} unknown type: {}\n", ' ', level * 3, filename);
			}
		}
	}

	TEST_CASE("filesystem", "[disable_recursion_pending]") {
		std::filesystem::create_directory("temp");
		std::filesystem::create_directory("temp/build");
		std::filesystem::create_directory("temp/include");
		std::filesystem::create_directory("temp/src");

		// create files
		std::ofstream("temp/CMakeLists.txt").put('a');
		std::ofstream("temp/build/moduleA.h").put('a');
		std::ofstream("temp/build/moduleA.cpp").put('b');
		std::ofstream("temp/include/moreinfo.txt").put('a');
		std::ofstream("temp/include/moduleA.h").put('a');
		std::ofstream("temp/include/moduleB.h").put('b');
		std::ofstream("temp/src/moduleA.cpp").put('a');
		std::ofstream("temp/src/moduleB.cpp").put('b');

		iterateOverDirectory("temp");

		std::error_code ec;
		std::filesystem::remove_all("temp", ec);
		fmt::print("{}\n", ec ? "NOT Removed" : "Removed");
		iterateOverDirectory("temp");
	}


	//void task(int count) { 
	//	fmt::print("{}, {}\n", std::chrono::system_clock::now(), count);
	//}

	//void triggerTask(std::stop_token st) { 
	//	auto t0 = std::chrono::system_clock::now();

	//	int count = 0;
	//	while (!st.stop_requested()) { 
	//		task(count++); // Call the task function 
	//		//std::this_thread::sleep_for( std::chrono::microseconds(2500) ); 
	//		std::this_thread::sleep_for(std::chrono::microseconds(10));

	//		if (std::chrono::system_clock::now() - t0 > 5s)
	//			break;
	//	} 
	//}

	//TEST_CASE("misc", "[sleep]") {
	//	std::ios_base::sync_with_stdio( false );
	//	std::jthread t(triggerTask);
	//	bool test = false;

	//	auto t0 = std::chrono::system_clock::now();

	//	while (true) { 
	//		if (std::chrono::system_clock::now() - t0 > 5s)
	//			break;
	//	} 

	//}

}
