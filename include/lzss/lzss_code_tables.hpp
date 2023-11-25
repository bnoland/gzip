#pragma once

namespace lzss::code_tables {

struct entry {
  unsigned int code;
  unsigned int extra_bits;
  unsigned int lower_bound;
  unsigned int upper_bound;
};

const entry& get_length_entry(unsigned int length);

const entry& get_distance_entry(unsigned int distance);

}  // namespace lzss::code_tables
