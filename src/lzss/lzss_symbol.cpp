#include "lzss/lzss_symbol.hpp"
#include "lzss/lzss_code_tables.hpp"
#include "lzss/lzss_constants.hpp"

#include <cassert>
#include <string>

namespace lzss {

const lzss_symbol END_OF_BLOCK_MARKER {lzss_symbol_type::LITERAL, 256};

lzss_symbol::lzss_symbol(lzss_symbol_type type, unsigned int value)
    : type_ {type}, value_ {value} {
  assert(!(type == lzss_symbol_type::LITERAL && value > 256) &&
         "trying to create invalid literal");

  assert(!(type == lzss_symbol_type::LENGTH &&
           (value < constants::MIN_BACKREF_LENGTH ||
            value > constants::MAX_BACKREF_LENGTH)) &&
         "trying to create invalid length");

  assert(!(type == lzss_symbol_type::DISTANCE &&
           value > constants::MAX_BACKREF_DISTANCE) &&
         "trying to create invalid distance");

  lookup_code_table_data();
}

void lzss_symbol::lookup_code_table_data() {
  switch (type_) {
    using enum lzss_symbol_type;

    case LITERAL:
      code_ = value_;
      break;
    case LENGTH: {
      auto entry {code_tables::get_length_entry(value_)};
      code_ = entry.code;
      extra_bits_ = entry.extra_bits;
      offset_ = value_ - entry.lower_bound;
      break;
    }
    case DISTANCE: {
      auto entry {code_tables::get_distance_entry(value_)};
      code_ = entry.code;
      extra_bits_ = entry.extra_bits;
      offset_ = value_ - entry.lower_bound;
      break;
    }
  }
}

lzss_symbol_type lzss_symbol::get_type() const {
  return type_;
}

unsigned int lzss_symbol::get_value() const {
  return value_;
}

unsigned int lzss_symbol::get_code() const {
  return code_;
}

unsigned int lzss_symbol::get_extra_bits() const {
  assert(type_ != lzss_symbol_type::LITERAL &&
         "literals do not have extra bits");
  return extra_bits_;
}

unsigned int lzss_symbol::get_offset() const {
  assert(type_ != lzss_symbol_type::LITERAL && "literals do not have offsets");
  return offset_;
}

void lzss_symbol_list::add(const lzss_symbol& symbol) {
  list_.push_back(symbol);
}

void lzss_symbol_list::clear() {
  list_.clear();
}

std::string lzss_symbol_list::to_string() const {
  std::string result {};

  for (const auto& symbol : list_) {
    switch (symbol.get_type()) {
      using enum lzss_symbol_type;

      case LITERAL:
        result += symbol.get_value();
        break;
      case LENGTH:
        result += "<";
        result += std::to_string(symbol.get_value());
        result += ":";
        break;
      case DISTANCE:
        result += std::to_string(symbol.get_value());
        result += ">";
        break;
    }
  }

  return result;
}

}  // namespace lzss
