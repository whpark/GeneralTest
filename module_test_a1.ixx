module;

export module module_test:a1;
import std;
//import :a2;

namespace module_test {
	class xClass2;
}

export namespace module_test {

	class xClass1 {
	public:
		std::unique_ptr<xClass2> m_class2;

	};

}