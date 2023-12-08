#pragma once

namespace lzss::code_tables {

struct Entry {
  unsigned int code;
  unsigned int extra_bits;
  unsigned int lower_bound;
  unsigned int upper_bound;
};

const Entry& get_length_entry(unsigned int length);

const Entry& get_distance_entry(unsigned int distance);

}  // namespace lzss::code_tables
