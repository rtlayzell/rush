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

#ifndef RUSH_AST_DECLS_PROTOTYPE_HPP
#define RUSH_AST_DECLS_PROTOTYPE_HPP

#include "rush/ast/types/type_ref.hpp"

#include <string>

namespace rush::ast {
	class prototype_declaration {
   public:
      prototype_declaration(
         std::string name,
         ast::type_ref ret_type,
         std::vector<ast::type_ref> param_types)
         : _name { std::move(name) }
         , _ret_type { ret_type }
         , _param_type { std::move(param_types) } {}

      virtual void attached(ast::scope& scope, ast::context& context) override {
         _type = context.function_type(*this);
         if (auto type = std::get_if<ast::type_ref>(&_type))
            _type = ast::resolve_named_types(*type, scope, context);
      }

      virtual void detached(ast::context&) override {
         _type = types::undefined;
         detach(*_params);
         detach(*_body);
      }
   private:
      std::string _name;
      ast::type_ref _type;
      std::vector<ast::type_ref
	};

   namespace decls {
      std::unique_ptr<ast::prototype_declaration>
   } // decls
} // rush::ast

#endif // RUSH_AST_DECLS_PROTOTYPE_HPP

