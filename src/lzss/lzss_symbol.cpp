#include "lzss/lzss_symbol.hpp"
#include "lzss/lzss_code_tables.hpp"
#include "lzss/lzss_constants.hpp"

#include <cassert>
#include <string>

namespace lzss {

const LzssSymbol END_OF_BLOCK_MARKER {LzssSymbolType::LITERAL, 256};

LzssSymbol::LzssSymbol(LzssSymbolType type, unsigned int value) : type_ {type}, value_ {value} {
  assert(!(type == LzssSymbolType::LITERAL && value > 256) && "trying to create invalid literal");

  assert(!(type == LzssSymbolType::LENGTH &&
           (value < constants::MIN_BACKREF_LENGTH || value > constants::MAX_BACKREF_LENGTH)) &&
         "trying to create invalid length");

  assert(!(type == LzssSymbolType::DISTANCE && value > constants::MAX_BACKREF_DISTANCE) &&
         "trying to create invalid distance");

  lookup_code_table_data();
}

void LzssSymbol::lookup_code_table_data() {
  switch (type_) {
    using enum LzssSymbolType;

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

void LzssSymbolList::add(const LzssSymbol& symbol) {
  list_.push_back(symbol);
}

void LzssSymbolList::clear() {
  list_.clear();
}

std::string LzssSymbolList::to_string() const {
  std::string result {};

  for (const auto& symbol : list_) {
    switch (symbol.get_type()) {
      using enum LzssSymbolType;

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
