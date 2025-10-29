#pragma once

#include <memory>

namespace incomplete_type {

	//
	class xIncompleteType;

	//
	class xTesterDefaultDest {
	public:
		std::unique_ptr<xIncompleteType> m_ptr;

		~xTesterDefaultDest() = default;
	};

	class xTesterDestInline {
	public:
		std::unique_ptr<xIncompleteType> m_ptr;

		~xTesterDestInline() {}
	};

	class xTesterDest {
	public:
		std::unique_ptr<xIncompleteType> m_ptr;

		~xTesterDest();
	};

}
