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

#ifndef RUSH_IRGEN_LLVM_DECLARATIONS_HPP
#define RUSH_IRGEN_LLVM_DECLARATIONS_HPP

#include "rush/ast/traversal.hpp"
#include "rush/ast/declarations.hpp"
#include "rush/irgen/llvm/generator.hpp"

#include "llvm/IR/IRBuilder.h"

namespace rush::irgen::llvm {
   class llvm_ir_declaration_generator
   : public irgen::llvm_ir_generator<> {
   private:
      struct parameter_binder;
   public:
      virtual void visit_storage_decl(ast::storage_declaration const& decl) override;
      virtual void visit_function_decl(ast::function_declaration const& decl) override;
   };
}

#endif // RUSH_IRGEN_LLVM_DECLARATIONS_HPP
