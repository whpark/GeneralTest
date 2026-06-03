#pragma once

#include <memory>

namespace incomplete_type {

	class xIncompleteType;

	//
	class xIncompleteTypeDefaultDest {
	public:
		std::string name;
		xIncompleteTypeDefaultDest() = default;
		~xIncompleteTypeDefaultDest() = default;
	};
	class xIncompleteTypeDefaultDestImpl {
	public:
		std::string name;
		xIncompleteTypeDefaultDestImpl();
		~xIncompleteTypeDefaultDestImpl();
	};
	class xIncompleteTypeDestInline {
	public:
		std::string name;
		xIncompleteTypeDestInline() {}
		~xIncompleteTypeDestInline() {}
	};
	class xIncompleteTypeDest {
	public:
		std::string name;
		xIncompleteTypeDest();
		~xIncompleteTypeDest();
	};

	//
	class xTester {
	public:
		std::unique_ptr<xIncompleteType> m_ptr;
		std::unique_ptr<xIncompleteTypeDefaultDest> m_ptrDefaultDest;
		std::unique_ptr<xIncompleteTypeDefaultDestImpl> m_ptrDefaultDestImpl;
		std::unique_ptr<xIncompleteTypeDestInline> m_ptrDestInline;
		std::unique_ptr<xIncompleteTypeDest> m_ptrDest;

		~xTester() = default;
	};



}
