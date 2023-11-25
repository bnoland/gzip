#include "prefix_codes/fixed_code_table.hpp"

#include <cassert>

namespace prefix_codes {

fixed_code_table::fixed_code_table() {
  init_length_literal_code_table();
  init_distance_code_table();
}

const fixed_code_table::entry& fixed_code_table::get_length_literal_entry(
    unsigned int code) const {
  assert(code < NUM_LENGTH_LITERAL_CODES &&
         "searching for invalid length/literal code in fixed code table");

  return length_literal_code_table_[code];
}

const fixed_code_table::entry& fixed_code_table::get_distance_entry(
    unsigned int code) const {
  assert(code < NUM_DISTANCE_CODES &&
         "searching for invalid distance code in fixed code table");

  return distance_code_table_[code];
}

void fixed_code_table::init_length_literal_code_table() {
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

void fixed_code_table::init_distance_code_table() {
  for (unsigned int code {0}; code <= 31; code++) {
    distance_code_table_[code] = {code, 5};
  }
}

}  // namespace prefix_codes
