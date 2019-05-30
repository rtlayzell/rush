#include "rush/ast/visitor.hpp"

#include "rush/ast/types.hpp"
#include "rush/ast/expressions.hpp"
#include "rush/ast/declarations.hpp"
#include "rush/ast/statements.hpp"

namespace rush::ast {
   void visitor::visit_throw_stmt(result_statement const& stmt) {
      visit_throw_stmt(static_cast<simple_statement const&>(stmt));
   }

   void visitor::visit_return_stmt(result_statement const& stmt) {
      visit_return_stmt(static_cast<simple_statement const&>(stmt));
   };

   void visitor::visit_simple_stmt(simple_statement const& stmt) {
#     define RUSH_SIMPLE_STMT_VISIT_SWITCH
#     include "rush/ast/stmts/_statements.hpp"
   };

   void visitor::visit_result_stmt(result_statement const& stmt) {
#     define RUSH_RESULT_STMT_VISIT_SWITCH
#     include "rush/ast/stmts/_statements.hpp"
   };

   void visitor::visit_unary_expr(unary_expression const& expr) {
#     define RUSH_UNARY_EXPRESSION_VISIT_SWITCH
#     include "rush/ast/exprs/_operators.hpp"
   }

   void visitor::visit_binary_expr(binary_expression const& expr) {
#		define RUSH_BINARY_EXPRESSION_VISIT_SWITCH
#		include "rush/ast/exprs/_operators.hpp"
   }
} // rush::ast