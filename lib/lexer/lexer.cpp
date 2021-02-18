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
#include "rush/core/ascii.hpp"
#include "rush/core/source.hpp"
#include "rush/extra/algorithm.hpp"
#include "rush/extra/iterator.hpp"
#include "rush/extra/function_ref.hpp"
#include "rush/lexer/token.hpp"
#include "rush/lexer/lexer.hpp"
#include "rush/lexer/lexer_iterator.hpp"
#include "rush/lexer/lookahead.hpp"
#include "rush/lexer/indentation.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iterator>
#include <streambuf>
#include <optional>
#include <functional>
#include <string_view>

using namespace rush::ascii;
namespace tok = rush::tokens;

namespace rush {
   template <typename InIter>
   class basic_lexer {
      basic_lexer(basic_lexer const&) = delete;
      void operator = (basic_lexer const&) = delete;

   public:
      explicit basic_lexer(rush::source const& src, lexer_options opts) noexcept
         : _opts { opts }
         , _src { &src } {}

      basic_lexer(basic_lexer&& other) = default;
      basic_lexer& operator = (basic_lexer&& other) = default;

      std::vector<lexical_token> tokenize(InIter first, InIter last) {
         auto toks = std::vector<lexical_token> {};
         _lab = std::make_unique<lookahead_buffer_type>(
            lexer_iterator { first, *first, {} },
            lexer_iterator { last, 0, rush::location::undefined});
         _indent = {};

         if (!eof()) {
            auto tok = next_token();
            for (; !tok.is(symbols::eof); tok = next_token())
               toks.push_back(std::move(tok));
         }

         return std::move(toks);
      }

   private:
      using lookahead_buffer_type = typename rush::lookahead_buffer<
         lexer_iterator<InIter>, 8>;

      lexer_options _opts;
      std::unique_ptr<lookahead_buffer_type> _lab;
      rush::indentation _indent;
      rush::location _loc; // location pinned before every new token.
      rush::source const* _src;

      bool eof() const noexcept {
         return _lab->empty();
      }

      bool is_line_start() const noexcept {
         return !eof() && _lab->peek().location().column() == 1;
      }

      rush::source const& source() const noexcept {
         return *_src;
      }

      rush::location const& location() const noexcept {
         return _loc;
      }

      bool check(codepoint_t cp, std::size_t offset = 0) {
         return !eof() && peek(offset) == cp;
      }

      bool icheck(codepoint_t cp, std::size_t offset = 0) {
         return !eof() && iequal(peek(offset), cp);
      }

      bool check_skip(codepoint_t cp, std::size_t offset = 0) {
         if (icheck(cp, offset)) { skip(offset + 1); return true; }
         return false;
      }

      bool icheck_skip(codepoint_t cp, std::size_t offset = 0) {
         if (icheck(cp, offset)) { skip(offset + 1); return true; }
         return false;
      }

      bool check(std::string_view str, std::size_t offset = 0) {
         for (std::size_t i = 0; i < str.length(); ++i)
            if (!check(str[i], offset + i)) return false;
         return true;
      }

      bool icheck(std::string_view str, std::size_t offset = 0) {
         for (std::size_t i = 0; i < str.length(); ++i)
            if (!icheck(str[i], offset + i)) return false;
         return true;
      }

      bool check_skip(std::string_view str, std::size_t offset = 0) {
         if (check(str, offset)) { skip(offset + str.size()); return true; }
         return false;
      }

      bool icheck_skip(std::string_view str, std::size_t offset = 0) {
         if (icheck(str, offset)) { skip(offset + str.size()); return true; }
         return false;
      }


      auto check_if(function_ref<bool(codepoint_t)> predicate, std::size_t offset = 0)
         -> decltype(predicate(codepoint_t{}), bool{}) {
            return !eof() && predicate(peek(offset));
         }

      codepoint_t peek(std::size_t offset = 0) {
         // assert(!eof() && "unexpected end of source.");
         return offset < _lab->size()
            ? _lab->peek(offset).elem()
            : npos;
      }

      codepoint_t skip(std::size_t offset = 1) {
         assert(!eof() && "unexpected end of source.");
         _lab->skip(offset);
         return !eof() ? peek() : npos;
      }

      std::string scan(std::size_t count) {
         assert(!eof() && "unexpected end of source.");
         std::string result;
         for (; !eof() && count > 0; --count)
            result.push_back(_lab->next().elem());
         return std::move(result);
      }

      rush::location skip_while(function_ref<bool(codepoint_t)> pred) {
         // assert(!eof() && "unexpected end of source.");
         auto cp = npos;
         while (!eof() && pred(cp = _lab->peek().elem())) _lab->skip();
         return location();
      }

      std::string scan_while(function_ref<bool(codepoint_t)> pred) {
         // assert(!eof() && "unexpected end of source.");
         std::string result;
         while (!eof() && pred(peek()))
            result.push_back(_lab->next().elem());
         return std::move(result);
      }

      std::string scan_digits(function_ref<bool(codepoint_t)> pred = is_digit) {
         std::string result;
         while (!eof() && pred(peek())) {
            result.push_back(_lab->next().elem());
            skip_while([this, &pred](auto cp) {
               return cp == '`' && (check_if(pred, 1) || check('`', 1)); });
         }

         return std::move(result);
      }

      lexical_token next_token() {
         auto indent = scan_whitespace();
         if (indent.has_value()) return *indent;

         if (!eof()) {
            _loc = _lab->peek().location(); // pin the location
            auto cp = peek();

            if (is_comment_head(cp)) return scan_comment();
            if (is_newline(cp)) return next_token();
            if (is_quote(cp)) return scan_string_literal();
            if (is_digit(cp)) return scan_numeric_literal();
            if (is_ident_head(cp)) return scan_identifier();

            auto symbol = symbols::to_value({ static_cast<char>(cp) });
            if (symbol != symbols::unknown) { return scan_symbol(symbol); }

            skip(); // skip unknown token.
            return tok::make_error_token("unexpected token", location(), source());
         }

         return _unwind_indentation();
      }

      lexical_token make_dedent() {
         _break = true;
         _indent_diff += 1;
         _indent.decrement();
         return tok::dedent(location(), source());
      }

      lexical_token make_indent() {
         _break = true;
         _indent_diff -= 1;
         _indent.increment();
         return tok::indent(location(), source());
      }

      bool _break = true;
      indentation::difference_type _indent_diff = 0;
      std::optional<lexical_token> scan_whitespace() {
         if (_indent.style() != indentation_style::unknown) {
            if (_indent_diff < 0) { return make_dedent(); }
            if (_indent_diff > 0) { return make_indent(); }
         }
         while (!eof()) {
            skip_while(is_vspace);
            if (eof()) break;

            if (is_line_start()) {
               _loc = _lab->peek().location(); // pin the location
               auto wspace = scan_while(is_hspace);

               if (eof() || is_comment_head(peek())) break;

               if (!is_newline(peek())) {
                  auto indent = _indent.measure(wspace.begin(), wspace.end());
                  if (_indent.style() != indentation_style::unknown)
                     _indent_diff = indentation::difference(indent, _indent);

                  if (_indent_diff == 0 && !_break) {
                     _break = true;
                     return tok::lbreak(location(), source());
                  }

                  if (indent < _indent) { return make_dedent(); }
                  if (indent > _indent) { return make_indent(); }
                  break;
               }
            } else {
               _break = false;
               skip_while(is_hspace);
               if (eof() || !is_vspace(peek()))
                  break;
            }
         }
         return std::nullopt;
      }

      lexical_token scan_comment() {
         assert(!eof() && "unexpected end of source.");
         assert(is_comment_head(peek()) && "expected the leading comment character while attempting to scan a comment.");
         auto comm = scan_while(std::not_fn(is_newline));
         return tok::make_comment_token(comm, location(), source());
      }

      lexical_token scan_identifier() {
         assert(!eof() && "unexpected end of source.");
         assert(is_ident_head(peek()) && "expected a leading identifier character while attempting to scan an identifier.");

         auto ident = scan_while(is_ident_body);
         if (ident == "_")
            return tok::make_symbol_token(symbols::underscore, location(), source());

         auto kw_val = keywords::to_value(ident);
         return kw_val != keywords::unknown
            ? tok::make_keyword_token(kw_val, location(), source())
            : tok::identifier(ident, location(), source());
      }

      lexical_token scan_string_literal() {
         assert(!eof() && "unexpected end of source.");
         assert(is_quote(peek()) && "expected a leading, double quotation mark, while attempting to scan a string literal.");

         auto prev = peek();
         auto escape = false;
         auto bracket = 0;
         auto bracket_escape = false;

         skip(); // consume quotation mark.
         auto result = scan_while([&](auto const& cp){
            if (bracket > 0) {
               if (cp == '{') ++bracket;
               if (cp == '}') --bracket;
               return true;
            } else if (!bracket_escape && prev == '$' && cp == '{') {
               ++bracket;
               return true;
            } else {
               escape = (prev == '\\' && !escape);
               bracket_escape = (escape && cp == '$');
               if (is_quote(cp) && !escape)
               { prev = cp; return false; }
               prev = cp;
               return true;
            }
         });

         if (!check_if(is_quote))
            return tok::make_error_token("expected closing \'\"\'", location(), source());

         skip(); // consume end quotation mark.
         return tok::string_literal(std::move(result), location(), source());
      }

      lexical_token_suffix scan_floating_literal_suffix() {
         if (!check_if(is_ident_body, 1) && icheck_skip('f'))
            return lexical_token_suffix::float_literal;
         return lexical_token_suffix::none;
      }

      lexical_token_suffix scan_integer_literal_suffix() {
         if (!check_if(is_ident_body, 2) && icheck_skip("ul"))
            return lexical_token_suffix::unsigned_long_literal;

         if (!check_if(is_ident_body, 1)) {
            if (icheck_skip('l')) return lexical_token_suffix::long_literal;
            if (icheck_skip('u')) return lexical_token_suffix::unsigned_literal;
         }
         return lexical_token_suffix::none;
      }

      lexical_token scan_integer_literal(function_ref<bool(codepoint_t)> predicate, int base) {
         auto value = scan_digits(predicate);
         auto suffix = scan_integer_literal_suffix();
         return is_ident_body(peek())
              ? tok::make_error_token("invalid number", skip_while(is_ident_body), source())
              : tok::suffixed_integer_literal(std::stoul(value, 0, base), suffix, location(), source());
      }

      lexical_token scan_numeric_literal() {
         assert(!eof() && "unexpected end of source.");
         assert(check('.') || check_if(is_digit) && "expected a leading digit while attempting to scan an integer literal.");

         if (is_zero_digit(peek())) {
            skip(); // consume zero digit.

            if (icheck('x')) { skip(); return scan_integer_literal(is_hex_digit, 16); }
            if (icheck('b')) { skip(); return scan_integer_literal(is_bin_digit, 2); }
            if (icheck('o')) { skip(); return scan_integer_literal(is_oct_digit, 8); }

            if (check('.') && check_if(is_digit, 1)) {
               skip(); // consume decimal point.
               auto fractional_part = scan_digits();
               auto suffix = scan_floating_literal_suffix();
               return is_ident_body(peek())
                    ? tok::make_error_token("invalid number", skip_while(is_ident_body), source())
                    : tok::suffixed_floating_literal(std::stod("0." + fractional_part), suffix, location(), source());
            }

            auto suffix = scan_integer_literal_suffix();
            return is_ident_body(peek())
                 ? tok::make_error_token("invalid number", skip_while(is_ident_body), source())
                 : tok::suffixed_integer_literal(0, suffix, location(), source());
         }

         auto integer_part = scan_digits();

         if (check('.') && check_if(is_digit, 1)) {
            skip(); // consume decimal point.
            auto fractional_part = scan_digits();
            auto suffix = scan_floating_literal_suffix();
            return is_ident_body(peek())
                 ? tok::make_error_token("invalid number", skip_while(is_ident_body), source())
                 : tok::suffixed_floating_literal(std::stod(integer_part + "." + fractional_part), suffix, location(), source());
         }

         assert(!integer_part.empty() && "expected an integer digit");
         auto suffix = scan_floating_literal_suffix();
         if (suffix != lexical_token_suffix::none) {
            return is_ident_body(peek())
                 ? tok::make_error_token("invalid number", skip_while(is_ident_body), source())
                 : tok::suffixed_floating_literal(std::stod(integer_part), suffix, location(), source());
         }

         suffix = scan_integer_literal_suffix();
         return is_ident_body(peek())
              ? tok::make_error_token("invalid number", skip_while(is_ident_body), source())
              : tok::suffixed_integer_literal(
                   std::stoll(integer_part),
                   suffix, location(), source());
      }

      lexical_token scan_symbol(symbol_token_t symbol) {
         assert(symbol != symbols::unknown && "unknown symbol");

         switch (symbol) {
            case symbols::ampersand: {
               if (check_skip('&', 1)) return tok::double_ampersand(location(), source());
               if (check_skip('=', 1)) return tok::ampersand_equals(location(), source());
            } break;

            case symbols::pipe: {
               if (check_skip('|', 1)) return tok::double_pipe(location(), source());
               if (check_skip('=', 1)) return tok::pipe_equals(location(), source());
            } break;

            case symbols::plus: {
               if (check_skip('+', 1)) return tok::plus_plus(location(), source());
               if (check_skip('=', 1)) return tok::plus_equals(location(), source());
            } break;

            case symbols::minus: {
               if (check_skip('>', 1)) return tok::thin_arrow(location(), source());
               if (check_skip('-', 1)) return tok::minus_minus(location(), source());
               if (check_skip('=', 1)) return tok::minus_equals(location(), source());
            } break;

            case symbols::colon: {
               if (check_skip(':', 1)) return tok::double_colon(location(), source());
               if (check_skip('=', 1)) return tok::colon_equals(location(), source());
            } break;

            case symbols::caret: {
               if (check_skip('=', 1)) return tok::caret_equals(location(), source());
            } break;

            case symbols::asterisk: {
               if (check_skip('=', 1)) return tok::asterisk_equals(location(), source());
            } break;

            case symbols::forward_slash: {
               if (check_skip('=', 1)) return tok::forward_slash_equals(location(), source());
            } break;

            case symbols::percent: {
               if (check_skip('=', 1)) return tok::percent_equals(location(), source());
            } break;

            case symbols::equals: {
               if (check_skip('>', 1)) return tok::thick_arrow(location(), source());
               if (check_skip('=', 1)) return tok::equals_equals(location(), source());
            } break;

            case symbols::exclamation_mark: {
               if (check_skip('=', 1)) return tok::exclamation_mark_equals(location(), source());
            } break;

            case symbols::left_chevron: {
               if (check_skip('=', 1)) return tok::left_chevron_equals(location(), source());
               if (check_skip('<', 1)) {
                  return check_skip('=')
                     ? tok::double_left_chevron_equals(location(), source())
                     : tok::double_left_chevron(location(), source());
               }
            } break;

            case symbols::right_chevron: {
               if (check_skip('=', 1)) return tok::right_chevron_equals(location(), source());
               if (check_skip('>', 1)) {
                  return check_skip('=')
                     ? tok::double_right_chevron_equals(location(), source())
                     : tok::double_right_chevron(location(), source());
               }
            } break;

            case symbols::tilde: {
               if (check_skip("!", 1))
                  return check_skip("=")
                       ? tok::tilde_exclamation_equals(location(), source())
                       : tok::tilde_exclamation(location(), source());
               if (check_skip("&", 1))
                  return check_skip("=")
                       ? tok::tilde_ampersand_equals(location(), source())
                       : tok::tilde_ampersand(location(), source());
               if (check_skip("^", 1))
                  return check_skip("=")
                       ? tok::tilde_caret_equals(location(), source())
                       : tok::tilde_caret(location(), source());
               if (check_skip("|", 1))
                  return check_skip("=")
                       ? tok::tilde_pipe_equals(location(), source())
                       : tok::tilde_pipe(location(), source());
            }

            case symbols::period: {
               if (check("..", 1)) { skip(3); return tok::ellipses(location(), source()); }
               if (check(".^", 1)) { skip(3); return tok::ellipses_caret(location(), source()); }
               if (check_if(is_digit, 1)) return scan_numeric_literal();
            } break;

            default:;
         }

         skip(); // consume symbol.
         return tok::make_symbol_token(symbol, location(), source());
      }


      lexical_token _unwind_indentation() {
         if (_indent.depth() > 0) {
            _loc = rush::location {
               _loc.line() + 1,
               _indent.depth()
            };

            _indent.decrement();
            return tok::dedent(location(), source());
         }
         return tok::eof(location(), source());
      }
   };

   lexical_analysis lex(std::string_view input, lexer_options const& opts) {
      return lex(input, "", opts);
	}

	lexical_analysis lex(std::istream& input, lexer_options const& opts) {
      return lex(input, "", opts);
	}

   lexical_analysis lex(std::string_view input, std::string_view id, lexer_options const& opts) {
      auto l = lexer { opts };
      auto src = source::from_string(input, id);
      return l.tokenize(std::move(src));
	}

	lexical_analysis lex(std::istream& input, std::string_view id, lexer_options const& opts) {
		auto l = lexer { opts };
      auto src = source::from_stream(input, id);
		return l.tokenize(std::move(src));
	}

   lexical_analysis lex(rush::source const& input, lexer_options const& opts) {
      auto l = lexer { opts };
      return l.tokenize(input);
	}

   lexical_analysis lexer::tokenize(rush::source&& src) {
      auto osrc = std::make_unique<rush::source>(std::move(src));
      auto result = this->tokenize(*osrc);
      result.owned(std::move(osrc));
      return std::move(result);
   }

   lexical_analysis lexer::tokenize(rush::source const& src) {
      auto first = src.begin();
      auto last = src.end();
      auto impl = basic_lexer<decltype(first)> { src, _opts };
      return lexical_analysis { impl.tokenize(first, last) };
   }
}
