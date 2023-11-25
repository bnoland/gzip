#pragma once

#include <cstdint>
#include <istream>

namespace bit_io {

class bit_reader {
 public:
  bit_reader(std::istream& input);

  bool get_single_bit();
  uint64_t get_bits(int num_bits, bool low_bit_first = true);
  bool eof() const;

 private:
  std::istream& input_;
  int current_bit_ {0};
  uint8_t current_byte_ {0};
};

}  // namespace bit_io
