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

#ifndef RUSH_LEXER_ANALYSIS_HPP
#define RUSH_LEXER_ANALYSIS_HPP

#include "rush/lexer/token.hpp"
#include "rush/core/source.hpp"

#include <string>
#include <string_view>
#include <iterator>
#include <iosfwd>
#include <vector>


namespace rush {

	class lexer;
	class lexer_options;

	class lexical_analysis final {
		friend class lexer;

	public:
		lexical_analysis(lexical_analysis const&) = delete;
		void operator = (lexical_analysis const&) = delete;

		lexical_analysis(lexical_analysis&&) = default;
		lexical_analysis& operator = (lexical_analysis&&) = default;

      using reference = typename std::vector<lexical_token>::reference;
      using const_reference = typename std::vector<lexical_token>::const_reference;

		using iterator = typename std::vector<lexical_token>::iterator;
		using const_iterator = typename std::vector<lexical_token>::const_iterator;

      void append(lexical_analysis&& other) {
         std::move(
            other._tokens.begin(),
            other._tokens.end(),
            std::back_inserter(_tokens));
      }

		bool empty() const noexcept {
			return _tokens.empty();
		}

      rush::source const& source() const noexcept {
         return _tokens.front().source();
      }

		std::size_t size() const noexcept {
			return _tokens.size();
		}

      reference front() noexcept {
         return _tokens.front();
      }

      const_reference front() const noexcept {
         return _tokens.front();
      }

      reference back() noexcept {
         return _tokens.back();
      }

      const_reference back() const noexcept {
         return _tokens.back();
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
		std::vector<lexical_token> _tokens;
      std::unique_ptr<rush::source> _osrc;

      explicit lexical_analysis(
         std::vector<lexical_token> toks)
         : _tokens(std::move(toks)) {}

      void owned(std::unique_ptr<rush::source> src) {
         _osrc = std::move(src);
      }
	};
} // rush

#endif // RUSH_LEXER_ANALYSIS_HPP
