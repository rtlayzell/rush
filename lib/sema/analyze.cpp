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
#include "rush/sema/analyze.hpp"
#include "rush/sema/engine.hpp"
#include "rush/parser/parse.hpp"

namespace rush {
   semantic_analysis analyze(std::string src) {
      ast::context ctx {};
      return analyze(rush::parse(std::move(src), ctx));
   }

   semantic_analysis analyze(rush::syntax_analysis const& syn) {
      auto eng = sema::engine {};
      auto& p = eng.analyze(syn).ast();

      return eng.analyze(syn);
   }
}
