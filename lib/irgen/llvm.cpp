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
#include "rush/irgen/llvm.hpp"
#include "rush/irgen/llvm/types.hpp"
#include "rush/irgen/llvm/expressions.hpp"
#include "rush/irgen/llvm/declarations.hpp"
#include "rush/irgen/llvm/generator.hpp"
#include "rush/ast/traversal.hpp"
#include "rush/ast/module.hpp"
#include "rush/ast/source.hpp"
#include "rush/ast/context.hpp"
#include "rush/ast/declarations.hpp"

#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"

#include <unordered_map>

using namespace rush;

struct ::irgen::irgenerator_result::impl {
   std::unique_ptr<::llvm::LLVMContext> context;
   std::unique_ptr<::llvm::Module> module;
};

class llvm_ir_module_generator : public irgen::llvm_ir_generator<llvm::Module> {
   using value_map_type = std::unordered_map<ast::nominal_declaration const*, llvm::Value*>;
private:
   std::unique_ptr<::llvm::Module> _module;
   ::llvm::IRBuilder<> _builder;
   ::llvm::LLVMContext& _context;
   value_map_type _values;

public:
   llvm_ir_module_generator(::llvm::LLVMContext& ctx)
      : _module { nullptr }
      , _context { ctx }
      , _builder { ctx }
      , _values { } {}

   virtual llvm::Module* result() const override {
      return _module.get();
   }

   virtual std::unique_ptr<::llvm::Module> module_() {
      return std::move(_module);
   }

   virtual void visit_module(ast::module_node const& mdl) override {
      _module = std::make_unique<::llvm::Module>(std::string { mdl.name() }, _context);
      traverse(mdl);
   }

   virtual void visit_source(ast::source_node const& src) override {
      if (_module) {
         _module->setSourceFileName(std::string { src.id() });
         initialize(*_module, _context, _builder, _values);

         auto type = ::llvm::FunctionType::get(
            ::llvm::IntegerType::getInt32Ty(_context),
               std::vector<::llvm::Type*> { ::llvm::IntegerType::getInt8PtrTy(_context) }, false );
         auto func = ::llvm::Function::Create(type,
            ::llvm::GlobalValue::LinkageTypes::ExternalLinkage,
            "puts", _module.get());
         func->addFnAttr(::llvm::Attribute::NoUnwind);
         // func->addAttribute(2, ::llvm::Attribute::NoUnwind);

         traverse(src);
      }
   }

   virtual void visit_function_decl(ast::function_declaration const& decl) override {
      generate(decl, irgen::llvm::llvm_ir_declaration_generator {});
   }
};

namespace rush::irgen {
   irgenerator_result::~irgenerator_result() = default;
   irgenerator_result::irgenerator_result(std::unique_ptr<impl> pimpl)
      : _pimpl { std::move(pimpl) } {}

   void irgenerator_result::dump() {
      _pimpl->module->print(::llvm::outs(), nullptr);
   }

   irgenerator_result genllvm(rush::syntax_analysis const& ast) {
      auto pimpl = std::make_unique<irgenerator_result::impl>();
      pimpl->context = std::make_unique<::llvm::LLVMContext>();
      pimpl->module = rush::visit(ast, llvm_ir_module_generator { *pimpl->context }).module_();
      return { std::move(pimpl) };
   }
}
