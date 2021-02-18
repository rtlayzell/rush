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
#include "rush/ast/exprs/identifier.hpp"
#include "rush/ast/context.hpp"
#include "rush/ast/scope.hpp"
#include "rush/ast/visitor.hpp"
#include "rush/ast/iterator.hpp"

#include "rush/ast/decls/class.hpp"
#include "rush/ast/decls/struct.hpp"
#include "rush/ast/decls/field.hpp"
#include "rush/ast/decls/method.hpp"
#include "rush/ast/decls/property.hpp"
#include "rush/ast/exprs/member.hpp"
#include "rush/ast/exprs/invoke.hpp"

using namespace rush;

class identifier_scope_resolution : public ast::visitor {
private:
   ast::scope const& _scope;
   ast::context& _context;
   ast::identifier_expression const* _ident;
   ast::nominal_declaration const* _result;
   ast::node const* _prev;
   bool _is_scoped;

public:
   identifier_scope_resolution(
      ast::scope const& scope,
      ast::context& context)
      : _scope { scope }
      , _context { context }
      , _ident { nullptr }
      , _result { nullptr }
      , _is_scoped { false } {}

   ast::nominal_declaration const& result() {
      return *_result;
   }

   virtual void visit_module(ast::module_node const&) override { /* block from re-entry */ }
   virtual void visit_source(ast::source_node const&) override { /* block from re-entry */ }
   virtual void visit_expr_list(ast::expression_list const&) override { /* block from re-entry */ }
   virtual void visit_block_stmt(ast::statement_block const&) override { /* block from re-entry */ }

   virtual void visit_identifier_expr(ast::identifier_expression const& expr) override {
      if (auto p = expr.parent()) {
         _prev = _ident = &expr;
         p->accept(*this);

         if (!_is_scoped) {
            auto& sym = _scope.lookup(_ident->name());
            _result = sym.is_undefined()
               ? _context.undeclared_declaration(std::string { _ident->name() })
               : sym.is_overloaded()
               ? _context.overloaded_declaration(std::string { _ident->name() }, sym.begin(), sym.end())
               : &(**sym.begin());
         }
      }
   }

   virtual void visit_invoke_expr(ast::invoke_expression const& expr) override {
      if (auto p = expr.parent()) {
         _prev = &expr;
         p->accept(*this);
      }
   }

   virtual void visit_member_access_expr(ast::member_access_expression const& expr) override {
      if (_prev != &expr.expression()) {
         _is_scoped = true;
      } else if (auto p = expr.parent()) {
         _prev = &expr;
         p->accept(*this);
      }
   }
};

class identifier_member_resolution : public ast::visitor {
private:
   ast::identifier_expression const* _ident;
   ast::nominal_declaration const* _result;
   ast::node const* _prev;
   ast::context& _context;

public:
   identifier_member_resolution(ast::context& context)
      : _result { nullptr }
      , _prev { nullptr }
      , _ident { nullptr }
      , _context { context } {}

   ast::nominal_declaration const* result() {
      return !_result
         ? _context.undeclared_declaration(
            std::string { _ident->name() })
         : _result;
   }

   virtual void visit_expr_list(ast::expression_list const&) override { /* block from re-entry */ }
   virtual void visit_block_stmt(ast::statement_block const&) override { /* block from re-entry */ }

   virtual void visit_identifier_expr(ast::identifier_expression const& expr) override {
      _ident = &expr;
      if (auto p = expr.parent()) {
         _prev = &expr;
         p->accept(*this);
      }
   }

   virtual void visit_user_type(ast::user_type const& type) override {
      _prev = &type;
      type.declaration().accept(*this);
   }

   virtual void visit_member_access_expr(ast::member_access_expression const& expr) override {
      if (_prev == &expr.member()) {
         _prev = &expr;
         expr.expression().result_type().accept(*this);
      } else if (auto p = expr.parent()) {
         _prev = &expr;
         p->accept(*this);
      }
   }

   virtual void visit_invoke_expr(ast::invoke_expression const& expr) override {
      if (auto p = expr.parent()) {
         _prev = &expr;
         p->accept(*this);
         // todo: check is method.
      }
   }

   virtual void visit_class_decl(ast::class_declaration const& decl) override {
      _prev = &decl;
      auto it = decl.find(_ident->name());
      _result = it == ast::iterator()
         ? _context.undeclared_declaration(std::string { _ident->name() })
         : rush::visit(*it, *this).result();
   }

   virtual void visit_struct_decl(ast::struct_declaration const& decl) override {
      _prev = &decl;
      auto it = decl.find(_ident->name());
      _result = it == ast::iterator()
         ? _context.undeclared_declaration(std::string { _ident->name() })
         : rush::visit(*it, *this).result();
   }

   virtual void visit_constant_field_decl(ast::constant_field_declaration const& decl) override {
      if (auto p = decl.find(_ident->name()))
         _result = p;
   }

   virtual void visit_variable_field_decl(ast::variable_field_declaration const& decl) override {
      if (auto p = decl.find(_ident->name()))
         _result = p;
   }

   virtual void visit_property_getter_decl(ast::property_getter_declaration const& decl) override {
      _result = &decl;
   }

   virtual void visit_property_setter_decl(ast::property_setter_declaration const& decl) override {
      _result = &decl;
   }

   virtual void visit_method_decl(ast::method_declaration const& decl) override {
      _result = &decl;
   }
};

namespace rush::ast {
   void identifier_expression::attached(ast::scope& scope, ast::context& context) {
      auto v = identifier_scope_resolution { scope, context };
      _decl = &rush::visit(*this, v).result();
   }

   void identifier_expression::detached(ast::context&) {
      _decl = nullptr;
   }

   ast::nominal_declaration const& identifier_expression::resolve_declaration() const {
      if (_decl != nullptr) return *_decl;
      auto v = identifier_member_resolution { *context() };
      return *(_decl = rush::visit(*this, v).result());
   }
} // rush::ast
