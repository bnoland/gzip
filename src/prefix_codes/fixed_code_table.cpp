#include "prefix_codes/fixed_code_table.hpp"

namespace prefix_codes {

FixedCodeTable::FixedCodeTable() {
  init_length_literal_code_table();
  init_distance_code_table();
}

void FixedCodeTable::init_length_literal_code_table() {
  for (unsigned int code {0}; code <= 143; code++) {
    length_literal_code_table_[code] = {0b00110000 + code, 8};
  }

  for (unsigned int code {144}; code <= 255; code++) {
    length_literal_code_table_[code] = {0b110010000 + (code - 144), 9};
  }

  for (unsigned int code {256}; code <= 279; code++) {
    length_literal_code_table_[code] = {code - 256, 7};
  }

  for (unsigned int code {280}; code <= 287; code++) {
    length_literal_code_table_[code] = {0b11000000 + (code - 280), 8};
  }
}

void FixedCodeTable::init_distance_code_table() {
  for (unsigned int code {0}; code <= 31; code++) {
    distance_code_table_[code] = {code, 5};
  }
}

}  // namespace prefix_codes
