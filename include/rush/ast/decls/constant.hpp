#pragma once

#ifndef RUSH_AST_DECLS_CONSTANT_HPP
#define RUSH_AST_DECLS_CONSTANT_HPP

#include "rush/ast/decls/storage.hpp"


namespace rush::ast {
	class constant_declaration;

	namespace decls {
		std::unique_ptr<constant_declaration> constant(
			rush::scope& scope,
			std::string name,
			ast::type type,
			std::unique_ptr<expression> init);
	}

	class constant_declaration : public storage_declaration {
		struct factory_tag_t {};

		friend std::unique_ptr<constant_declaration>
			decls::constant(rush::scope&, std::string, ast::type, std::unique_ptr<expression>);

	public:
		constant_declaration(sema::symbol symbol, std::unique_ptr<expression> init, factory_tag_t)
			: storage_declaration { symbol, std::move(init) } {}

		virtual declaration_kind kind() const noexcept override {
			return declaration_kind::constant;
		}

		using node::accept;
		virtual void accept(ast::visitor& v) const override {
			v.visit_constant_decl(*this);
		}
	};

	namespace decls {
		inline std::unique_ptr<constant_declaration> constant(
			rush::scope& scope,
			std::string name,
			ast::type type,
			std::unique_ptr<expression> init
		) {
			if (!init) throw std::invalid_argument("constant declaration requires an intializer.");
			// todo: handle the case where the constant has already been defined.
			// should probably do this outside of the factory functions.. be S.O.L.I.D
			auto symbol = scope.insert(sema::make_constant_entry(std::move(name), type.symbol()));
			return std::make_unique<constant_declaration>(
				std::move(symbol),
				std::move(init),
				constant_declaration::factory_tag_t{});
		}

		inline std::unique_ptr<constant_declaration> constant(
			rush::scope& scope,
			std::string name,
			std::unique_ptr<expression> init
		) {
			return constant(
				scope,
				std::move(name),
				init->result_type(),
				std::move(init));
		}

		inline std::unique_ptr<constant_declaration> constant(
			std::string name,
			ast::type type,
			std::unique_ptr<expression> init
		) {
			return constant(
				rush::global_scope,
				std::move(name),
				std::move(type),
				std::move(init));
		}

		inline std::unique_ptr<constant_declaration> constant(
			std::string name,
			std::unique_ptr<expression> init
		) {
			return constant(
				std::move(name),
				init->result_type(),
				std::move(init));
		}
	} // rush::ast::decls
} // rush::ast

#endif // RUSH_AST_DECLS_CONSTANT_HPP
