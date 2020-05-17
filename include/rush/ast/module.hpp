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

#ifndef RUSH_AST_MODULE_HPP
#define RUSH_AST_MODULE_HPP

#include "rush/ast/decls/import.hpp"
#include "rush/ast/decls/module.hpp"
#include "rush/ast/decls/declaration.hpp"
#include "rush/ast/decls/undeclared.hpp"

#include <vector>
#include <algorithm>

namespace rush::ast {
   class module : public ast::node {
   public:
      module(std::string id)
         : _id { std::move(id) } {}

      std::string const& id() const noexcept {
         return _id;
      }

      void push_back(std::unique_ptr<ast::import_declaration> imp) {
         _imports.push_back(std::move(imp));
      }

      void push_back(std::unique_ptr<ast::declaration> decl, ast::module_access access) {
         _decls.push_back(std::make_unique<ast::module_declaration>(std::move(decl), access));
      }

      std::vector<std::unique_ptr<import_declaration>> const& imports() const noexcept {
         return _imports;
      }

      std::vector<std::unique_ptr<module_declaration>> const& declarations() const noexcept {
         return _decls;
      }

      ast::nominal_declaration const* undeclared_identifier(std::string name) {
         auto udid = std::make_unique<ast::undeclared_identifier>(std::move(name));
         push_back(std::move(udid), ast::module_access::internal);
         return reinterpret_cast<ast::nominal_declaration const*>(_decls.back().get());
      }

      using node::accept;
      virtual void accept(ast::visitor& v) const override {
         v.visit_module(*this);
      }

      virtual void attach(ast::node&, ast::context& ctx) override {
         std::for_each(_imports.begin(), _imports.end(), [this, &ctx](auto& imp) { imp->attach(*this, ctx); });
         std::for_each(_decls.begin(), _decls.end(), [this, &ctx](auto& decl) { decl->attach(*this, ctx); });
      }

      virtual void detach(ast::node&, ast::context& ctx) override {
         std::for_each(_imports.begin(), _imports.end(), [this, &ctx](auto& imp) { imp->detach(*this, ctx); });
         std::for_each(_decls.begin(), _decls.end(), [this, &ctx](auto& decl) { decl->detach(*this, ctx); });
      }

   private:
      std::vector<std::unique_ptr<import_declaration>> _imports;
      std::vector<std::unique_ptr<module_declaration>> _decls;
      std::string _id;
   };
} // rush::ast

#endif // RUSH_AST_MODULE_HPP
