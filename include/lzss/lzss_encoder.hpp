#pragma once

#include "lzss/lzss_string_table.hpp"
#include "lzss/lzss_symbol.hpp"

#include <string_view>

namespace lzss {

class lzss_encoder {
 public:
  void encode(std::string_view input_buffer);

  const auto& get_symbol_list() const { return symbol_list_; }
  const auto& get_string_table() const { return string_table_; }

 private:
  void output_back_reference(unsigned int length, unsigned int distance);
  void output_literal(unsigned int value);

  unsigned int current_position_ {0};
  lzss_string_table string_table_ {};
  lzss_symbol_list symbol_list_ {};
};

}  // namespace lzss
