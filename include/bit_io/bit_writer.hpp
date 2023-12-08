#pragma once

#include <cstdint>
#include <ostream>

namespace bit_io {

class BitWriter {
 public:
  BitWriter(std::ostream& output);

  void put_single_bit(bool value);
  void put_bits(uint64_t value, int num_bits, bool low_bit_first = true);
  void pad_to_byte();
  void finish();

 private:
  std::ostream& output_;
  int current_bit_ {0};
  uint8_t current_byte_ {0};
};

}  // namespace bit_io
