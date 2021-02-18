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

#ifndef RUSH_LEXER_LEXER_HPP
#define RUSH_LEXER_LEXER_HPP

#include "rush/lexer/lex.hpp"
#include "rush/lexer/result.hpp"

#include <string_view>
#include <iosfwd>

namespace rush {
   class source;

	template <typename InIter>
	class lexer_iterator;

	class lexer {
		friend class lexical_iterator;

		lexer(lexer const&) = delete;
		void operator = (lexer const&) = delete;

	public:
		explicit lexer(lexer_options opts = {}) noexcept
         : _opts { std::move(opts) } {}

		lexer(lexer&& other) = default;
		lexer& operator = (lexer&& other) = default;

      lexical_analysis tokenize(rush::source&&);
		lexical_analysis tokenize(rush::source const&);

   private:
      lexer_options _opts;
	};
};

#endif // RUSH_LEXER_LEXER_HPP
