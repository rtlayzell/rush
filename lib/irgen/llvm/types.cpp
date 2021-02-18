#include "rush/irgen/llvm/types.hpp"

namespace rush::irgen::llvm {
   struct extract_tuple_types : public llvm_ir_generator<std::vector<::llvm::Type*>> {
   private:
      std::vector<::llvm::Type*> _types;
   public:
      extract_tuple_types() : _types { } {}

      std::vector<::llvm::Type*> types() {
         return std::move(_types);
      }

      void visit_tuple_type(ast::tuple_type const& type) {
         auto tts = type.types();
         std::transform(tts.begin(), tts.end(), std::back_inserter(_types),
            [this] (auto t) { return generate(t, llvm_ir_type_generator {}).result(); });
      }
   };

   void llvm_ir_type_generator::visit_builtin_void_type(ast::builtin_void_type const&) {
      _result = builder().getVoidTy();
   }

   void llvm_ir_type_generator::visit_builtin_bool_type(ast::builtin_bool_type const&) {
      _result = builder().getInt1Ty();
   }

   // void llvm_ir_type_generator::visit_builtin_string_type(ast::builtin_string_type const&) {
   //    _result = nullptr;
   // }

   void llvm_ir_type_generator::visit_function_type(ast::function_type const& type) {
      auto ret_type = rush::visit(type.return_type(), *this).result();
      auto param_types = generate(type.parameter_types(), extract_tuple_types {}).types();
      _result = ::llvm::FunctionType::get(ret_type, param_types, false);
   }

   void llvm_ir_type_generator::visit_builtin_string_type(ast::builtin_string_type const& type) {
      _result = ::llvm::IntegerType::getInt8PtrTy(context());
   }

   void llvm_ir_type_generator::visit_builtin_integral_type(ast::builtin_integral_type const& type) {
      _result = ::llvm::IntegerType::get(context(), type.bit_width());
   }

   void llvm_ir_type_generator::visit_builtin_floating_type(ast::builtin_floating_point_type const& type) {
      switch (type.fpkind()) {
      case ast::floating_point_kind::ieee16: _result = ::llvm::Type::getHalfTy(context()); break;
      case ast::floating_point_kind::ieee32: _result = ::llvm::Type::getFloatTy(context()); break;
      case ast::floating_point_kind::ieee64: _result = ::llvm::Type::getDoubleTy(context()); break;
      case ast::floating_point_kind::ieee80: _result = ::llvm::Type::getX86_FP80Ty(context()); break;
      case ast::floating_point_kind::ieee128: _result = ::llvm::Type::getFP128Ty(context()); break;
      case ast::floating_point_kind::ppc128: _result = ::llvm::Type::getPPC_FP128Ty(context()); break;
      };
   }
} // rush::irgen::llvm
