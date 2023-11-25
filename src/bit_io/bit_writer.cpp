#include "bit_io/bit_writer.hpp"

#include <cassert>
#include <cstdint>
#include <ostream>

namespace bit_io {

bit_writer::bit_writer(std::ostream& output) : output_ {output} {}

void bit_writer::put_single_bit(bool value) {
  current_byte_ |= (static_cast<uint8_t>(value) << current_bit_);

  ++current_bit_;
  if (current_bit_ == 8) {
    output_.put(current_byte_);
    current_bit_ = 0;
    current_byte_ = 0;
  }
}

void bit_writer::put_bits(uint64_t value, int num_bits, bool low_bit_first) {
  assert(num_bits > 0 && num_bits <= 64);

  if (low_bit_first) {
    for (int i = 0; i < num_bits; i++) {
      put_single_bit(value & (1 << i));
    }
  } else {
    for (int i = num_bits - 1; i >= 0; i--) {
      put_single_bit(value & (1 << i));
    }
  }
}

void bit_writer::pad_to_byte() {
  while (current_bit_ != 0) {
    put_single_bit(false);
  }
}

void bit_writer::finish() {
  pad_to_byte();
}

}  // namespace bit_io
