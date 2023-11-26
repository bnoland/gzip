#pragma once

#include <string>
#include <vector>
#include <cassert>

namespace lzss {

enum class lzss_symbol_type { LITERAL, LENGTH, DISTANCE };

class lzss_symbol {
 public:
  lzss_symbol(lzss_symbol_type type, unsigned int value);

  auto get_type() const { return type_; }
  auto get_value() const { return value_; }

  auto get_code() const { return code_; }
  auto get_extra_bits() const {
    assert(type_ != lzss_symbol_type::LITERAL &&
           "literals do not have extra bits");
    return extra_bits_;
  }
  auto get_offset() const {
    assert(type_ != lzss_symbol_type::LITERAL &&
           "literals do not have offsets");
    return offset_;
  }

 private:
  void lookup_code_table_data();

  const lzss_symbol_type type_;
  const unsigned int value_;

  unsigned int code_;
  unsigned int extra_bits_;
  unsigned int offset_;
};

extern const lzss_symbol END_OF_BLOCK_MARKER;

// XXX: Just use std::list<lzss_symbol> instead. Provide a function that returns
// a string representation of such a list.
class lzss_symbol_list {
 public:
  void add(const lzss_symbol& symbol);
  void clear();

  std::string to_string() const;

  auto begin() const { return list_.begin(); }
  auto end() const { return list_.end(); }

 private:
  std::vector<lzss_symbol> list_ {};
};

}  // namespace lzss
