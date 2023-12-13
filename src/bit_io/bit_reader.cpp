#include "bit_io/bit_reader.hpp"

#include <cassert>
#include <cstdint>
#include <istream>

namespace bit_io {

BitReader::BitReader(std::istream& input) : input_ {input} {}

bool BitReader::get_single_bit() {
  if (current_bit_ == 0) {
    current_byte_ = input_.get();
  }

  bool value = current_byte_ & (1 << current_bit_);

  ++current_bit_;
  if (current_bit_ == 8) {
    current_bit_ = 0;
  }

  return value;
}

uint64_t BitReader::get_bits(int num_bits, bool low_bit_first) {
  assert(num_bits > 0 && num_bits <= 64);

  uint64_t value = 0;

  if (low_bit_first) {
    for (int i = 0; i < num_bits; i++) {
      value |= (static_cast<uint64_t>(get_single_bit()) << i);
    }
  } else {
    for (int i = num_bits - 1; i >= 0; i--) {
      value |= (static_cast<uint64_t>(get_single_bit()) << i);
    }
  }

  return value;
}

void BitReader::align_to_byte() {
  while (current_bit_ != 0) {
    get_single_bit();
  }
}

bool BitReader::eof() const {
  return input_.eof();
}

}  // namespace bit_io
