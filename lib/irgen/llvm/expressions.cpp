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
#include "rush/irgen/llvm/types.hpp"
#include "rush/irgen/llvm/expressions.hpp"

#include "fmt/format.h"

using namespace rush;


// class llvm_integral_binary_operation : public irgen::llvm_ir_generator<llvm::Value> {
// private:
//    ast::expression const& _binexpr;
//    llvm::Value* _result;
//    llvm::Value* _lhs;
//    llvm::Value* _rhs;

// public:
//    llvm_integral_binary_operation(ast::expression const& expr, llvm::Value* lhs, llvm::Value* rhs) {

//    }
// }

// class llvm_floating_binary_operation : public irgen::llvm_ir_generator<llvm::Value> {
// private:
//    llvm::Value* _result;
//    llvm::Value* _lhs;
//    llvm::Value* _rhs;

// public:
//    llvm_ir_right_shift(llvm::Value* lhs, llvm::Value* rhs)
//       : _result { nullptr }
//       , _lhs { lhs }
//       , _rhs { rhs } {}

//    virtual llvm::Value* result() const noexcept override {
//       return _result;
//    }

//    virtual void visit_builtin_integral_type(ast::builtin_integral_type const& type) override {
//       _result = builder().CreateLShr(_lhs, _rhs, "lshrtmp");
//    }
// };

class llvm_ir_modulo : public irgen::llvm_ir_generator<llvm::Value> {
private:
   llvm::Value* _result;
   llvm::Value* _lhs;
   llvm::Value* _rhs;

public:
   llvm_ir_modulo(llvm::Value* lhs, llvm::Value* rhs)
      : _result { nullptr }
      , _lhs { lhs }
      , _rhs { rhs } {}

   virtual llvm::Value* result() const noexcept override {
      return _result;
   }

   virtual void visit_builtin_integral_type(ast::builtin_integral_type const& type) override {
      _result = type.is_signed()
         ? builder().CreateSRem(_lhs, _rhs, "modtmp")
         : builder().CreateURem(_lhs, _rhs, "modtmp");
   }
};

class llvm_ir_addition : public irgen::llvm_ir_generator<llvm::Value> {
private:
   llvm::Value* _result;
   llvm::Value* _lhs;
   llvm::Value* _rhs;
public:
   llvm_ir_addition(llvm::Value* lhs, llvm::Value* rhs)
      : _result { nullptr }
      , _lhs { lhs }
      , _rhs { rhs } {}

   virtual llvm::Value* result() const noexcept override {
      return _result;
   }

   virtual void visit_builtin_integral_type(ast::builtin_integral_type const& type) override {
      _result = builder().CreateAdd(_lhs, _rhs, "addtmp");
   }

   virtual void visit_builtin_floating_type(ast::builtin_floating_point_type const& type) override {
      _result = builder().CreateFAdd(_lhs, _rhs, "faddtmp");
   }
};

class llvm_ir_subtraction : public irgen::llvm_ir_generator<llvm::Value> {
private:
   llvm::Value* _result;
   llvm::Value* _lhs;
   llvm::Value* _rhs;

public:
   llvm_ir_subtraction(llvm::Value* lhs, llvm::Value* rhs)
      : _result { nullptr }
      , _lhs { lhs }
      , _rhs { rhs } {}

   virtual llvm::Value* result() const noexcept override {
      return _result;
   }

   virtual void visit_builtin_integral_type(ast::builtin_integral_type const& type) override {
      _result = builder().CreateSub(_lhs, _rhs, "subtmp");
   }

   virtual void visit_builtin_floating_type(ast::builtin_floating_point_type const& type) override {
      _result = builder().CreateFSub(_lhs, _rhs, "fsubtmp");
   }
};

class llvm_ir_division : public irgen::llvm_ir_generator<llvm::Value> {
private:
   llvm::Value* _result;
   llvm::Value* _lhs;
   llvm::Value* _rhs;

public:
   llvm_ir_division(llvm::Value* lhs, llvm::Value* rhs)
      : _result { nullptr }
      , _lhs { lhs }
      , _rhs { rhs } {}

   virtual llvm::Value* result() const noexcept override {
      return _result;
   }

   virtual void visit_builtin_integral_type(ast::builtin_integral_type const& type) override {
      _result = type.is_signed()
              ? builder().CreateSDiv(_lhs, _rhs, "sdivtmp")
              : builder().CreateUDiv(_lhs, _rhs, "udivtmp");
   }

   virtual void visit_builtin_floating_type(ast::builtin_floating_point_type const& type) override {
      _result = builder().CreateFDiv(_lhs, _rhs, "fdivtmp");
   }
};

class llvm_ir_multiplication : public irgen::llvm_ir_generator<llvm::Value> {
private:
   llvm::Value* _result;
   llvm::Value* _lhs;
   llvm::Value* _rhs;

public:
   llvm_ir_multiplication(llvm::Value* lhs, llvm::Value* rhs)
      : _result { nullptr }
      , _lhs { lhs }
      , _rhs { rhs } {}

   virtual llvm::Value* result() const noexcept override {
      return _result;
   }

   virtual void visit_builtin_integral_type(ast::builtin_integral_type const& type) override {
      _result = builder().CreateMul(_lhs, _rhs, "multmp");
   }

   virtual void visit_builtin_floating_type(ast::builtin_floating_point_type const& type) override {
      _result = builder().CreateFMul(_lhs, _rhs, "fmultmp");
   }
};

class llvm_ir_left_shift : public irgen::llvm_ir_generator<llvm::Value> {
private:
   llvm::Value* _result;
   llvm::Value* _lhs;
   llvm::Value* _rhs;

public:
   llvm_ir_left_shift(llvm::Value* lhs, llvm::Value* rhs)
      : _result { nullptr }
      , _lhs { lhs }
      , _rhs { rhs } {}

   virtual llvm::Value* result() const noexcept override {
      return _result;
   }

   virtual void visit_builtin_integral_type(ast::builtin_integral_type const& type) override {
      _result = builder().CreateShl(_lhs, _rhs, "shltmp");
   }
};

class llvm_ir_right_shift : public irgen::llvm_ir_generator<llvm::Value> {
private:
   llvm::Value* _result;
   llvm::Value* _lhs;
   llvm::Value* _rhs;

public:
   llvm_ir_right_shift(llvm::Value* lhs, llvm::Value* rhs)
      : _result { nullptr }
      , _lhs { lhs }
      , _rhs { rhs } {}

   virtual llvm::Value* result() const noexcept override {
      return _result;
   }

   virtual void visit_builtin_integral_type(ast::builtin_integral_type const& type) override {
      _result = builder().CreateLShr(_lhs, _rhs, "lshrtmp");
   }
};



namespace rush::irgen::llvm {

   void llvm_ir_expression_generator::visit_unary_expr(ast::unary_expression const& expr) {
      auto operand = generate(expr.operand(), *this).result();
      switch (expr.opkind()) {
         case ast::unary_operator::positive:
            _result = operand;
            break;
         case ast::unary_operator::negative: {
            auto name = fmt::format("{}.neg", operand->getName().str());
            _result = builder().CreateNeg(operand, name);
         } break;
         default: break;
      }
   }

   void llvm_ir_expression_generator::visit_binary_expr(ast::binary_expression const& expr) {
      auto lhs = rush::visit(expr.left_operand(), *this).result();
      auto rhs = rush::visit(expr.right_operand(), *this).result();
      if (!lhs || !rhs) return;

      // if (expr.left_operand().result_type() != expr.result_type())
      //    lhs = builder().CreateCast(llvm::Instruction::CastOps::)

      _result = nullptr;
      switch (expr.opkind()) {
         case ast::binary_operator::exponentiation: break;
         case ast::binary_operator::modulo:
            _result = generate(expr.result_type(), llvm_ir_modulo { lhs, rhs }).result(); break;
         case ast::binary_operator::addition:
            _result = generate(expr.result_type(), llvm_ir_addition { lhs, rhs }).result(); break;
         case ast::binary_operator::subtraction:
            _result = generate(expr.result_type(), llvm_ir_subtraction { lhs, rhs }).result(); break;
         case ast::binary_operator::multiplication:
            _result = generate(expr.result_type(), llvm_ir_multiplication { lhs, rhs }).result(); break;
         case ast::binary_operator::division:
            _result = generate(expr.result_type(), llvm_ir_division { lhs, rhs }).result(); break;
         case ast::binary_operator::left_shift:
            _result = generate(expr.result_type(), llvm_ir_left_shift { lhs, rhs }).result(); break;
         case ast::binary_operator::right_shift:
            _result = generate(expr.result_type(), llvm_ir_right_shift { lhs, rhs }).result(); break;
         case ast::binary_operator::less_than: break;
         case ast::binary_operator::less_equals: break;
         case ast::binary_operator::greater_than: break;
         case ast::binary_operator::greater_equals: break;
         case ast::binary_operator::equal:
            _result = lhs->getType()->isFloatingPointTy() || rhs->getType()->isFloatingPointTy()
                    ? builder().CreateFCmpOEQ(lhs, rhs, "cmpeq")
                    : builder().CreateICmpEQ(lhs, rhs, "cmpeq");
            break;
         case ast::binary_operator::not_equal:
            _result = lhs->getType()->isFloatingPointTy() || rhs->getType()->isFloatingPointTy()
                    ? builder().CreateFCmpONE(lhs, rhs, "cmpeq")
                    : builder().CreateICmpNE(lhs, rhs, "cmpeq");
            break;
         case ast::binary_operator::bitwise_and: break;
         case ast::binary_operator::bitwise_xor: break;
         case ast::binary_operator::bitwise_or: break;
         case ast::binary_operator::inclusive_range: break;
         case ast::binary_operator::exclusive_range: break;
         case ast::binary_operator::logical_and: break;
         case ast::binary_operator::logical_or: break;
         case ast::binary_operator::assignment: break;
         case ast::binary_operator::mathexp_assignment: break;
         case ast::binary_operator::compound_multiplication: break;
         case ast::binary_operator::compound_subtraction: break;
         case ast::binary_operator::compound_division: break;
         case ast::binary_operator::compound_addition: break;
         case ast::binary_operator::compound_modulo: break;
         case ast::binary_operator::compound_left_shift: break;
         case ast::binary_operator::compound_right_shift: break;
         case ast::binary_operator::compound_bitwise_or: break;
         case ast::binary_operator::compound_bitwise_xor: break;
         case ast::binary_operator::compound_bitwise_and: break;
      }
   }

   void llvm_ir_expression_generator::visit_invoke_expr(ast::invoke_expression const& expr) {
      struct function_call_generator : llvm_ir_generator<::llvm::CallInst>{
         ::llvm::CallInst* _result = nullptr;
         ast::invoke_expression const& _expr;
      public:
         function_call_generator(ast::invoke_expression const& expr)
            : _expr { expr } {}

         virtual ::llvm::CallInst* result() const noexcept override {
            return _result;
         }

         virtual void visit_identifier_expr(ast::identifier_expression const& expr) override {
            auto function = static_cast<::llvm::Function*>(lookup(expr.declaration()));
               std::vector<::llvm::Value*> args;
               for (auto& arg : _expr.arguments())
                  args.push_back(generate(arg, llvm_ir_expression_generator {}).result());
               _result = builder().CreateCall(function, args, fmt::format("{}.retval", function->getName().str()));
         }
      };

      _result = generate(expr.callable(), function_call_generator { expr }).result();
   }

   void llvm_ir_expression_generator::visit_identifier_expr(ast::identifier_expression const& expr) {
      if ((_result = lookup(expr.declaration()))) {
         _result = builder().CreateLoad(_result, std::string { expr.name() });
      }
   }

   void llvm_ir_expression_generator::visit_literal_expr(ast::string_literal_expression const& expr) {
      // _result = nullptr;
      // auto type = ::llvm::ArrayType::get(
      //    ::llvm::IntegerType::getInt8Ty(context()),
      //    expr.value().length());

      // _result = builder().CreateGlobalString(expr.value(), "str");

      // std::vector<char> data;
      // std::copy(expr.value().begin(), expr.value().end(), std::back_inserter(data));
      // ::llvm::StringLiteral::
      // _result = ::llvm::ConstantArray::get(type, context(), data);
   }

   void llvm_ir_expression_generator::visit_literal_expr(ast::boolean_literal_expression const& expr) {
      _result = nullptr;
      auto type = generate(expr.result_type(), llvm_ir_type_generator {}).result();
      if (type) _result = ::llvm::ConstantInt::get(type, (int)expr.value(), false);
   }

   void llvm_ir_expression_generator::visit_literal_expr(ast::integer_literal_expression const& expr) {
      _result = nullptr;
      auto type = generate(expr.result_type(), llvm_ir_type_generator {}).result();
      if (type) _result = ::llvm::ConstantInt::get(type, expr.value(), expr.is_signed());
   }

   void llvm_ir_expression_generator::visit_literal_expr(ast::floating_literal_expression const& expr) {
      _result = nullptr;
      switch (expr.result_type().as<ast::builtin_floating_point_type>()->fpkind()) {
         case ast::floating_point_kind::ieee16: break;
         case ast::floating_point_kind::ieee32:
            _result = ::llvm::ConstantFP::get(context(),
               ::llvm::APFloat((float)expr.value()));
            break;
         case ast::floating_point_kind::ieee64:
            _result = ::llvm::ConstantFP::get(context(),
               ::llvm::APFloat((double)expr.value()));
            break;
         case ast::floating_point_kind::ieee80: break;
         case ast::floating_point_kind::ieee128: break;
         case ast::floating_point_kind::ppc128: break;
      }
   }
} // rush::irgen

