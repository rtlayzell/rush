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

#ifndef CRUSH_COMMANDS_DUMP_HPP
#define CRUSH_COMMANDS_DUMP_HPP

#include "rush/core/source.hpp"
#include "rush/lexer/lex.hpp"
#include "rush/parser/parse.hpp"
#include "rush/irgen/llvm.hpp"
#include "rush/lexer/dump.hpp"
#include "rush/parser/dump.hpp"
#include "rush/ast/context.hpp"
#include "cxxopts/cxxopts.hpp"

#include <iostream>
#include <vector>

namespace crush::commands {

   void dump_llvm_ir(std::vector<rush::source>& srcs, cxxopts::ParseResult& opts) {
      auto ctx = rush::ast::context {};
      auto ast = rush::parse(srcs.begin(), srcs.end(), ctx);
      auto ir = rush::irgen::genllvm(ast);
      ir.dump();
   }

   void dump_lex(std::vector<rush::source>& srcs, cxxopts::ParseResult& opts) {
      auto lxa = rush::lex(srcs.begin(), srcs.end());
      rush::dump(lxa);
      std::cout << std::endl;
   }

   void dump_parse(std::vector<rush::source>& srcs, cxxopts::ParseResult& opts) {
      auto ctx = rush::ast::context {};
      auto ast = rush::parse(srcs.begin(), srcs.end(), ctx);
      rush::dump(ast);
      std::cout << std::endl;
   }
}

#endif // CRUSH_COMMANDS_DUMP_HPP
