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
#include "rush/parser/string_parser.hpp"
#include "rush/ast/patterns.hpp"
#include "rush/ast/expressions.hpp"
#include "rush/ast/declarations.hpp"
#include "rush/diag/syntax_error.hpp"

namespace ast = rush::ast;
namespace errs = rush::diag::errs;
namespace exprs = rush::ast::exprs;
namespace ptrns = rush::ast::ptrns;

namespace rush {

#define RUSH_IS_UNARY_PREFIX_OP_FUNC
#define RUSH_IS_UNARY_POSTFIX_OP_FUNC
#define RUSH_IS_BINARY_OP_FUNC
#define RUSH_PRECEDENCE_FUNC
#define RUSH_ASSOCIATIVITY_FUNC
#include "rush/ast/exprs/_operators.hpp"

	int compare_operator_precedence(lexical_token const& lhs, lexical_token const& rhs) {
		return is_binary_operator(lhs) && is_binary_operator(rhs)
           ? binary_precedence(lhs) - binary_precedence(rhs) : 0;
	}

	rush::parse_result<ast::expression> parser::parse_expr() {
		auto result = parse_primary_expr();
		while (result.success() && is_binary_operator(peek_skip_indent()))
			result = parse_binary_expr(std::move(result));

		return std::move(result);
	}

   rush::parse_result<ast::expression> parser::parse_paren_expr() {
      assert(peek_skip_indent().is(symbols::left_parenthesis) && "expected token to be \'(\'");
      if (peek_skip_indent(1).is(symbols::underscore)) {
         return parse_lambda_expr();
      } else if (peek_skip_indent(1).is_identifier()) {
         auto next = peek_skip_indent(2);
         if (next.is_symbol()) {
            switch (next.symbol()) {
            default: break;
            case symbols::comma:
            case symbols::colon:
            case symbols::equals:
               return parse_complex_paren_expr();
            case symbols::right_parenthesis:
               if (peek_skip_indent(3).is_any(symbols::thick_arrow, symbols::colon_equals, symbols::thin_arrow))
                  return parse_lambda_expr();
            }
         }
      }

      return parse_simple_paren_expr();
   }

   rush::parse_result<ast::expression> parser::parse_simple_paren_expr() {
      assert(peek_skip_indent().is(symbols::left_parenthesis) && "expected token to be \'(\'");
      if (peek_skip_indent(1).is(symbols::right_parenthesis)
       || peek_skip_indent(1).is(symbols::left_brace))
         return parse_lambda_expr();

      next_skip_indent(); // consume '('
      auto result = parse_expr();
      if (result.failed()) return std::move(result);

      auto next = peek_skip_indent();
      if (next.is(symbols::comma)) {
         return parse_tuple_literal_expr(std::move(result));
      } else {
         if (next.is_not(symbols::right_parenthesis))
            return errs::expected_closing_parenthesis(next);
         next_skip_indent(); // consume ')'
      }

      return exprs::parens(std::move(result));
   }

   rush::parse_result<ast::expression> parser::parse_complex_paren_expr() {
      assert(peek_skip_indent().is(symbols::left_parenthesis) && "expected token to be \'(\'");

      auto ident = peek_skip_indent(1);
      auto next = peek_skip_indent(2);

      // named tuple or equals expression edge-case.
      if (next.is(symbols::equals)) {
         next_skip_indent(); // consume '('
         next_skip_indent(); // consume ident.
         next_skip_indent(); // consume '='
         auto expr_result = parse_expr();

         if (expr_result.failed())
            return std::move(expr_result);

         auto tok = peek_skip_indent();
         if (!tok.is_symbol())
            return errs::expected_closing_parenthesis_or_tuple(tok);

         switch (tok.symbol()) {
         default: return errs::expected_closing_parenthesis_or_tuple(tok);
         case symbols::comma: // tuple expression
            return parse_tuple_literal_expr(ptrns::binding(
               ptrns::name(ident.text()),
               std::move(expr_result)));
         case symbols::right_parenthesis: // assignment expression
            next_skip_indent(); // consume ')'
            return exprs::parens(
               exprs::assignment(
                  exprs::identifier(ident.text()),
                  std::move(expr_result)));
         }
      }

      return is_lambda_expr_ahead()
         ? parse_lambda_expr()
         : parse_tuple_literal_expr();
   }

   rush::parse_result<ast::expression> parser::parse_array_literal_expr() {
      assert(peek_skip_indent().is(symbols::left_bracket) && "expected left square bracket to start array expression.");
      next_skip_indent(); // consume '[' symbol

      if (consume_skip_indent(symbols::right_bracket))
         return exprs::array(exprs::list());

      auto expr_list_result = parse_expr_list();
      if (expr_list_result.failed())
         return std::move(expr_list_result).as<ast::expression>();

      // consume possible trailing comma.
      consume_skip_indent(symbols::comma);

      return consume_skip_indent(symbols::right_bracket)
           ? rush::parse_result<ast::expression> { exprs::array(std::move(expr_list_result)) }
           : errs::expected_closing_square_bracket_or_comma(peek_skip_indent());
   }

   rush::parse_result<ast::expression> parser::parse_tuple_literal_expr() {
      assert(peek_skip_indent().is(symbols::left_parenthesis) && "expected token to be \'(\'");
      next_skip_indent(); // consume '('

      auto first = parse_tuple_element_expr();
      if (first.failed()) return std::move(first);

      return parse_tuple_literal_expr(std::move(first));
   }

   rush::parse_result<ast::expression> parser::parse_tuple_literal_expr(rush::parse_result<ast::expression> result) {
      assert(peek_skip_indent().is(symbols::comma) && "expected comma to start tuple expression.");
      auto elist = parse_tuple_element_list(std::move(result));
      return elist.failed()
         ? std::move(elist).as<ast::expression>()
         : exprs::tuple(std::move(elist));
   }

   rush::parse_result<ast::expression> parser::parse_tuple_element_expr() {
      if (peek_skip_indent().is_identifier()
      && peek_skip_indent(1).is_any(symbols::colon, symbols::equals)) {

         auto ident = next_skip_indent();
         std::unique_ptr<ast::pattern> ptrn = ptrns::name(ident.text());

         if (consume_skip_indent(symbols::colon)) {
            auto type_result = parse_type();
            if (type_result.failed())
               return std::move(type_result).errors();

            ptrn = ptrns::annotation(
               std::move(ptrn),
               type_result.type());
         }

         if (!consume_skip_indent(symbols::equals))
            return errs::expected(peek_skip_indent(), "=");

         auto expr_result = parse_expr();
         if (expr_result.failed())
            return std::move(expr_result);

         return ptrns::binding(
            std::move(ptrn),
            std::move(expr_result));
      } else {
         auto expr_result = parse_expr();
         return std::move(expr_result);
      }
   }

   rush::parse_result<ast::expression_list> parser::parse_tuple_element_list(rush::parse_result<ast::expression> first) {
      if (peek_skip_indent().is(symbols::comma)) {
         next_skip_indent(); // consume ','

         std::vector<std::unique_ptr<ast::expression>> results;
         results.push_back(std::move(first));

         do {
            auto elem_result = parse_tuple_element_expr();
            if (elem_result.failed())
               return std::move(elem_result).as<ast::expression_list>();

            results.push_back(std::move(elem_result));
         } while (consume_skip_indent(symbols::comma));

         return consume_skip_indent(symbols::right_parenthesis)
            ? rush::parse_result<ast::expression_list> { exprs::list(std::move(results)) }
            : errs::expected_closing_parenthesis(peek_skip_indent());
      } else {
         if (!consume_skip_indent(symbols::right_parenthesis))
            return errs::expected_closing_parenthesis(peek_skip_indent());

         std::vector<std::unique_ptr<ast::expression>> args;
         args.push_back(std::move(first));
         return exprs::list(std::move(args));
      }
   }

   rush::parse_result<ast::expression> parser::parse_lambda_expr() {
      auto params = std::unique_ptr<ast::pattern> {};
      if (peek_skip_indent().is_identifier()) {
         auto result = parse_named_pattern();
         if (result.failed()) return std::move(result).as<ast::expression>();
         params = std::move(result);
      } else if (peek_skip_indent().is(symbols::underscore)) {
         auto result = parse_discard_pattern();
         if (result.failed()) return std::move(result).as<ast::expression>();
         params = std::move(result);
      } else if (peek_skip_indent().is(symbols::left_bracket)) {
         auto result = parse_array_destructure_pattern();
         if (result.failed()) return std::move(result).as<ast::expression>();
         params = std::move(result);
      } else if (peek_skip_indent().is(symbols::left_brace)) {
         auto result = parse_object_destructure_pattern();
         if (result.failed()) return std::move(result).as<ast::expression>();
         params = std::move(result);
      } else {
         auto result = parse_parameter_list();
         if (result.failed()) return std::move(result).as<ast::expression>();
         params = std::move(result);
      }

      auto return_type = rush::parse_type_result {};
      if (consume_skip_indent(symbols::thin_arrow)) {
         return_type = parse_type();
         if (return_type.failed())
            return std::move(return_type).errors();
      }

      if (!peek_skip_indent().is_any(symbols::thick_arrow, symbols::colon_equals))
         return errs::expected_function_expr_body(peek_skip_indent());

      auto body_result = parse_function_expr_body();
      if (body_result.failed())
         return std::move(body_result).as<ast::expression>();

      return return_type.failed()
           ? exprs::lambda(std::move(params), std::move(body_result))
           : exprs::lambda(return_type.type(), std::move(params), std::move(body_result));
   }

	rush::parse_result<ast::expression> parser::parse_primary_expr() {
		auto tok = peek_skip_indent();

      rush::parse_result<ast::expression> result;
		switch (tok.type()) {
		default: return errs::expected_primary_expr(tok);
		case lexical_token_type::string_literal: result = parse_string_expr(); break;
		case lexical_token_type::integer_literal: result = parse_integer_expr(); break;
		case lexical_token_type::floating_literal: result = parse_floating_expr(); break;
		case lexical_token_type::identifier:
         result = peek_skip_indent(1).is(symbols::thick_arrow)
            ? parse_lambda_expr()
            : parse_identifier_expr();
         break;
		case lexical_token_type::keyword:
			switch (tok.keyword()) {
			default: return errs::unexpected_keyword_expr(tok);
         case keywords::nil_: next_skip_indent(); result = exprs::nil(); break;
			case keywords::true_: next_skip_indent(); result = exprs::literal(true, *_context); break;
			case keywords::false_: next_skip_indent(); result = exprs::literal(false, *_context); break;
         case keywords::this_: next_skip_indent(); result = exprs::this_(); break;
         case keywords::base_: next_skip_indent(); result = exprs::base_(); break;
         case keywords::new_: result = parse_new_expr(); break;
			} break;
		case lexical_token_type::symbol:
         if (is_unary_prefix_operator(tok)) result = parse_unary_expr();
         else switch (tok.symbol()) {
         case symbols::underscore:
            result = peek_skip_indent(1).is(symbols::thick_arrow)
               ? parse_lambda_expr()
               : parse_discard_pattern();
            break;
         case symbols::ellipses: result = parse_spread_expr(); break;
         case symbols::left_parenthesis: result = parse_paren_expr(); break;
         case symbols::left_brace:
            result = is_lambda_expr_ahead()
                   ? parse_lambda_expr()
                   : errs::unexpected_symbol_expr(tok);
            break;
         case symbols::left_bracket:
            result = is_lambda_expr_ahead()
                   ? parse_lambda_expr()
                   : parse_array_literal_expr();
            break;
         default: return errs::unexpected_symbol_expr(tok);
         } break;
		}

      while (result.success() && is_unary_postfix_operator(peek_skip_indent()))
         result = parse_unary_postfix_expr(std::move(result));

      while (result.success() && peek_skip_indent().is(symbols::period))
         result = parse_member_access_expr(std::move(result));

      while (result.success() && peek_skip_indent().is(symbols::left_bracket))
         result = parse_subscript_expr(std::move(result));

      return std::move(result);
	}

	rush::parse_result<ast::expression> parser::parse_string_expr() {
		assert(peek_skip_indent().is_string_literal() && "expected token to be a string literal.");
		auto tok = next_skip_indent();
      auto sp = rush::string_parser { *this };
      return sp.parse(tok);
	}

	rush::parse_result<ast::expression> parser::parse_integer_expr() {
		assert(peek_skip_indent().is_integer_literal() && "expected token to be an integer literal.");
		auto tok = next_skip_indent();
		auto val = tok.integer_value();
		switch (tok.suffix()) {
		case lexical_token_suffix::none: return exprs::literal(static_cast<int>(val), *_context);
		case lexical_token_suffix::long_literal: return exprs::literal(static_cast<long>(val), *_context);
		case lexical_token_suffix::unsigned_literal: return exprs::literal(static_cast<unsigned>(val), *_context);
		case lexical_token_suffix::unsigned_long_literal: return exprs::literal(static_cast<unsigned long>(val), *_context);
		default: throw;
		}
	}

	rush::parse_result<ast::expression> parser::parse_floating_expr() {
		assert(peek_skip_indent().is_floating_literal() && "expected token to be a floating literal.");
		auto tok = next_skip_indent();
		auto val = tok.floating_value();
		switch (tok.suffix()) {
		case lexical_token_suffix::none: return exprs::literal(static_cast<double>(val), *_context);
		case lexical_token_suffix::float_literal: return exprs::literal(static_cast<float>(val), *_context);
		default: throw;
		}
	}

	rush::parse_result<ast::expression> parser::parse_identifier_expr() {
		assert(peek_skip_indent().is_identifier() && "expected token to be an identifier.");
		auto tok = next_skip_indent();
      return exprs::identifier(tok.text());
	}

   rush::parse_result<ast::expression> parser::parse_subscript_expr(rush::parse_result<ast::expression> operand_expr) {
      assert(peek_skip_indent().is(symbols::left_bracket) && "expected token to be a '['.");
      next_skip_indent(); // consume '['.

      auto subscript_expr = parse_expr();
      if (subscript_expr.failed())
         return std::move(subscript_expr).as<ast::expression>();

      auto expr_result = exprs::subscript(
         std::move(operand_expr),
         std::move(subscript_expr));

      return consume_skip_indent(symbols::right_bracket)
           ? rush::parse_result<ast::expression> { std::move(expr_result) }
           : errs::expected_closing_square_bracket_or_comma(peek_skip_indent());
   }

   rush::parse_result<ast::expression> parser::parse_member_access_expr(rush::parse_result<ast::expression> expr) {
      assert(peek_skip_indent().is(symbols::period) && "expected token to be a '.'.");
      next_skip_indent(); // consume period.

      auto ident_result = parse_primary_expr();
      auto ma_expr = exprs::member_access(
         std::move(expr),
         std::move(ident_result));

      return std::move(ma_expr);
	}

   rush::parse_result<ast::expression> parser::parse_unary_expr() {
      assert(is_unary_prefix_operator(peek_skip_indent()) && "expected unary operator.");

      auto tok = next_skip_indent(); // consume unary operator token.
      auto operand_result = parse_primary_expr();
      if (operand_result.failed())
         return std::move(operand_result);

      auto next = peek_skip_indent();
      if (is_unary_postfix_operator(next)) {
         operand_result = parse_unary_postfix_expr(std::move(operand_result));
         if (operand_result.failed()) return std::move(operand_result);
      } else if (is_binary_operator(next) && unary_precedence(tok) > binary_precedence(next)) {
         operand_result = parse_binary_expr(std::move(operand_result));
         if (operand_result.failed()) return std::move(operand_result);
      }

      switch (tok.symbol()) {
      default: return errs::not_supported(tok, fmt::format("unary operator '{}'", tok.text()));
      case symbols::plus: return exprs::positive(std::move(operand_result));
      case symbols::minus: return exprs::negative(std::move(operand_result));
      case symbols::plus_plus: return exprs::pre_increment(std::move(operand_result));
      case symbols::minus_minus: return exprs::pre_decrement(std::move(operand_result));
      case symbols::exclamation_mark: return exprs::logical_negation(std::move(operand_result));
      case symbols::tilde_exclamation: return exprs::bitwise_negation(std::move(operand_result));
      }
   }

   rush::parse_result<ast::expression> parser::parse_unary_postfix_expr(rush::parse_result<ast::expression> operand_result) {
      assert(is_unary_postfix_operator(peek_skip_indent()) && "expected unary postfix operator.");

      auto tok = peek_skip_indent();
      switch (tok.symbol()) {
      default: return errs::not_supported(tok, fmt::format("unary postfix operator '{}'", tok.text()));

      case symbols::plus_plus:
         operand_result = exprs::post_increment(std::move(operand_result));
         next_skip_indent();
         break;

      case symbols::minus_minus:
         operand_result = exprs::post_decrement(std::move(operand_result));
         next_skip_indent();
         break;

      case symbols::left_parenthesis:
         operand_result = parse_invoke_expr(std::move(operand_result));
         if (operand_result.failed()) return std::move(operand_result);
         break;
      }

      return is_unary_postfix_operator(peek_skip_indent())
         ? parse_unary_postfix_expr(std::move(operand_result))
         : std::move(operand_result);
   }

	rush::parse_result<ast::expression> parser::parse_binary_expr(rush::parse_result<ast::expression> lhs) {
		assert(is_binary_operator(peek_skip_indent()) && "expected binary operator.");

		auto tok = peek_skip_indent();
      if (tok.is(symbols::question_mark))
         return parse_ternary_expr(std::move(lhs));

		auto rhs = parse_binary_expr_rhs();

      if (lhs.failed() || rhs.failed()) {
         lhs = std::move(rhs);
         return std::move(lhs);
      }

		rush::parse_result<ast::expression> result;

      if (tok.is_symbol()) {
		switch (tok.symbol()) {
      default: return errs::not_supported(tok, fmt::format("binary operator '{}'", tok.text()));
      case symbols::equals: result = exprs::assignment(std::move(lhs), std::move(rhs)); break;
      case symbols::colon_equals: result = exprs::assignment(std::move(lhs), std::move(rhs)); break;
      // arithmetic binary operators
		case symbols::plus: result = exprs::addition(std::move(lhs), std::move(rhs)); break;
		case symbols::minus: result = exprs::subtraction(std::move(lhs), std::move(rhs)); break;
		case symbols::percent: result = exprs::modulo(std::move(lhs), std::move(rhs)); break;
		case symbols::asterisk: result = exprs::multiplication(std::move(lhs), std::move(rhs)); break;
		case symbols::caret: result = exprs::exponentiation(std::move(lhs), std::move(rhs)); break;
		case symbols::forward_slash: result = exprs::division(std::move(lhs), std::move(rhs)); break;

      // compound arithmetic binary operators
      case symbols::plus_equals: result = exprs::compound_addition(std::move(lhs), std::move(rhs)); break;
      case symbols::minus_equals: result = exprs::compound_subtraction(std::move(lhs), std::move(rhs)); break;
      case symbols::percent_equals: result = exprs::compound_modulo(std::move(lhs), std::move(rhs)); break;
      case symbols::asterisk_equals: result = exprs::compound_multiplication(std::move(lhs), std::move(rhs)); break;
      case symbols::forward_slash_equals: result = exprs::compound_division(std::move(lhs), std::move(rhs)); break;

      // bit-wise binary operators
      case symbols::tilde_pipe: result = exprs::bitwise_or(std::move(lhs), std::move(rhs)); break;
      case symbols::tilde_caret: result = exprs::bitwise_xor(std::move(lhs), std::move(rhs)); break;
      case symbols::tilde_ampersand: result = exprs::bitwise_and(std::move(lhs), std::move(rhs)); break;
      case symbols::double_left_chevron: result = exprs::left_shift(std::move(lhs), std::move(rhs)); break;
      case symbols::double_right_chevron: result = exprs::right_shift(std::move(lhs), std::move(rhs)); break;

      // compound bit-wise binary operators
      case symbols::tilde_pipe_equals: result = exprs::compound_bitwise_or(std::move(lhs), std::move(rhs)); break;
      case symbols::tilde_caret_equals: result = exprs::compound_bitwise_xor(std::move(lhs), std::move(rhs)); break;
      case symbols::tilde_ampersand_equals: result = exprs::compound_bitwise_and(std::move(lhs), std::move(rhs)); break;
      case symbols::double_left_chevron_equals: result = exprs::compound_left_shift(std::move(lhs), std::move(rhs)); break;
      case symbols::double_right_chevron_equals: result = exprs::compound_right_shift(std::move(lhs), std::move(rhs)); break;

      // special operators
      case symbols::ellipses: result = exprs::inclusive_range(std::move(lhs), std::move(rhs)); break;
      case symbols::ellipses_caret: result = exprs::exclusive_range(std::move(lhs), std::move(rhs)); break;

      // logical binary operators
		case symbols::double_pipe: result = exprs::logical_or(std::move(lhs), std::move(rhs)); break;
		case symbols::double_ampersand: result = exprs::logical_and(std::move(lhs), std::move(rhs)); break;
      case symbols::equals_equals: result = exprs::equal(std::move(lhs), std::move(rhs)); break;
      case symbols::exclamation_mark_equals: result = exprs::not_equal(std::move(lhs), std::move(rhs)); break;
		case symbols::left_chevron: result = exprs::less_than(std::move(lhs), std::move(rhs)); break;
		case symbols::right_chevron: result = exprs::greater_than(std::move(lhs), std::move(rhs)); break;
		case symbols::left_chevron_equals: result = exprs::less_equals(std::move(lhs), std::move(rhs)); break;
		case symbols::right_chevron_equals: result = exprs::greater_equals(std::move(lhs), std::move(rhs)); break;
		}
      } else {
         switch (tok.keyword()) {
         default: return errs::not_supported(tok, fmt::format("binary operator '{}'", tok.text()));
         // case keywords::is_: result = exprs::is(std::move(lhs), std::move(rhs)); break;
         // case keywords::as_: result = exprs::as(std::move(lhs), std::move(rhs)); break;
         }
      }

      return std::move(result);
	}

   rush::parse_result<ast::expression> parser::parse_binary_expr_rhs() {
		assert(is_binary_operator(peek_skip_indent()) && "expected binary operator.");

		auto prev = next_skip_indent(); // consume binary operator symbol.

      if (prev.is(symbols::ellipses)) {
         consume_skip_indent(symbols::left_chevron);
      }

      auto rhs_result = parse_primary_expr();
		if (rhs_result.success()) {
         auto next = peek_skip_indent(); // look-ahead for possible binary operator.

         while (is_binary_operator(next)) {
            auto opcmp = compare_operator_precedence(next, prev);
            if (opcmp < 0 || (opcmp == 0 && binary_associativity(next) > 0)) {
               rhs_result = parse_binary_expr(std::move(rhs_result));
               next = peek_skip_indent();
            } else break;
         }
      }

      return std::move(rhs_result);
	}

   rush::parse_result<ast::expression> parser::parse_ternary_expr(rush::parse_result<ast::expression> cond_result) {
      assert(peek_skip_indent().is(symbols::question_mark) && "expected ternary expression sequence.");
      next_skip_indent(); // consume '?' symbol.

      auto true_result = parse_expr();
      if (true_result.failed()) return std::move(true_result);

      if (peek_skip_indent().is_not(symbols::colon))
         return errs::expected(peek_skip_indent(), ":");
      next_skip_indent(); // consume ':' symbol.

      auto false_result = parse_expr();
      if (false_result.failed()) return std::move(false_result);

      return exprs::ternary(
         std::move(cond_result),
         std::move(true_result),
         std::move(false_result));
   }

   rush::parse_result<ast::expression> parser::parse_argument() {
      if (peek_skip_indent().is_identifier()
      && peek_skip_indent(1).is(symbols::colon)) {
         auto ident = next_skip_indent();
         next_skip_indent(); // skip ':';

         auto expr_result = parse_expr();
         if (expr_result.failed())
            return std::move(expr_result);

         return ptrns::binding(
            ptrns::name(ident.text()),
            std::move(expr_result));
      } else {
         auto expr_result = parse_expr();
         return std::move(expr_result);
      }
   }

   rush::parse_result<ast::expression_list> parser::parse_expr_list() {
      std::vector<std::unique_ptr<ast::expression>> results;
      do {
         auto result = parse_expr();
         if (result.failed())
            return std::move(result)
               .as<ast::expression_list>();

         results.push_back(std::move(result));
      } while (
         consume_skip_indent(symbols::comma) &&
         peek_skip_indent().is_not_any(
            symbols::right_parenthesis,
            symbols::right_bracket,
            symbols::right_brace));

      return exprs::list(std::move(results));
   }

   rush::parse_result<ast::expression_list> parser::parse_argument_list() {
      assert(peek_skip_indent().is(symbols::left_parenthesis) && "expected '(' start of argument list.");
		next_skip_indent(); // consume '(' symbol

      // try consume ')' symbol
      if (consume_skip_indent(symbols::right_parenthesis))
         return exprs::list();

      auto arg_result = parse_argument();
      if (arg_result.failed())
         return std::move(arg_result).as<ast::expression_list>();

      if (peek_skip_indent().is(symbols::comma))
         return parse_argument_list(std::move(arg_result));

      if (!consume_skip_indent(symbols::right_parenthesis))
         return errs::expected_closing_parenthesis(peek_skip_indent());

      std::vector<std::unique_ptr<ast::expression>> args;
      args.push_back(std::move(arg_result));
      return exprs::list(std::move(args));
   }

   rush::parse_result<ast::expression_list> parser::parse_argument_list(rush::parse_result<ast::expression> first) {
      assert(peek_skip_indent().is(symbols::comma) && "expected comma to start argument list.");
      next_skip_indent(); // consume ','

      std::vector<std::unique_ptr<ast::expression>> results;
      results.push_back(std::move(first));

      do {
         auto arg_result = parse_argument();
         if (arg_result.failed())
            return std::move(arg_result).as<ast::expression_list>();

         results.push_back(std::move(arg_result));
      } while (consume_skip_indent(symbols::comma));

      return consume_skip_indent(symbols::right_parenthesis)
           ? rush::parse_result<ast::expression_list> { exprs::list(std::move(results)) }
           : errs::expected_closing_parenthesis(peek_skip_indent());
   }

   rush::parse_result<ast::expression> parser::parse_invoke_expr(rush::parse_result<ast::expression> expr_result) {
      assert(peek_skip_indent().is(symbols::left_parenthesis) && "expected ternary expression sequence.");

      auto result = parse_argument_list();
      if (result.failed())
         return std::move(result).as<ast::expression>();

		return exprs::invoke(
         std::move(expr_result),
         std::move(result));
   }

   rush::parse_result<ast::expression> parser::parse_new_expr() {
      assert(peek_skip_indent().is(keywords::new_) && "expected 'new' keyword.");
      next_skip_indent(); // consume 'new' keyword.

      if (!peek_skip_indent().is_identifier())
         return errs::expected(peek_skip_indent(), "type");

      auto ident = parse_identifier_expr();
      if (ident.failed()) return std::move(ident);

      if (!peek_skip_indent().is(symbols::left_parenthesis))
         return errs::expected(peek_skip_indent(), "(");

      auto expr = parse_invoke_expr(std::move(ident));
      if (expr.failed()) return std::move(expr);

      return exprs::new_(std::move(expr));
   }

   rush::parse_result<ast::expression> parser::parse_spread_expr() {
      assert(consume_skip_indent(symbols::ellipses) && "expected '...' symbol.");

      auto expr_result = parse_primary_expr();
      if (expr_result.failed())
         return std::move(expr_result);

      return exprs::spread(std::move(expr_result));
   }
}
