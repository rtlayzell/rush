#pragma once

#ifndef RUSH_AST_UNARY_HPP
#define RUSH_AST_UNARY_HPP

#include "rush/ast/expression.hpp"

#include <memory>

namespace rush::ast {
	class unary_expression : public expression {
	public:
		expression const& operand() const noexcept { return *_operand; }

		virtual void accept(ast::visitor&) const override;
	private:
		expression* _operand;
	};

	std::unique_ptr<unary_expression> increment_expr(std::unique_ptr<expression>);
	std::unique_ptr<unary_expression> decrement_expr(std::unique_ptr<expression>);
} // rush

#endif // RUSH_AST_UNARY_HPP
