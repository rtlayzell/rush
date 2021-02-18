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
#include "rush/ast/types/extension.hpp"

#include "rush/ast/decls/class.hpp"
#include "rush/ast/decls/struct.hpp"
#include "rush/ast/decls/field.hpp"
#include "rush/ast/decls/method.hpp"
#include "rush/ast/decls/property.hpp"
#include "rush/ast/decls/parameter.hpp"
#include "rush/ast/decls/constant.hpp"
#include "rush/ast/decls/variable.hpp"
#include "rush/ast/decls/function.hpp"
#include "rush/ast/decls/property.hpp"

#include "rush/ast/exprs/invoke.hpp"
#include "rush/ast/exprs/lambda.hpp"
#include "rush/ast/exprs/list.hpp"
#include "rush/ast/exprs/binary.hpp"

#include "rush/ast/stmts/iteration.hpp"

#include "rush/ast/patterns.hpp"

#include "rush/ast/visitor.hpp"
#include "rush/ast/iterator.hpp"
#include "rush/ast/module.hpp"
#include "rush/ast/context.hpp"


using namespace rush;

class pattern_type_resolver : public ast::visitor {
private:
   ast::type_ref _type;
   ast::context& _context;
   ast::pattern const& _ptrn;

public:
   pattern_type_resolver(ast::pattern const& ptrn, ast::context& context)
      : _type { ast::types::undefined }
      , _context { context }
      , _ptrn { ptrn } {}

   ast::type_ref result() const noexcept { return _type; }

   virtual void visit_ptrn_list(ast::pattern_list const& ptrn) override {
      if (auto p = ptrn.parent()) p->accept(*this);
   }

   virtual void visit_expr_list(ast::expression_list const& expr) override {
      if (auto p = expr.parent()) p->accept(*this);
   }

   virtual void visit_rest_ptrn(ast::rest_pattern const& ptrn) override {
      struct find_last_pattern : public ast::visitor {
         ast::pattern const* result;
         virtual void visit_ptrn_list(ast::pattern_list const& ptrn) override { result = &ptrn.back(); }
         virtual void visit_named_ptrn(ast::named_pattern const& ptrn) override { result = &ptrn; }
      };

      if (auto p = ptrn.parent()) {
         p->accept(*this);
         if (&_ptrn == rush::visit(ptrn.pattern(), find_last_pattern {}).result)
            _type = _context.array_type(_type);
      }
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
      struct find_member : ast::visitor {
         ast::type_ref result;
         std::size_t index;
         std::string_view name;

         find_member(std::size_t pos, std::string_view name)
            : result { ast::types::undefined }
            , index { pos }
            , name { name } {}

         virtual void visit_user_type(ast::user_type const& type) override {
            type.declaration().accept(*this);
         }

         virtual void visit_tuple_type(ast::tuple_type const& type) override {
            auto types = type.types();
            if (index < types.size())
               result = types[index];
         }

         virtual void visit_class_decl(ast::class_declaration const& decl) override {
            auto it = decl.find(name);
            if (it != ast::iterator()) it->accept(*this);
         }

         virtual void visit_struct_decl(ast::struct_declaration const& decl) override {
            auto it = decl.find(name);
            if (it != ast::iterator()) it->accept(*this);
         }

         virtual void visit_constant_field_decl(ast::constant_field_declaration const& decl) override {
            if (auto p = decl.find(name)) result = p->type();
         }

         virtual void visit_variable_field_decl(ast::variable_field_declaration const& decl) override {
            if (auto p = decl.find(name)) result = p->type();
         }

         virtual void visit_property_getter_decl(ast::property_getter_declaration const& decl) override {
            result = decl.type();
         }

         virtual void visit_property_setter_decl(ast::property_setter_declaration const& decl) override {
            result = decl.type();
         }

         virtual void visit_method_decl(ast::method_declaration const& decl) override {
            result = decl.type();
         }
      };

      // todo: find and extract types of the destructured member.
      if (auto p = dynamic_cast<ast::named_pattern const*>(&_ptrn)) {
         _type = rush::visit(ptrn.type(), find_member { ast::index_of(*p, ptrn), p->name() }).result;
      }
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

	virtual void visit_variable_decl(ast::variable_declaration const& decl) override { _type = ast::types::inference_fail; }
	virtual void visit_constant_decl(ast::constant_declaration const& decl) override {
      _type = ast::types::inference_fail;
      auto it = ast::find_ancestor<ast::iteration_statement>(&decl);
      if (it != ast::ancestor_iterator()) {
         auto expr_type = it->expression().result_type();
         if (auto t = expr_type.as<ast::array_type>()) {
            _type = t->underlying_type();
         }
         else if (auto e = dynamic_cast<ast::binary_expression const*>(&it->expression())) {
            if (e->opkind() == ast::binary_operator::inclusive_range
             || e->opkind() == ast::binary_operator::exclusive_range)
               _type = e->result_type();
         }
      }
   }

   virtual void visit_parameter_decl(ast::parameter_declaration const& decl) override {
      _type = ast::types::inference_fail;
      auto it = ast::find_ancestor<ast::property_setter_declaration>(&decl);
      if (it != ast::ancestor_iterator()) {
         // if (auto getter = it->type_declaration().find_property_getter(it->name()))
         //    _type = getter->type();
      }
   }
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

   virtual void visit_rest_ptrn(ast::rest_pattern const& ptrn) override {
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

         virtual void visit_storage_decl(ast::storage_declaration const& decl) override { decl.pattern().accept(*this); }
         virtual void visit_parameter_decl(ast::parameter_declaration const& decl) override { decl.pattern().accept(*this); }

         virtual void visit_identifier_expr(ast::identifier_expression const& expr) override {
            if (!expr.is_undeclared())
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

	virtual void visit_storage_decl(ast::storage_declaration const& decl) override { _decl = &decl; }
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
         ? rush::visit(*parent(), pattern_type_resolver { *this, *context() }).result()
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
            named_pattern_declaration_resolver { *this }).result();
         if (result != nullptr) return result;
      }

      return context()
         ? context()->undeclared_declaration(std::string { name() })
         : nullptr;
   }

   ast::type_ref destructure_pattern::resolve_type() const {
      return parent()
           ? rush::visit(*parent(), pattern_type_resolver { *this, *context() }).result()
           : ast::types::undeclared;
   }
} // rush::ast
