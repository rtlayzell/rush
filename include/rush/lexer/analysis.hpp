#pragma once

#ifndef RUSH_LEXER_ANALYSIS_HPP
#define RUSH_LEXER_ANALYSIS_HPP

#include "rush/lexer/options.hpp"

#include <string>
#include <iosfwd>
#include <vector>


namespace rush {

	namespace detail {
		class lexical_analysis_iterator {

		};
	}

	class lexical_analysis final {
		friend lexical_analysis lex(char const*, lexer_options const&);
		friend lexical_analysis lex(std::string const&, lexer_options const&);
		friend lexical_analysis lex(std::istream&, lexer_options const&);

	public:
		lexical_analysis(lexical_analysis const&) = delete;
		void operator = (lexical_analysis const&) = delete;

		lexical_analysis(lexical_analysis&&);
		void operator = (lexical_analysis&&);

		using iterator = typename std::vector<lexical_token>::iterator;
		using const_iterator = typename std::vector<lexical_token>::const_iterator;

		bool empty() const noexcept {
			return _tokens.empty();
		}

		std::size_t size() const noexcept {
			return _tokens.size();
		}

		const_iterator begin() const noexcept {
			return _tokens.begin();
		}

		const_iterator end() const noexcept {
			return _tokens.end();
		}

		const_iterator cbegin() const noexcept {
			return _tokens.cbegin();
		}

		const_iterator cend() const noexcept {
			return _tokens.cend();
		}

	private:
		lexical_analysis(std::vector<lexical_token> toks)
			: _tokens(std::move(toks)) {}

		std::vector<lexical_token> _tokens;
	};
} // rush

#endif // RUSH_LEXER_ANALYSIS_HPP