#pragma once

#include <array>
#include <cassert>

namespace prefix_codes {

class FixedCodeTable {
 public:
  FixedCodeTable();

  struct Entry {
    unsigned int prefix_code;
    unsigned int num_bits;
  };

  const auto& get_length_literal_entry(unsigned int code) const {
    assert(code < length_literal_code_table_.size() && "searching for invalid length/literal code in fixed code table");
    return length_literal_code_table_[code];
  }
  const auto& get_distance_entry(unsigned int code) const {
    assert(code < distance_code_table_.size() && "searching for invalid distance code in fixed code table");
    return distance_code_table_[code];
  }

 private:
  void init_length_literal_code_table();
  void init_distance_code_table();

  std::array<Entry, 288> length_literal_code_table_;
  std::array<Entry, 32> distance_code_table_;
};

}  // namespace prefix_codes
