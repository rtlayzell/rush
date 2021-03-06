/*************************************************************************
* Copyright 2018 Reegan Troy Layzell
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*       http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*************************************************************************/
#pragma once

#ifndef RUSH_AST_STMTS_EXPRESSION_HPP
#define RUSH_AST_STMTS_EXPRESSION_HPP

#include "rush/ast/stmts/statement.hpp"
#include "rush/ast/exprs/expression.hpp"

#include "rush/ast/visitor.hpp"

namespace rush::ast {

	// simple wrapper around expressions so that some
	// expressions may be treated as statements
	class expression_statement : public statement {
	public:
		expression_statement(std::unique_ptr<expression> expr)
         : _expr(std::move(expr)) { adopt(*_expr); }

      virtual ast::statement_kind kind() const noexcept override {
         return ast::statement_kind::expression;
      }

		using node::accept;
		virtual void accept(ast::visitor& v) const override {
			_expr->accept(v);
		}

   protected:
      virtual void adopted(ast::node& parent) override {
         orphan(*_expr);
         adopt(*_expr, &parent);
      }

      virtual void attached(ast::scope& scope, ast::context&) override {
         attach(scope, *_expr);
      }

      virtual void detached(ast::context&) override {
         detach(*_expr);
      }

	private:
		std::unique_ptr<expression> _expr;
	};

   namespace stmts {
      inline std::unique_ptr<expression_statement> expr_stmt(std::unique_ptr<expression> expr) {
         if (expr == nullptr) throw new std::invalid_argument("expr == nullptr");
         return std::make_unique<expression_statement>(std::move(expr));
      }
   }
} // rush::ast::details

#endif // RUSH_AST_STMTS_EXPRESSION_HPP
