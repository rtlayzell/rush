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

#ifndef RUSH_IRGEN_LLVM_EXPRESSIONS_HPP
#define RUSH_IRGEN_LLVM_EXPRESSIONS_HPP

#include "rush/ast/traversal.hpp"
#include "rush/ast/expressions.hpp"
#include "rush/irgen/llvm/generator.hpp"


namespace rush::irgen::llvm {
   class llvm_ir_expression_generator : public llvm_ir_generator<::llvm::Value> {
   private:
      ::llvm::Value* _result;

   public:
      llvm_ir_expression_generator()
         : _result { nullptr } {}

      virtual ::llvm::Value* result() const override {
         return _result;
      }

      virtual void visit_unary_expr(ast::unary_expression const&) override;
      virtual void visit_binary_expr(ast::binary_expression const&) override;

      virtual void visit_literal_expr(ast::string_literal_expression const&) override;
      virtual void visit_literal_expr(ast::integer_literal_expression const&) override;
      virtual void visit_literal_expr(ast::boolean_literal_expression const&) override;
      virtual void visit_literal_expr(ast::floating_literal_expression const&) override;

      virtual void visit_identifier_expr(ast::identifier_expression const&) override;
      virtual void visit_invoke_expr(ast::invoke_expression const& expr) override;
   };
} // rush::irgen

#endif // RUSH_IRGEN_LLVM_EXPRESSIONS_HPP
