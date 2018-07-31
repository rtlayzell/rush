#pragma once

#ifndef RUSH_AST_LITERAL_HPP
#define RUSH_AST_LITERAL_HPP

#include "rush/ast/expression.hpp"

#include <variant>
#include <string>

namespace rush::ast {
	class literal_expression : public expression {
		friend std::unique_ptr<literal_expression> literal_expr(std::string);
		friend std::unique_ptr<literal_expression> literal_expr(double, std::size_t);
		friend std::unique_ptr<literal_expression> literal_expr(std::uint64_t, std::size_t);

		using variant_type = std::variant<
			std::string,
			std::uint64_t,
			double>;
	public:

		virtual void accept(ast::visitor&) const override;
	private:
		variant_type _val;
		literal_expression(variant_type val) : _val(std::move(val)) {}
	};

	std::unique_ptr<literal_expression> literal_expr(std::string);
	std::unique_ptr<literal_expression> literal_expr(double, std::size_t);
	std::unique_ptr<literal_expression> literal_expr(std::uint64_t, std::size_t);
} // rush

#endif // RUSH_AST_LITERAL_HPP