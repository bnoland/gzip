#pragma once

#include <cassert>

namespace prefix_codes {

// XXX: Does this even need to be a class?
class fixed_code_table {
 public:
  fixed_code_table();

  struct entry {
    unsigned int prefix_code;
    unsigned int num_bits;
  };

  const auto& get_length_literal_entry(unsigned int code) const {
    assert(code < NUM_LENGTH_LITERAL_CODES && "searching for invalid length/literal code in fixed code table");
    return length_literal_code_table_[code];
  }
  const auto& get_distance_entry(unsigned int code) const {
    assert(code < NUM_DISTANCE_CODES && "searching for invalid distance code in fixed code table");
    return distance_code_table_[code];
  }

 private:
  void init_length_literal_code_table();
  void init_distance_code_table();

  static const unsigned int NUM_LENGTH_LITERAL_CODES {288};
  static const unsigned int NUM_DISTANCE_CODES {32};

  entry length_literal_code_table_[NUM_LENGTH_LITERAL_CODES];
  entry distance_code_table_[NUM_DISTANCE_CODES];
};

}  // namespace prefix_codes
