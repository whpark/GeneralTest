#include <fmt/core.h>
#include <fmt/xchar.h>
#include <fmt/chrono.h>

#include <catch.hpp>
#include <boost/endian.hpp>
#include "boost/pfr.hpp"
#include <lzma.h>

import std;

using namespace std::literals;

namespace testPFR {
	struct A {
		int x;
		float y;
	};
	struct B : A {
		//double z;
	};

	TEST_CASE("pfr", "[pfr]") {
		int k = -1;
		fmt::println("k:{}, k/3:{}", k, k/3);
		fmt::println("(k-3+1)/3:{}", (k-3+1)/3);
		A a{1, 2.0f};
		B b{{3, 4.0f}/*, 5.0*/};
		REQUIRE(boost::pfr::get<0>(a) == 1);
		REQUIRE(boost::pfr::get<1>(a) == 2.0f);
		//REQUIRE(boost::pfr::get<0>(b) == 3);
	}

}

namespace testVirtual {

	class A {
	public:
		A() {
			fmt::println("A::A()");
			REQUIRE(Open() == 'A');
		}
		virtual ~A() {
			fmt::println("A::~A()");
			REQUIRE(Close() == 'A');
		}
		virtual int Open() {
			fmt::println("A::Open()");
			return 'A';
		}
		virtual int Close() {
			fmt::println("A::Close()");
			return 'A';
		}
	};

	class B : public A {
	public:
		B() {
			fmt::println("B::B()");
			REQUIRE(Open() == 'B');
		}
		~B() override {
			fmt::println("B::~B()");
			REQUIRE(Close() == 'B');
		}

		int Open() override {
			fmt::println("B::Open()");
			return 'B';
		}
		int Close() override {
			fmt::println("B::Close()");
			return 'B';
		}
	};

	class C : public A {
	public:
		C() {
			fmt::println("C::C()");
			REQUIRE(Open() == 'A');
		}
		~C() override {
			fmt::println("C::~C()");
			REQUIRE(Close() == 'A');
		}

		//int Open() override {
		//	fmt::println("B::Open()");
		//	return 'B';
		//}
		//int Close() override {
		//	fmt::println("B::Close()");
		//	return 'B';
		//}
	};

	class D : public C {
	public:
		D() {
			fmt::println("D::D()");
			REQUIRE(Open() == 'D');
		}
		~D() override {
			fmt::println("D::~D()");
			REQUIRE(Close() == 'D');
		}
		int Open() override {
			fmt::println("D::Open()");
			return 'D';
		}
		int Close() override {
			fmt::println("D::Close()");
			return 'D';
		}
	};

	TEST_CASE("virtual", "[virtual]") {
		// ???????????
		fmt::println("====== virtual ======");
		std::unique_ptr<A> a = std::make_unique<A>();
		std::unique_ptr<A> b = std::make_unique<B>();
		std::unique_ptr<A> c = std::make_unique<C>();
		std::unique_ptr<A> d = std::make_unique<D>();
		fmt::println("====== closing ======");
	}

}

namespace test {

	struct sTest {
		int x{};
		std::string str;

		auto GetX() const { return x; }
		auto GetStr() const { return str; }

	};

	TEST_CASE("projected") {
		std::vector<sTest> lst{ {1, "one"}, {2, "two"}, {3, "three"} };
		auto iter1 = std::ranges::find(lst, 2, &sTest::x);
		REQUIRE(iter1->str == "two");
		auto iter2 = std::ranges::find(lst, "three", &sTest::GetStr);
		REQUIRE(iter2->x == 3);

	}

	template < typename T >
	void Func(T&& t) {
		fmt::print("{}", t);
		if constexpr (std::is_const_v<std::remove_reference_t<decltype(t)>>) {
			fmt::print(" is const\n");
		}
		else {
			fmt::print(" is not const\n");
			t += "sdfsdf";
		}

		if constexpr (std::is_reference_v<decltype(t)>) {
			fmt::print(" is reference\n");
		}
		else {
			fmt::print(" is not reference\n");
		}
	}
	TEST_CASE("universal reference") {
		std::string const str = "Hello, World!";
		std::string const& str2 = str;
		Func(str2);

		std::string str3 = "Hello, World!";
		std::string_view sv(str3);
		std::span sp(str3);
		sp[3] = 'a';
		fmt::print("{}\n", str3);

	}

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
		int i = 0;
		int j = ++i = 43;
		REQUIRE(j == 43);
		REQUIRE(i == 43);

		std::vector<int> data{8, 1, 9, 2, 7, 3, 6, 4, 5};
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
		static std::wstring const str2(L"Hello World!");
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

	TEST_CASE("charset") {
		//static_assert(__cpp_char8_t < 202207L);
	//#if (__cpp_char8_t >= 202207L)
	//	std::string str1 = u8"test";
	//	std::string str2 = {u8"test"};
	//	std::string str3 = u8"test"s;
	//#endif
		std::u32string str4 = U"sdfasdf";
		std::u16string str5 = u"sdfasdf";
	#if (__cpp_char8_t >= 201810L)
		char8_t asd = u8'a';
		std::u8string str6 = u8"sdfasdf";
	#endif

	}

	//=============================================================================================================================
	// https://www.sandordargo.com/blog/2024/03/06/std-filesystem-part2-iterate-over-directories
	void iterateOverDirectory(const std::filesystem::path& root) {
		if (!exists(root))
			return;
		std::array<std::string, 2> allowed_extensions{".h", ".cpp"};

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

	TEST_CASE("filesystem", "[parent_path]") {
		namespace fs = std::filesystem;

		fs::path path("/a/b/c/d/");
		REQUIRE(path.parent_path() == fs::path("/a/b/c/d"));
		REQUIRE(path.parent_path().parent_path() == fs::path("/a/b/c"));
		REQUIRE(path.parent_path().parent_path() != fs::path("/a/b/c/"));
	}

	TEST_CASE("liblzma", "[lzma]") {
		// get lzma version
		const auto version = lzma_version_string();
		fmt::print("lzma version: {}\n", version);
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

	//template < typename T >
	//class xCRTPBase {
	//public:
	//	xCRTPBase(this auto&& self) {

	//	}
	//};

	//TEST_CASE("deducing this") {

	//}


	struct A {
		int x, y;
		auto operator <=> (A const&) const = default;
	};

	TEST_CASE("spaceship") {

		A a{100, 150};
		A b{100, 150};

		REQUIRE(!(a < b));
		REQUIRE((a < b) != true);

	}
}
struct __declspec(align(1)) sReturn {
	char c;
	int i;
	int b;
};
static_assert(sizeof(sReturn) == 12);	// WHY?????????? -> only larger values valid
static_assert(offsetof(sReturn, i) == 4);	// WHY??????

#pragma pack(push, 1)
struct sReturn2 {
	char c;
	int i;
	int b;
};
#pragma pack(pop)
static_assert(sizeof(sReturn2) == 9);
static_assert(offsetof(sReturn2, i) == 1);

namespace {

	class A {
	public:
		int i;
	protected:
		int a;

	public:
		auto operator <=> (A const&) const = default;
		auto& valueA() { return a; }
	};

	TEST_CASE("misc1") {
		A a;
		a.i = 10;
		a.valueA() = 20;

		A a2 = a;

		REQUIRE(a == a2);
		a2.valueA() = 30;
		REQUIRE(a < a2);
	}

	struct { int a; int b; } PutSomething(int a, int b) { return {a, b}; };
	//auto PutSomething2(int a, int b) -> struct sReturn { int a, b;} {
	//	struct sReturn {
	//		int a; int b;
	//	};
	//	return sReturn{a, b};
	//};

}

namespace {

#if 0
	TEST_CASE("istream") {
		{
			std::string str;
			str.assign(512*1024, 0);
			for (size_t i{}; i < str.size(); i++) {
				str[i] = 'a' + (i % 28);
				if (i%28 == 26)
					str[i] = '\r';
				if (i%28 == 27)
					str[i] = '\n';
			}
			std::ofstream f("\\test.txt", std::ios_base::binary);
			f.write(str.data(), str.size());
		}
		std::ifstream f("\\test.txt", std::ios_base::binary);
		f.seekg(0, std::ios_base::end);
		auto len = f.tellg();
		REQUIRE(len > 0);
		f.seekg(0);
		std::string str;
		str.reserve(len);
		do {
			std::string buf;
			buf.assign(80, 0);

			if (auto read = f.readsome(buf.data(), buf.size()); read > 0) {
				buf.resize(read);
				str += buf;
			}
			else
				break;
		} while (!f.eof());

		while (!str.empty()) {
			f.putback(str.back());
			REQUIRE(f.get() == str.back());
			f.putback(str.back());
			str.pop_back();
		}
		REQUIRE(true);

	}
#endif

}

namespace {

	TEST_CASE("infinity") {
		SECTION("double") {
			double d = DBL_MAX/DBL_MIN;
			REQUIRE(std::isinf(d));
			REQUIRE(d > 0);
			REQUIRE(!(d < 0));
			d = -d;
			REQUIRE(std::isinf(d));
			REQUIRE(d < 0);
			REQUIRE(!(d > 0));
		}

		SECTION("float") {
			float f = std::numeric_limits<float>::infinity();
			REQUIRE(std::isinf(f));
			REQUIRE(f > 0);
			REQUIRE(!(f < 0));
			f = -f;
			REQUIRE(std::isinf(f));
			REQUIRE(f < 0);
			REQUIRE(!(f > 0));
		}
	}

}

namespace test_copy_class {

	class A {
	private:
		std::string str;
	public:
		int a{};

		A() = default;
		//A(A const& ) = default;
		//A(A&&) = default;
		//A& operator=(A const&) = default;
		//A& operator=(A&&) = default;
		//virtual ~A() = default; 
		A(std::string, int) {}
		auto operator <=> (A const&) const = default;

		virtual void SetPrivate(std::string str) {
			this->str = std::move(str);
		}
		std::string const& GetString() const {
			return str;
		}

	};

	TEST_CASE("copy_class") {
		A a1, a2;
		a1.SetPrivate("Hello, World!, long string .................................... ");	// prevent SSO
		a2 = a1;

		REQUIRE(a1.GetString() == a2.GetString());
		REQUIRE(a1 == a2);

		a2.SetPrivate("merong");
		REQUIRE(a1 != a2);

		a2 = std::move(a1);
		REQUIRE(a1.GetString() == "");
	}

}

namespace {

	TEST_CASE("SSO test") {
		std::string str1 = "1";
		std::string str2 = std::move(str1);
		REQUIRE(str1.empty());
		REQUIRE(str2 == "1");
		std::vector<int> v;
	}

	void FuncVariant(std::variant<int, double, std::string> param) {
		std::visit([](auto&& arg) {
			std::println("FuncVariant: {}", arg);
		}, param);
	}

	TEST_CASE("variant") {
		std::variant<int, std::string, double> v;
		REQUIRE(v.index() == 0);
		v.emplace<int>(10);
		v.emplace<std::string>("Hello, World!");
		std::get<std::string>(v).clear();
		REQUIRE(std::get<std::string>(v).empty());

		v.emplace<double>(3.14);
		v.emplace<double>(3.15);
		REQUIRE(std::get<double>(v) == 3.15);

		std::string str{"bbb"};
		FuncVariant(std::move(str));
		REQUIRE(str.empty());
		int a{3};
		FuncVariant(a);
		REQUIRE(a == 3);

		//std::variant<int, std::string> v;
		//REQUIRE(v.index() == 0);
		//v.emplace<std::string>("Hello, World!");
		//REQUIRE(v.index() == 1);
		//REQUIRE(std::get<std::string>(v) == "Hello, World!");
		//v.emplace<int>(10);
		//REQUIRE(v.index() == 0);
		//REQUIRE(std::get<int>(v) == 10);
	}

}

namespace memory {

	template < typename T >
	struct TCloner {
		std::unique_ptr<T> operator () (T const& self) const { return self.clone(); }
	};
	template < typename T >
	struct TStaticCloner {
		std::unique_ptr<T> operator () (T const& self) const { return std::make_unique<T>(self); }
	};

	template < /*concepts::cloneable */typename T, class CLONE = TCloner<T> >
	class TCloneablePtr : public std::unique_ptr<T> {
	public:
		using base_t = std::unique_ptr<T>;
		using this_t = TCloneablePtr;

		using base_t::base_t;
		//TCloneablePtr(this_t&& other) : base_t(std::move(other)) {}
		using base_t::operator =;
		using base_t::operator *;
		using base_t::operator ->;
		using base_t::operator bool;

		static inline CLONE cloner;

		TCloneablePtr(std::unique_ptr<T>&& other) : base_t(std::move(other)) {}
		TCloneablePtr(this_t&& other) : base_t(std::move(other)) {}
		// copy constructor
		TCloneablePtr(std::unique_ptr<T> const& other) : base_t(other ? cloner(*other) : nullptr) {}
		TCloneablePtr(this_t const& other) : base_t(other ? cloner(*other) : nullptr) {}

		TCloneablePtr& operator = (std::unique_ptr<T>&& other) { base_t::operator = (std::move(other)); return *this; }
		TCloneablePtr& operator = (this_t&& other) { base_t::operator = (std::move(other)); return *this; }
		TCloneablePtr& operator = (std::unique_ptr<T> const& other) { base_t::operator = (cloner(*other)); return *this; }
		TCloneablePtr& operator = (this_t const& other) { base_t::operator = (cloner(*other)); return *this; }

		template < typename U, class CLONE2 >
		TCloneablePtr& operator = (TCloneablePtr<U, CLONE2>&& other) {
			reset(other.release());
			return*this;
		}
		template < typename U, class CLONE2 >
		TCloneablePtr& operator = (TCloneablePtr<U, CLONE2> const& other) {
			static CLONE2 cloner2;
			this->reset(other ? cloner2(*other).release() : nullptr);
			return*this;
		}
	};

	template < typename T1, typename T2, typename ... targs, typename ... targs2 >
	bool operator == (TCloneablePtr<T1, targs...> const& a, TCloneablePtr<T2, targs2...> const& b) {
		bool bEmptyA = !a;
		bool bEmptyB = !b;
		if (bEmptyA and bEmptyB) return true;
		else if (bEmptyA or bEmptyB) return false;
		return *a == *b;
	}
	template < typename T1, typename T2, typename ... targs, typename ... targs2 >
	bool operator != (TCloneablePtr<T1, targs...> const& a, TCloneablePtr<T2, targs2...> const& b) {
		return !(a == b);
	}
	template < typename T1, typename T2, typename ... targs, typename ... targs2 >
	bool operator < (TCloneablePtr<T1, targs...> const& a, TCloneablePtr<T2, targs2...> const& b) {
		bool bEmptyA = !a;
		bool bEmptyB = !b;
		if (bEmptyA and bEmptyB) return false;
		else if (bEmptyA) return true;
		else if (bEmptyB) return false;
		return *a < *b;
	}
	template < typename T1, typename T2, typename ... targs, typename ... targs2 >
	bool operator > (TCloneablePtr<T1, targs...> const& a, TCloneablePtr<T2, targs2...> const& b) {
		return b < a;
	}
	template < typename T1, typename T2, typename ... targs, typename ... targs2 >
	bool operator <= (TCloneablePtr<T1, targs...> const& a, TCloneablePtr<T2, targs2...> const& b) {
		return a == b or a < b;
	}
	template < typename T1, typename T2, typename ... targs, typename ... targs2 >
	bool operator >= (TCloneablePtr<T1, targs...> const& a, TCloneablePtr<T2, targs2...> const& b) {
		return b <= a;
	}

	class aaa {
	public:
		int i, k;
	public:
		virtual std::unique_ptr<aaa> clone() const {
			return std::make_unique<aaa>(*this);
		}
		std::unique_ptr<aaa> cloneSelf() const {
			return std::make_unique<aaa>(*this);
		}

		auto operator <=> (aaa const&) const = default;
	};
	class bbb : public aaa {
	public:
		virtual std::unique_ptr<aaa> clone() const {
			return std::make_unique<bbb>(*this);
		}
		static std::unique_ptr<bbb> cloneSelf(bbb const& self) {
			return std::make_unique<bbb>(self);
		}

		auto operator <=> (bbb const&) const = default;
	};

	TEST_CASE("cloneable") {
		TCloneablePtr<aaa> a = std::make_unique<aaa>();
		a->i = 42;
		a->k = 20;
		TCloneablePtr<aaa> a2 = a;
		REQUIRE(a == a2);
		//TCloneablePtr<aaa, decltype(&aaa::cloneSelf)> a3 = a2;
		TCloneablePtr<aaa> a4 = std::move(a);
		REQUIRE(a4 != a);
		REQUIRE(a4 == a2);

		struct cloner {
			std::unique_ptr<bbb> operator () (bbb const& self) {
				return bbb::cloneSelf(self);
			}
		};
		TCloneablePtr<bbb, cloner> b;
		b = std::make_unique<bbb>();
		TCloneablePtr<bbb, TStaticCloner<bbb>> b2 = b;
		//TCloneablePtr<bbb, TCloner<bbb>> b4 = b2;
		TCloneablePtr<bbb, TStaticCloner<bbb>> b5 = b2;
		TCloneablePtr<aaa, decltype([](auto const& self) { return std::make_unique<aaa>(self); })> a5 = std::make_unique<bbb>();
		TCloneablePtr<aaa> a6 = a5;
		//b = a;
	}

}


namespace destructors {
	class elem {
	public:
		std::string name;
	public:
		elem(std::string name) : name(std::move(name)) { fmt::print("elem::elem({})\n", name); }
		~elem() { fmt::print("elem::~elem({})\n", name); };
	};

	class A {
	public:
		A() { fmt::print("A::A()\n"); }
		virtual ~A() { fmt::print("A::~A()\n"); }
	};
	class B : public A {
	public:
		B() { fmt::print("B::B()\n"); }
		~B() { fmt::print("B::~B()\n"); }
	};
	class Ci : public B {
	public:
		elem e{"Ci"s};
		Ci() { fmt::print("Ci::Ci()\n"); }
		~Ci() { fmt::print("Ci::~Ci()\n"); }
	};
	class Ce : public B {
	public:
		elem e{"Ce"s};
		Ce();
		~Ce();
	};

	TEST_CASE("destructors") {
		std::unique_ptr<A> c1 = std::make_unique<Ci>();
		std::unique_ptr<A> c2 = std::make_unique<Ce>();

	}

	Ce::Ce() { fmt::print("Ce::Ce()\n"); }
	Ce::~Ce() { fmt::print("Ce::~Ce()\n"); }

}

namespace multi_inherence {
	class A {
	public:
		int i;
		int same_name;
	};
	class B {
	public:
		int j;
		int same_name;
	};

	class C : public A, public B {
	public:
		int same_name;

		A& getA() { return *this; }
		B& getB() { return *this; }
	};

	TEST_CASE("multi_inherence") {
		C c;
		c.getA().i = 10;
		c.getB().j = 20;
		c.A::same_name = 1;
		c.B::same_name = 2;
		c.C::same_name = 3;

		SECTION("ref1") {
			void* ptA = &c.getA();
			void* ptB = &c.getB();
			REQUIRE(ptA != ptB);
			C* pCa = (C*)ptA;
			C* pCb = (C*)ptB;
			REQUIRE(pCa == &c);
			REQUIRE(pCb != &c);
			REQUIRE(pCa != pCb);
		}
		SECTION("ref2") {
			A* ptA = &c.getA();
			B* ptB = &c.getB();
			REQUIRE((void*)ptA != (void*)ptB);
			C* pCa = (C*)ptA;
			C* pCb = (C*)ptB;
			REQUIRE(pCa == &c);
			REQUIRE(pCb == &c);
			REQUIRE(pCa == pCb);
		}


		SECTION("static_cast") {
			A* pA = static_cast<A*>(&c);
			B* pB = static_cast<B*>(&c);
			REQUIRE((void*)pA != (void*)pB);
			C* pCa = static_cast<C*>(pA);
			C* pCb = static_cast<C*>(pB);
			pCa->A::same_name = 10;
			pCb->B::same_name = 20;
			REQUIRE(pCa == &c);
			REQUIRE(pCb == &c);
			REQUIRE(pCa == pCb);
		}

		SECTION("dynamic_down_cast") {
			A* pA = dynamic_cast<A*>(&c);
			B* pB = dynamic_cast<B*>(&c);
			REQUIRE((void*)pA != (void*)pB);
			C* pCa = static_cast<C*>(pA);
			C* pCb = static_cast<C*>(pB);
			REQUIRE(pCa == &c);
			REQUIRE(pCb == &c);
			REQUIRE(pCa == pCb);
		}

		SECTION("aa") {
			void* ptA = &c.getA();
			void* ptB = &c.getB();
			REQUIRE(ptA != ptB);

			C* pCa = static_cast<C*>(ptA);
			C* pCb = static_cast<C*>(ptB);
			REQUIRE(pCa == &c);
			REQUIRE(pCb != &c);
			REQUIRE(pCa != pCb);
		}

	}

};

//namespace test {
//
//	struct s {
//		s(int x);
//		//: x(x) {
//		//	vec.push_back(*this);
//		//}
//		int x;
//	};
//	std::vector<s> vec;
//	s::s(int x) : x(x) {
//		vec.push_back(*this);
//	}
//	inline s a = s(100);
//	inline s b = s(200);
//	inline s c = s(300);
//	inline s d = s(400);
//	inline s e = s(500);
//	inline s f = s(600);
//
//	TEST_CASE("static fiasco") {
//	}
//}
//

namespace test {

	template < typename T, size_t N >
	class ArrayView;

	template < typename T, size_t N >
	class Array {
	public:
		using array_t = Array;
		using data_t = T;
		constexpr static inline size_t size() { return N; }
	public:
		std::array<T, N> m_array{};
		ArrayView<T, N> View() const;
	};

	template < typename T, size_t N >
	class ArrayView {
	public:
		std::span<T const, N> m_span;

		ArrayView() = default;
		ArrayView(ArrayView const&) = default;
		ArrayView(ArrayView&&) = default;
		ArrayView& operator=(ArrayView const&) = default;
		ArrayView& operator=(ArrayView&&) = default;
		ArrayView(Array<T, N> const& arr) : m_span(arr.m_array) {}
	};

	template < typename T, size_t N >
	ArrayView<T, N> Array<T, N>::View() const {
		return ArrayView<T, N>(*this);
	}

	template < typename T, size_t N >
	Array<T, N> Multiply(ArrayView<T, N> a_, ArrayView<T, N> b_) {
		Array<T, N> arr{};
		ArrayView<T, N> a = a_, b = b_;
		for (size_t i = 0; i < N; ++i) {
			arr.m_array[i] = a.m_span[i] * b.m_span[i];
		}
		return arr;
	}

	//template < typename TArray >
	//MultiPly(TArray a, TArray b) -> MultiPly<typename TArray::data_t, TArray::size()>;

	TEST_CASE("mul", "view") {

		Array<int, 3> a1;
		Array<int, 3> a2;

		auto a3 = Multiply(a1.View(), a2.View());
	}

}

namespace test20250520 {

	class sClass1 {
	public:
		int a;
		int b;

		int sClass1::* which {&sClass1::a};
	};

	template < typename T, typename T2, T2 T::* offset >
	void Func(T a) {

	}

	TEST_CASE("offset") {
		sClass1 s1;

		auto offset = &sClass1::a;
		int sClass1::* k = &sClass1::b;
		auto name = typeid(decltype(offset)).name();
		std::println("sClass1::a --> {}", typeid(decltype(offset)).name());

		s1.*offset = 3;
		s1.which = k;
		s1.*s1.which = 10;
		REQUIRE(s1.a == 3);
		REQUIRE(s1.b == 10);

		Func<sClass1, int, &sClass1::a>(s1);
	}


	struct base {
		bool a = false;
	};

	struct config : base {
		bool b = false;
	};

	template <auto Config, auto member_ptr>
	constexpr auto make_true() {
		auto ret = Config;
		ret.*member_ptr = true;
		return ret;
	}

	//static constexpr auto x = make_true<config{}, &config::a>();
}

namespace test_MSVC_static {

	class IBase {
	public:
		virtual ~IBase() = default;
		virtual void foo() = 0;
		//inline static std::map<std::string, std::function<std::unique_ptr<IBase>()>> map;
		static auto& GetMap() {
			static std::map<std::string, std::function<std::unique_ptr<IBase>()>> map;
			return map;
		}
		struct sRegister {
			sRegister(std::string const& name, std::function<std::unique_ptr<IBase>()> creator) {
				GetMap()[name] = std::move(creator);
			}
		};
	};

	template < class T >
	class TDerived : public IBase {
	public:
		T a{};

		void foo() override {};

		inline static sRegister reg{"derived", [] { return std::make_unique<TDerived>(); } };
	};

	class xDerived : public TDerived<int> {
	public:
	};

	TEST_CASE("static init order") {

		struct {
			int a{5};
			int b{42};
		} st;
		auto* ptr = &st;

		if (auto* entity = (xDerived*)(ptr))
			fmt::println("a:{}", entity->a);

	}

}

namespace test_20250522 {

	class xBase {
	public:
		int a{};
		double b{};
		std::string c;

		auto operator <=> (xBase const&) const = default;
	};

	template < typename TSelf >
	class TTest : public xBase {
	public:
		using self_t = TSelf;

		auto operator <=> (TTest const&) const = default;

		bool Compare(this auto&& self) {
			return true;
		}
	};

	class xDerived : public TTest<xDerived> {
	public:

		auto operator <=> (xDerived const&) const = default;
	};

	TEST_CASE("auto ") {

		xDerived a, b;
		REQUIRE(a.Compare());

	}


}

namespace test_20250523 {

	TEST_CASE("ranges") {
		{
			auto to_upper = [] (unsigned char c) { return std::toupper(c); };
			std::string s = "foobar";

			/* this has always been possible; v_indiri depends on s */
			auto v_indiri = s | std::views::transform(to_upper);

			fmt::println("s : {}", s);

			/* this is "new"; v_owning is standalone */
			auto v_owning = std::move(s) | std::views::transform(to_upper);

			fmt::println("typename v_owning: {}", typeid(v_owning).name());
			fmt::println("contents of v_owning: {}", std::string(v_owning.begin(), v_owning.end()));
			fmt::println("s : {}", s);


		}

		{
				/* Finding the smallest non-negative number in a vector */
			std::vector vec{-1, 2, -3, 1, 7};
			auto non_neg = [](int i) { return i >= 0; };

			//auto it1 = std::ranges::min_element(vec | std::views::filter(non_neg));
			//REQUIRE(*it1 == 1);   // broken, because filter_view is never borrowed

			//auto non_negative_vec = vec | std::ranges::views::filter(non_neg);
			//auto it2 = std::ranges::min_element(non_negative_vec);
			//REQUIRE(*it2 == 1); // now it should work correctly

		}
	}


}