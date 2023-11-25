#pragma once

#include "lzss/lzss_string_table.hpp"
#include "lzss/lzss_symbol.hpp"

#include <string_view>

namespace lzss {

class lzss_encoder {
 public:
  // XXX: Just return the symbol list immediately from encode().
  void encode(std::string_view input_buffer);

  // XXX: These can be inline.
  const lzss_symbol_list& get_symbol_list() const;
  const lzss_string_table& get_string_table() const;

 private:
  void output_back_reference(unsigned int length, unsigned int distance);
  void output_literal(unsigned int value);

  unsigned int current_position_ {0};
  lzss_string_table string_table_ {};
  lzss_symbol_list symbol_list_ {};
};

}  // namespace lzss
