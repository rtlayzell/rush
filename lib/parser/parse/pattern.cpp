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
#include "rush/parser/parser.hpp"
#include "rush/diag/syntax_error.hpp"

#include <vector>

namespace ptrns = rush::ast::ptrns;
namespace errs = rush::diag::errs;



namespace rush {
   rush::parse_result<ast::pattern> parser::parse_storage_pattern(std::string decl_type) {
      auto result = parse_pattern_list([this, &decl_type]()
         -> rush::parse_result<ast::pattern> {
            auto tok = peek_skip_indent();
            if (tok.is_identifier()) {
               return parse_named_pattern(decl_type);
            } else if (tok.is(symbols::left_bracket)) {
               return parse_destructure_pattern();
            }
            return errs::expected(tok, "storage pattern");
         });
      if (result.failed()) return std::move(result);

      if (peek_skip_indent().is(symbols::colon))
         result = parse_type_annotation_pattern(std::move(result));
      return std::move(result);
   }

   rush::parse_result<ast::pattern> parser::parse_argument_pattern() {
      return errs::not_supported(_eof, "argument pattern");
   }

   rush::parse_result<ast::pattern> parser::parse_parameter_pattern() {
      return errs::not_supported(_eof, "parameter pattern");
   }

   rush::parse_result<ast::pattern> parser::parse_named_pattern(std::string decl_type) {
      assert(peek_skip_indent().is_identifier() && "expected identifier.");

      auto ident = next_skip_indent();
      auto decl = ptrns::named(ident.text());
      if (!_scope.insert({ *decl })) {
         if (decl_type == "variable") return errs::local_variable_name_previously_defined(ident);
         if (decl_type == "constant") return errs::local_constant_name_previously_defined(ident);
         return errs::internal_parse_error(ident);
      }

      return std::move(decl);
   }

   rush::parse_result<ast::pattern> parser::parse_discard_pattern() {
      if (!consume_skip_indent(symbols::underscore))
         return errs::expected(peek_skip_indent(), "_");
      return ptrns::discard();
   }

   rush::parse_result<ast::pattern> parser::parse_binding_pattern(rush::parse_result<ast::pattern> lhs) {
      return errs::not_supported(_eof, "binding pattern");
   }

   rush::parse_result<ast::pattern> parser::parse_destructure_pattern() {
      assert(peek_skip_indent().is(symbols::left_bracket) && "expected '{' parsing destructure pattern.");
      next_skip_indent(); // consume '{'

      auto result = parse_pattern_list([this]()
         -> rush::parse_result<ast::pattern> {
            auto tok = peek_skip_indent();
            auto result = rush::parse_result<ast::pattern> {};
            if (tok.is_identifier()) {
               result = parse_named_pattern("constant");
            } else if (tok.is(symbols::underscore)) {
               result = parse_discard_pattern();
            } else if (tok.is(symbols::left_bracket)) {
               result = parse_destructure_pattern();
            } else return errs::expected(tok, "storage pattern");

            if (result.failed()) return std::move(result);
            if (peek_skip_indent().is(symbols::equals))
               result = parse_binding_pattern(std::move(result));

            return std::move(result);
         });

      // consume '}'
      if (!consume_skip_indent(symbols::right_bracket))
         return errs::expected_closing_bracket(peek_skip_indent());

      return ptrns::destructure(std::move(result));
   }

   rush::parse_result<ast::pattern> parser::parse_type_annotation_pattern(rush::parse_result<ast::pattern> lhs) {
      auto type_result = parse_type_annotation();
      if (type_result.failed())
         return std::move(type_result).errors();
      if (type_result.is_undefined())
         return errs::expected_type_annotation(peek_skip_indent());

      return ptrns::annotation(std::move(lhs), type_result.type());
   }

   rush::parse_result<ast::pattern> parser::parse_pattern_list(
      rush::function_ref<rush::parse_result<ast::pattern>()> parseFn) {
         std::vector<rush::parse_result<ast::pattern>> results;
         do { results.push_back(parseFn()); }
         while (consume_skip_indent(symbols::comma));

         auto it = std::find_if(
            results.begin(), results.end(),
            [](auto& r) { return r.failed(); });
         if (it != results.end()) return std::move(*it);

         std::vector<std::unique_ptr<ast::pattern>> patterns;
         std::move(results.begin(), results.end(), std::back_inserter(patterns));
         return ptrns::list(std::move(patterns));
      }
}
