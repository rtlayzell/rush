#include "rush/irgen/llvm/declarations.hpp"
#include "rush/irgen/llvm/statements.hpp"
#include "rush/irgen/llvm/expressions.hpp"
#include "rush/irgen/llvm/types.hpp"

#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"

#include "fmt/format.h"

namespace rush::irgen::llvm {
   struct llvm_ir_declaration_generator::parameter_binder
   : public llvm_ir_generator<> {
   private:
      ::llvm::Function::arg_iterator _first;
      ::llvm::Function::arg_iterator _last;
      llvm_ir_declaration_generator& _gen;
      ::llvm::IRBuilder<> _builder;
   public:
      parameter_binder(
         llvm_ir_declaration_generator& gen,
         ::llvm::Function& func,
         ::llvm::iterator_range<::llvm::Function::arg_iterator> args)
         : _first { args.begin() }
         , _last { args.end() }
         , _gen { gen }
         , _builder {
            &func.getEntryBlock(),
               func.getEntryBlock().begin() } {}

      virtual void visit_named_ptrn(ast::named_pattern const& ptrn) override {
         if (_first != _last) {
            _first->setName(std::string { ptrn.name() });
            auto type = _gen.generate(ptrn.type(), irgen::llvm::llvm_ir_type_generator {}).result();
            auto value = _builder.CreateAlloca(type, 0, std::string { fmt::format("{}.addr", ptrn.name()) });
            _builder.CreateStore(_first, value);
            _gen.insert(ptrn, value);
            ++_first;
         }
      }

      virtual void visit_object_destructure_ptrn(ast::object_destructure_pattern const& ptrn) override {
         if (_first != _last) {
            _first->setName("$obj-destr");
            ++_first;
         }
      }

      virtual void visit_array_destructure_ptrn(ast::array_destructure_pattern const& ptrn) override {
         if (_first != _last) {
            _first->setName("$arr-destr");
            ++_first;
         }
      }
   };

   void llvm_ir_declaration_generator::visit_storage_decl(ast::storage_declaration const& decl) {
      struct named_pattern_extractor : public ast::traversal {
         std::vector<ast::named_pattern const*> names;
         virtual void visit_named_ptrn(ast::named_pattern const& ptrn) override {
            names.push_back(&ptrn);
         }
      };

      struct initializer : public llvm_ir_generator<::llvm::Value> {
         std::vector<ast::named_pattern const*> names;
         virtual void visit_named_ptrn(ast::named_pattern const& ptrn) override {
            names.push_back(&ptrn);
         }

         virtual void visit_binding_ptrn(ast::binding_pattern const& ptrn) override {
            ptrn.pattern().accept(*this);
            auto value = generate(ptrn.expression(), irgen::llvm::llvm_ir_expression_generator {}).result();
            for (auto name : names) { builder().CreateStore(value, lookup(*name)); }
            names.clear();
         }
      };

      auto ptrns = std::move(rush::visit(decl.pattern(), named_pattern_extractor {}).names);
      for (auto ptrn : ptrns) {
         auto type = generate(ptrn->type(), irgen::llvm::llvm_ir_type_generator {}).result();
         auto value = builder().CreateAlloca(type, 0, fmt::format("{}.addr", ptrn->name()));
         insert(*ptrn, value);
      }

      generate(decl.pattern(), initializer {});
   }

   void llvm_ir_declaration_generator::visit_function_decl(ast::function_declaration const& decl) {
      auto type = (::llvm::FunctionType*)generate(
         decl.type(), irgen::llvm::llvm_ir_type_generator {})
         .result();

      auto function = ::llvm::Function::Create(type,
         ::llvm::Function::ExternalLinkage,
         std::string { decl.name() },
         module_());

      auto block = ::llvm::BasicBlock::Create(context(), "entry", function);
      builder().SetInsertPoint(block);

      insert(decl, function);
      rush::visit(decl.parameters(),
         parameter_binder { *this, *function, function->args() });
      generate(decl.body(), llvm_ir_statement_generator {});
   }
} // rush::irgen::llvm
