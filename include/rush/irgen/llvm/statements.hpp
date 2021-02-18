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

#ifndef RUSH_IRGEN_LLVM_STATEMENTS_HPP
#define RUSH_IRGEN_LLVM_STATEMENTS_HPP

#include "rush/irgen/llvm/generator.hpp"
#include "llvm/IR/Value.h"

namespace rush::irgen::llvm {
   class llvm_ir_statement_generator
   : public llvm_ir_generator<> {
   private:
      ::llvm::Value* _result;
   public:
      virtual ::llvm::Value* result() const noexcept override {
         return _result;
      }

      virtual void visit_storage_decl(ast::storage_declaration const&) override;

      virtual void visit_if_stmt(ast::conditional_statement const&) override;
      virtual void visit_return_stmt(ast::result_statement const&) override;
   };
} // rush::irgen

#endif // RUSH_IRGEN_LLVM_STATEMENTS_HPP
