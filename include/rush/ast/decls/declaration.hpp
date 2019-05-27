#pragma once

#ifndef RUSH_AST_DECLS_DECLARATION_HPP
#define RUSH_AST_DECLS_DECLARATION_HPP

#include "rush/ast/node.hpp"

#include <string>
#include <cstdint>

namespace rush::ast {
   class node;
   class type_ref;
   class identifier;

	enum class declaration_kind : std::uint8_t {
      parameter,
		constant,
		variable,
		function,
		alias,
		enum_,
		class_,
		struct_,
		concept,
		interface,
		extension,
		operator_,
	};

	class declaration : public node {
		declaration(declaration const&) = delete;
		void operator = (declaration const&) = delete;

	public:
      virtual std::string name() const = 0;

      virtual ast::type_ref type() const = 0;

		virtual declaration_kind kind() const = 0;

      virtual ast::identifier identifier() const = 0;

   protected:
      declaration() = default;
	};
}

#endif // RUSH_AST_DECLS_DECLARATION_HPP
