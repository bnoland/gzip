#pragma once

namespace lzss::code_tables {

struct Entry {
  unsigned int code;
  unsigned int extra_bits;
  unsigned int lower_bound;
  unsigned int upper_bound;
};

const Entry& get_length_entry_by_code(unsigned int code);
const Entry& get_distance_entry_by_code(unsigned int code);
const Entry& get_length_entry_by_length(unsigned int length);
const Entry& get_distance_entry_by_distance(unsigned int distance);

}  // namespace lzss::code_tables
