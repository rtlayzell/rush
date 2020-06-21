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
#include "rush/ast/exprs/array.hpp"
#include "rush/ast/exprs/lambda.hpp"
#include "rush/ast/ptrns/type_annotation.hpp"
#include "rush/ast/decls/function.hpp"
#include "rush/ast/stmts/result.hpp"
#include "rush/ast/exprs/binary.hpp"
#include "rush/ast/context.hpp"

using namespace rush;

class array_literal_type_resolver : public ast::visitor {
public:
   array_literal_type_resolver(ast::node const* arr, ast::context& context)
      : _result { ast::types::undefined }
      , _array { arr }
      , _context { context } {}


   ast::type_ref result() const {
      return !_result.is<ast::array_type>()
           ? _context.array_type(
               _result == ast::types::undefined
               ? ast::types::inference_fail
               : _result)
           : _result;
   }

   virtual void visit_binding_ptrn(ast::binding_pattern const& ptrn) override {
      ptrn.pattern().accept(*this);
   }

   virtual void visit_type_annotation_ptrn(ast::type_annotation_pattern const& ptrn) override {
      _result = ptrn.type();
   }

   virtual void visit_result_stmt(ast::result_statement const& stmt) override {
      auto it = ast::find_ancestor<ast::lambda_expression>(&stmt);
      if (it != ast::ancestor_iterator())
      { _result = it->return_type(); }
      else {
         auto it = ast::find_ancestor<ast::function_declaration>(&stmt);
         if (it != ast::ancestor_iterator()) _result = it->return_type();
      }
   }

   virtual void visit_binary_expr(ast::binary_expression const& expr) override {
      if (_array != &expr.left_operand()) {
         switch (expr.opkind()) {
         default: return;
         case ast::binary_operator::assignment:
            _result = expr.left_operand().result_type();
         }
      }
   }

private:
   ast::type_ref _result;
   ast::node const* _array;
   ast::context& _context;
};

namespace rush::ast {
   ast::type_ref array_literal_expression::resolve_type() const {
      return parent()
           ? rush::visit(*parent(), array_literal_type_resolver { this, *context() }).result()
           : ast::types::undeclared;
   }
}
