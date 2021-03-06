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

#ifndef RUSH_IRGEN_LLVM_HPP
#define RUSH_IRGEN_LLVM_HPP

#include "rush/parser/result.hpp"
#include "rush/irgen/result.hpp"

#include <memory>

namespace rush::irgen {
   class irgenerator_result {
   public:
      friend irgenerator_result genllvm(rush::syntax_analysis const&);
      ~irgenerator_result();

      void dump();

   private:
      struct impl;
      std::unique_ptr<impl> _pimpl;
      irgenerator_result(std::unique_ptr<impl>);
   };

   irgenerator_result genllvm(rush::syntax_analysis const&);
}

#endif // RUSH_IRGEN_LLVM_HPP
