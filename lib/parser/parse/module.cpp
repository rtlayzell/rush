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
#include "rush/ast/source.hpp"
#include "rush/ast/module.hpp"
#include "rush/parser/parser.hpp"
#include "rush/diag/syntax_error.hpp"

namespace decls = rush::ast::decls;
namespace errs = rush::diag::errs;

namespace rush {
   rush::parse_result<ast::module_node> parser::parse_module() {
      std::vector<std::unique_ptr<ast::source_node>> results;
      while (peek_skip_indent().is_not(symbols::eof)) {
         auto result = parse_source();
         if (result.failed())
            return std::move(result).as<ast::module_node>();
         results.push_back(std::move(result));
      }

      return ast::module_("main", std::move(results));
   }

   rush::parse_result<ast::source_node> parser::parse_source() {
      auto tok = peek_skip_indent();
      auto& source = tok.source();
      std::vector<std::unique_ptr<ast::node>> results;

      for (
         ; tok.is_not(symbols::eof)
         && &source == &tok.source()
         ;  tok = peek_skip_indent()) {

         while (consume_skip_indent(symbols::semi_colon)); // ignore stray semi-colons
         auto exported = consume_skip_indent(keywords::export_);
         auto decl_result = parse_toplevel_decl();
         if (decl_result.failed()) {
            auto expr_result = parse_expr();
            if (expr_result.failed())
               return std::move(decl_result).as<ast::source_node>();

            results.push_back(
               std::move(expr_result).as<ast::node>());
         }
         else {
            results.push_back(exported
               ? decls::exported(std::move(decl_result))
               : decls::internal(std::move(decl_result)));
         }
      }

      return ast::source(source, std::move(results));
   }

   rush::parse_result<ast::import_declaration> parser::parse_import_decl() {
      assert(peek_skip_indent().is(keywords::import_) && "expected 'import' keyword");
      next_skip_indent(); // consume import keyword.

      // parse_qualified_name();
      auto tok = peek_skip_indent();
      if (!tok.is_identifier())
         return errs::expected_module_name(tok);

      std::string name = tok.text();
      next_skip_indent();

      while ((tok = peek_skip_indent()).is(symbols::period)) {
         next_skip_indent();
         tok = peek_skip_indent();
         if (!tok.is_identifier())
            return errs::expected_qualified_name(tok);

         name += ".";
         name += tok.text();
         next_skip_indent();
      }

      return decls::import(std::move(name));
   }

}
