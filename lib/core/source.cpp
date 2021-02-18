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
#include "rush/core/source.hpp"

#include "llvm/Support/MemoryBuffer.h"

#include <variant>
#include <iostream>
#include <iterator>

namespace rush {
   struct source::impl {
   public:
      llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> buffer;

      impl() : buffer { llvm::MemoryBuffer::getMemBuffer("") } {}
      impl(llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> buf)
         : buffer { std::move(buf) } {}
   };

   source const source::none = source();

   source source::from_file(std::filesystem::path path, bool is_volatile) {
      return { std::make_unique<source::impl>(llvm::MemoryBuffer::getFile(
         llvm::StringRef { path.string() },
         -1L, true, is_volatile)) };
   }

   source source::from_string(std::string_view input, std::string_view id) {
      return { std::make_unique<source::impl>(llvm::MemoryBuffer::getMemBuffer(
         llvm::StringRef { input.begin(), input.size() },
         llvm::StringRef { id.begin(), id.size() })) };
   }

   source source::from_stream(std::istream& input, std::string_view id) {
      auto eof = std::istreambuf_iterator<char> {};
      return source::from_string(std::string {
         std::istreambuf_iterator<char>(input), eof }, id);
   }


   source::~source() = default;
   source::source(source&& other) = default;
   source& source::operator = (source&& other) = default;

   source::source()
      : source(std::make_unique<source::impl>()) {}

   source::source(std::unique_ptr<source::impl> pimpl)
      : _pimpl { std::move(pimpl) } {}

   std::error_code source::error() const {
      return _pimpl->buffer.getError();
   }

   std::string_view source::id() const {
      if (_pimpl->buffer) {
         auto ident = (*_pimpl->buffer)->getBufferIdentifier();
         return { ident.begin(), ident.size() };
      } else { return "<no-id>"; }
   }

   std::string_view source::buffer() const {
      if (_pimpl->buffer) {
         auto buf = (*_pimpl->buffer)->getBuffer();
         return { buf.begin(), buf.size() };
      } else { return ""; }
   }

   std::size_t source::size() const {
      return (*_pimpl->buffer)->getBufferSize();
   }

   std::size_t source::length() const {
      return (*_pimpl->buffer)->getBufferSize();
   }

   std::string_view::const_iterator source::begin() const {
      return (*_pimpl->buffer)->getBufferStart();
   }

   std::string_view::const_iterator source::end() const {
      return (*_pimpl->buffer)->getBufferEnd();
   }
}
