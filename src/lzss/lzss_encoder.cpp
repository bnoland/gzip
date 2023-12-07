#include "lzss/lzss_encoder.hpp"
#include "lzss/lzss_constants.hpp"
#include "lzss/lzss_string_table.hpp"
#include "lzss/lzss_symbol.hpp"

#include <string_view>

namespace lzss {

void lzss_encoder::encode(std::string_view input_buffer) {
  symbol_list_.clear();

  unsigned int input_pos {0};

  while (input_pos < input_buffer.length()) {
    std::string string {input_buffer.substr(input_pos, constants::MAX_BACKREF_LENGTH)};
    auto back_ref {string_table_.get_back_reference(string)};

    string_table_.insert(string, current_position_);

    if (!back_ref.has_value()) {
      output_literal(input_buffer[input_pos]);
      input_pos++;
      continue;
    }

    auto [position, length] {back_ref.value()};
    auto distance {current_position_ - position};

    if (distance > constants::MAX_BACKREF_DISTANCE) {
      output_literal(input_buffer[input_pos]);
      input_pos++;
      continue;
    }

    output_back_reference(length, distance);
    input_pos += length;
  }
}

void lzss_encoder::output_back_reference(unsigned int length, unsigned int distance) {
  symbol_list_.add({lzss_symbol_type::LENGTH, length});
  symbol_list_.add({lzss_symbol_type::DISTANCE, distance});
  current_position_ += length;
}

void lzss_encoder::output_literal(unsigned int value) {
  // XXX: Nasty static cast...
  symbol_list_.add({lzss_symbol_type::LITERAL, static_cast<unsigned char>(value)});
  current_position_++;
}

}  // namespace lzss
