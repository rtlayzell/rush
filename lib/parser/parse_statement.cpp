#include "rush/ast/statements.hpp"

#include "rush/parser/parser.hpp"

namespace rush {
   std::unique_ptr<ast::statement> parser::parse_stmt() {
      auto tok = peek_skip_indent();
      if (tok.is_keyword()) {
         switch (tok.keyword()) {
            default: return error("'{}' is not a valid statement.", tok);
            case keywords::let_: {
               auto decl = parse_constant_decl();
               if (!decl) return nullptr;
               return ast::stmts::decl_stmt(std::move(decl));
            }
            case keywords::var_: {
               auto decl = parse_variable_decl();
               if (!decl) return nullptr;
               return ast::stmts::decl_stmt(std::move(decl));
            }
            case keywords::if_: return parse_if_stmt();
            case keywords::for_: return parse_for_stmt();
            case keywords::while_: return parse_while_stmt();
            case keywords::return_: return parse_return_stmt();
            case keywords::break_: return parse_break_stmt();
            case keywords::continue_: return parse_continue_stmt();
            case keywords::switch_: return parse_switch_stmt();
            case keywords::throw_: return parse_throw_stmt();
            case keywords::try_: return parse_try_stmt();
            case keywords::yield_: return parse_yield_stmt();
            case keywords::with_: return parse_with_stmt();
            case keywords::base_:
            case keywords::this_: break; // these keywords will form part of a statement expression
         }
      }

      auto expr = parse_expr();
      return (expr != nullptr)
         ? ast::stmts::expr_stmt(std::move(expr))
         : nullptr;
   }

   std::unique_ptr<ast::statement_block> parser::parse_block_stmt() {
      assert(peek_with_indent().is(symbols::indent) && "expected indentation at start of block.");
      next_with_indent(); // consume indentation.

      std::vector<std::unique_ptr<ast::statement>> stmts;
      while (peek_with_indent().is_not(symbols::dedent)) {
         auto stmt = parse_stmt();
         if (!stmt) return nullptr;
         stmts.emplace_back(std::move(stmt));
      }

      next_with_indent(); // consume dedentation.
      return ast::stmts::block(std::move(stmts));
   }

   std::unique_ptr<ast::statement> parser::parse_if_stmt() {
      assert(peek_skip_indent().is(keywords::if_) && "expected 'if' keyword.");
      next_skip_indent(); // consume 'if' keyword.

      auto cond = parse_expr();
      if (!cond) return nullptr;

      if (!peek_skip_indent().is(symbols::colon))
         return error("expected body of 'if' statement before '{}'", next_skip_indent());
      next_skip_indent(); // consume ':' symbol.

      _scope.push(scope_kind::block);
      auto then = (peek_with_indent().is(symbols::indent))
         ? parse_block_stmt()
         : parse_stmt(); // stdc++ does not support move-only initializer-list ast::stmts::block({ parse_stmt() });
      _scope.pop();

      if (!peek_skip_indent().is(keywords::else_)) {
         return ast::stmts::if_(
            std::move(cond),
            std::move(then));
      }

      auto else_ = parse_else_stmt();
      if (!else_) return nullptr;
      return ast::stmts::if_(
         std::move(cond),
         std::move(then),
         std::move(else_));
   }

   std::unique_ptr<ast::statement> parser::parse_else_stmt() {
      assert(peek_skip_indent().is(keywords::else_) && "expected 'else' keyword");
      next_skip_indent(); // consume 'else' keyword

      auto tok = peek_skip_indent();
      if (tok.is_keyword()) {
         return parse_compound_stmt();
      } else if (tok.is(symbols::colon)) {
         next_skip_indent(); // consume ':' symbol.

         _scope.push(scope_kind::block);
         auto else_ = (peek_with_indent().is(symbols::indent))
            ? parse_block_stmt()
            : parse_stmt(); // stdc++ does not support move-only initializer-list ast::stmts::block({ parse_stmt() });
         _scope.pop();

         return std::move(else_);
      }

      return error("expected a compound statement or block statement before '{}'", tok);
   }

   std::unique_ptr<ast::statement> parser::parse_for_stmt() {
      return nullptr;
   }

   std::unique_ptr<ast::statement> parser::parse_while_stmt() {
      return nullptr;
   }

   std::unique_ptr<ast::statement> parser::parse_return_stmt() {
      assert(peek_skip_indent().is(keywords::return_) && "expected 'return' keyword.");
      next_skip_indent(); // consume 'return' keyword.

      // hack: check for possible empty return statements
      // by predicting the possibility that the next token
      // starts an expression.
      auto tok = peek_with_indent();
      if (tok.is(symbols::dedent)) {
         return ast::stmts::return_();
      } else if (tok.is_keyword()) {
         switch (tok.keyword()) {
         default: return ast::stmts::return_();
         case keywords::new_:
         case keywords::nil_:
         case keywords::this_:
         case keywords::base_:
         case keywords::true_:
         case keywords::false_:
         case keywords::sizeof_:
         case keywords::switch_:
         case keywords::typeof_: break;
         }
      }

      auto expr = parse_expr();
      if (!expr) return nullptr;

      if (peek_with_indent().is_not(symbols::dedent))
         return error("return statement is not the last statement in the control block", tok);

      return ast::stmts::return_(std::move(expr));
   }

   std::unique_ptr<ast::statement> parser::parse_break_stmt() {
      assert(peek_skip_indent().is(keywords::break_) && "expected 'break' keyword.");
      next_skip_indent(); // consume 'break' keyword.

      return ast::stmts::break_();
   }

   std::unique_ptr<ast::statement> parser::parse_continue_stmt() {
      assert(peek_skip_indent().is(keywords::continue_) && "expected 'continue' keyword.");
      next_skip_indent(); // consume 'continue' keyword.

      return ast::stmts::continue_();
   }

   std::unique_ptr<ast::statement> parser::parse_switch_stmt() {
      assert(peek_skip_indent().is(keywords::switch_) && "expected 'switch' keyword.");
      next_skip_indent(); // consume 'switch' keyword.

      return nullptr;
   }

   std::unique_ptr<ast::statement> parser::parse_throw_stmt() {
      assert(peek_skip_indent().is(keywords::throw_) && "expected 'throw' keyword.");
      next_skip_indent(); // consume 'throw' keyword.

      return nullptr;
   }

   std::unique_ptr<ast::statement> parser::parse_try_stmt() {
      assert(peek_skip_indent().is(keywords::try_) && "expected 'try' keyword.");
      next_skip_indent(); // consume 'try' keyword.

      return nullptr;
   }

   std::unique_ptr<ast::statement> parser::parse_yield_stmt() {
      assert(peek_skip_indent().is(keywords::yield_) && "expected 'yield' keyword.");
      next_skip_indent(); // consume 'yield' keyword.

      auto expr = parse_expr();
      if (!expr) return nullptr;
      return ast::stmts::yield_(std::move(expr));
   }

   std::unique_ptr<ast::statement> parser::parse_with_stmt() {
      assert(peek_skip_indent().is(keywords::with_) && "expected 'with' keyword.");
      next_skip_indent(); // consume 'with' keyword.

      return nullptr;
   }


   std::unique_ptr<ast::statement> parser::parse_compound_stmt() {
      auto tok = peek_skip_indent();
      switch (tok.keyword()) {
      default: return error("expected compound statement before '{}'", tok);
      case keywords::if_: return parse_if_stmt();
      }
   }
}
