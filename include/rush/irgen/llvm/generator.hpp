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

#ifndef RUSH_IRGEN_GENERATOR_HPP
#define RUSH_IRGEN_GENERATOR_HPP

#include "rush/ast/decls/nominal.hpp"
#include "rush/ast/traversal.hpp"

#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"

#include <unordered_map>

namespace rush::irgen {
   template <typename IRNodeT = ::llvm::Value>
   class llvm_ir_generator : public ast::traversal {
   public:
      template <typename IRNodeU>
      friend class llvm_ir_generator;

      using value_map_type = std::unordered_map<ast::nominal_declaration const*, llvm::Value*>;
      virtual IRNodeT* result() const {
         return nullptr;
      }

   protected:
      llvm::Value* lookup(ast::nominal_declaration const& decl) {
         assert(_values);
         auto it = _values->find(&decl);
         return it != _values->end()
              ? it->second
              : nullptr;
      }

      llvm::Value* insert(ast::nominal_declaration const& decl, llvm::Value* value) {
         assert(_values);
         (*_values)[&decl] = value;
         return value;
      }

      llvm::Module& module_() {
         assert(_module);
         return *_module;
      }

      llvm::LLVMContext& context() {
         assert(_context);
         return *_context;
      }

      llvm::IRBuilder<>& builder() {
         assert(_builder);
         return *_builder;
      }

      template <typename AstNodeT, typename GeneratorT>
      GeneratorT& generate(AstNodeT const& node, GeneratorT& gen) {
         assert(_values && "unintialized value map.");
         gen.initialize(module_(), context(), builder(), *_values);
         return rush::visit(node, gen);
      }

      template <typename AstNodeT, typename GeneratorT>
      GeneratorT& generate(AstNodeT const& node, GeneratorT&& gen) {
         return generate(node, gen);
      }

      void initialize(
         llvm::Module& module_,
         llvm::LLVMContext& context,
         llvm::IRBuilder<>& builder,
         value_map_type& values) {
            _module = &module_;
            _context = &context;
            _builder = &builder;
            _values = &values;
         }

   private:
      llvm::Module* _module;
      llvm::LLVMContext* _context;
      llvm::IRBuilder<>* _builder;
      value_map_type* _values;
   };
}

#endif // RUSH_IRGEN_GENERATOR_HPP
