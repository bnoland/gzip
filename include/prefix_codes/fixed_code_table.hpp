#pragma once

namespace prefix_codes {

class fixed_code_table {
 public:
  fixed_code_table();

  struct entry {
    unsigned int prefix_code;
    unsigned int num_bits;
  };

  // XXX: Make these inline?
  const entry &get_length_literal_entry(unsigned int code) const;
  const entry &get_distance_entry(unsigned int code) const;

 private:
  void init_length_literal_code_table();
  void init_distance_code_table();

  static const unsigned int NUM_LENGTH_LITERAL_CODES {288};
  static const unsigned int NUM_DISTANCE_CODES {32};

  entry length_literal_code_table_[NUM_LENGTH_LITERAL_CODES];
  entry distance_code_table_[NUM_DISTANCE_CODES];
};

}  // namespace prefix_codes
