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
#include "rush/ast/ptrns/destructure.hpp"
#include "rush/ast/ptrns/type_annotation.hpp"
#include "rush/ast/types/extension.hpp"
#include "rush/ast/decls/parameter.hpp"
#include "rush/ast/decls/constant.hpp"
#include "rush/ast/decls/variable.hpp"
#include "rush/ast/decls/function.hpp"
#include "rush/ast/exprs/invoke.hpp"
#include "rush/ast/exprs/lambda.hpp"
#include "rush/ast/exprs/list.hpp"
#include "rush/ast/visitor.hpp"
#include "rush/ast/ptrns/list.hpp"
#include "rush/ast/ptrns/named.hpp"
#include "rush/ast/ptrns/binding.hpp"
#include "rush/ast/ptrns/type_annotation.hpp"
#include "rush/ast/iterator.hpp"
#include "rush/ast/module.hpp"

using namespace rush;

class pattern_type_resolver : public ast::visitor {
public:
   pattern_type_resolver()
      : _type { ast::types::undefined } {}

   ast::type_ref result() const noexcept { return _type; }

   virtual void visit_ptrn_list(ast::pattern_list const& ptrn) override {
      if (ptrn.parent()) ptrn.parent()->accept(*this);
   }

   virtual void visit_expr_list(ast::expression_list const& expr) override {
      if (expr.parent()) expr.parent()->accept(*this);
   }

   virtual void visit_binding_ptrn(ast::binding_pattern const& ptrn) override {
      struct parameter_type_finder : public ast::visitor {
      public:
         std::string_view _name;
         ast::type_ref result = ast::types::undefined;
         virtual void visit_named_ptrn(ast::named_pattern const& ptrn) override {
            // if name is empty this is just the first node to accept this visitor, the node we're trying to resolve.
            if (_name.empty()) { _name = ptrn.name(); ptrn.declaration()->accept(*this); }
            else if (_name == ptrn.name()) { result = ptrn.type(); }
         }

         virtual void visit_ptrn_list(ast::pattern_list const& ptrn) override {
            auto it = std::find_if(ptrn.begin(), ptrn.end(), [this](auto& p) {
               p.accept(*this);
               return result != ast::types::undefined;
            });
         }

         virtual void visit_binding_ptrn(ast::binding_pattern const& ptrn) override { ptrn.pattern().accept(*this); }
         virtual void visit_parameter_decl(ast::parameter_declaration const& decl) override { decl.pattern().accept(*this); }
         virtual void visit_type_annotation_ptrn(ast::type_annotation_pattern const& ptrn) override { ptrn.pattern().accept(*this); }
      };

      _type = ptrn.kind() == ast::binding_kind::parameter && ptrn.parent()
            ? rush::visit(ptrn.pattern(), parameter_type_finder {}).result
            : ptrn.expression().result_type();
   }

   virtual void visit_array_destructure_ptrn(ast::array_destructure_pattern const& ptrn) override {
      // todo: find and extract types of the destructured member.
      _type = ptrn.type();
   }

   virtual void visit_object_destructure_ptrn(ast::object_destructure_pattern const& ptrn) override {
      // todo: find and extract types of the destructured member.
      _type = ptrn.type();
   }

   virtual void visit_type_annotation_ptrn(ast::type_annotation_pattern const& ptrn) override {
      struct strip_type_extension : ast::visitor {
         ast::type_ref result;
         strip_type_extension(ast::type_ref type) : result { std::move(type) } {}
         virtual void visit_type_extension(ast::type_extension const& type)
         { result = type.underlying_type(); }
      };

      _type = rush::visit(
         ptrn.type(),
         strip_type_extension { ptrn.type() }).result;
   }

	virtual void visit_constant_decl(ast::constant_declaration const& decl) override { _type = ast::types::inference_fail; }
	virtual void visit_variable_decl(ast::variable_declaration const& decl) override { _type = ast::types::inference_fail; }
   virtual void visit_parameter_decl(ast::parameter_declaration const& decl) override { _type = ast::types::inference_fail; }

private:
   ast::type_ref _type;
};

class named_pattern_declaration_resolver : public ast::visitor {
public:
   named_pattern_declaration_resolver(ast::named_pattern const& name)
      : _decl { nullptr }
      , _named { name } {}

   ast::declaration const* result() const noexcept {
      return _decl;
   }

   virtual void visit_ptrn_list(ast::pattern_list const& ptrn) override {
      if (ptrn.parent()) ptrn.parent()->accept(*this);
   }

   virtual void visit_expr_list(ast::expression_list const& expr) override {
      if (expr.parent()) expr.parent()->accept(*this);
   }

   virtual void visit_named_ptrn(ast::named_pattern const& ptrn) override {
      if (ptrn.parent()) ptrn.parent()->accept(*this);
   }

   virtual void visit_binding_ptrn(ast::binding_pattern const& ptrn) override {
      if (ptrn.parent()) ptrn.parent()->accept(*this);
   }

   virtual void visit_array_destructure_ptrn(ast::array_destructure_pattern const& ptrn) override {
      if (ptrn.parent()) ptrn.parent()->accept(*this);
   }

   virtual void visit_object_destructure_ptrn(ast::object_destructure_pattern const& ptrn) override {
      if (ptrn.parent()) ptrn.parent()->accept(*this);
   }

   virtual void visit_type_annotation_ptrn(ast::type_annotation_pattern const& ptrn) override {
      if (ptrn.parent()) ptrn.parent()->accept(*this);
   }

   virtual void visit_invoke_expr(ast::invoke_expression const& expr) override {
      struct parameter_decl_finder : public ast::visitor {
         std::string_view name;
         ast::declaration const* result = nullptr;

         parameter_decl_finder(std::string_view name)
            : name { std::move(name) } {}

         virtual void visit_ptrn_list(ast::pattern_list const& ptrn) override {
            std::find_if(ptrn.begin(), ptrn.end(), [this](auto& p) {
               p.accept(*this);
               return result != nullptr;
            });
         }

         virtual void visit_named_ptrn(ast::named_pattern const& ptrn) override {
            if (name == ptrn.name()) result = &ptrn;
         }

         virtual void visit_binding_ptrn(ast::binding_pattern const& ptrn) override {
            ptrn.pattern().accept(*this);
         }

         virtual void visit_type_annotation_ptrn(ast::type_annotation_pattern const& ptrn) override {
            ptrn.pattern().accept(*this);
         }

         virtual void visit_lambda_expr(ast::lambda_expression const& expr) override {
            expr.parameters().accept(*this);
         }

         virtual void visit_function_decl(ast::function_declaration const& decl) override {
            decl.parameters().accept(*this);
         }

         virtual void visit_constant_decl(ast::constant_declaration const& decl) override { decl.pattern().accept(*this); }
         virtual void visit_variable_decl(ast::variable_declaration const& decl) override { decl.pattern().accept(*this); }
         virtual void visit_parameter_decl(ast::parameter_declaration const& decl) override { decl.pattern().accept(*this); }

         virtual void visit_identifier_expr(ast::identifier_expression const& expr) override {
            if (!expr.is_unresolved())
               expr.declaration().accept(*this);
         }
      };

      if (expr.callable().result_type().kind() != ast::type_kind::error) {
         _decl = rush::visit(
            expr.callable(),
            parameter_decl_finder { _named.name() }).result;

         if (_decl != nullptr)
            rush::visit(*_decl, *this);
      }
   }

	virtual void visit_constant_decl(ast::constant_declaration const& decl) override { _decl = &decl; }
	virtual void visit_variable_decl(ast::variable_declaration const& decl) override { _decl = &decl; }
   virtual void visit_parameter_decl(ast::parameter_declaration const& decl) override { _decl = &decl; }

private:
   ast::declaration const* _decl;
   ast::named_pattern const& _named;
};

namespace rush::ast {
   ast::type_ref named_pattern::resolve_type() const {
      if (_resolving_type) return ast::types::circular_ref;

      _resolving_type = true;
      auto result_type = parent()
         ? rush::visit(*parent(), pattern_type_resolver {}).result()
         : ast::types::undeclared;
      _resolving_type = false;

      return result_type.kind() == type_kind::error
          && result_type != ast::types::circular_ref
           ? ast::types::inference_fail
           : result_type;
   }

   ast::declaration const* named_pattern::resolve_declaration() const {
      if (parent()) {
         auto result = rush::visit(*parent(),
                       named_pattern_declaration_resolver { *this })
                      .result();
         if (result != nullptr) return result;
      }

      auto miter = ast::find_ancestor<ast::module>(this);
      if (miter != ast::ancestor_iterator<ast::module>())
         return &miter->undeclared_declaration();

      return nullptr;
   }

   ast::type_ref destructure_pattern::resolve_type() const {
      return parent()
           ? rush::visit(*parent(), pattern_type_resolver {}).result()
           : ast::types::undeclared;
   }
} // rush::ast