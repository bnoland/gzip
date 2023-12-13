#include "gzip/gzip_reader.hpp"

#include <cassert>

namespace gzip {

GzipReader::GzipReader(std::istream& input, std::ostream& output)
    : input_ {input}, output_ {output}, bit_reader_ {input} {}

void GzipReader::read() {
  read_header();
  read_deflate_bit_stream();
  read_footer();
}

void GzipReader::read_header() {
  const unsigned int MAGIC1 {0x1f};
  const unsigned int MAGIC2 {0x8b};
  const unsigned int COMPRESSION_METHOD {0x08};

  if (bit_reader_.get_bits(8) != MAGIC1 || bit_reader_.get_bits(8) != MAGIC2) {
    throw GzipReaderError("Invalid gzip file.");
  }
  if (bit_reader_.get_bits(8) != COMPRESSION_METHOD) {
    throw GzipReaderError("Invalid compression method.");
  }

  // Read the rest of the header. We currently don't do anything with this data.
  bit_reader_.get_bits(8);
  bit_reader_.get_bits(32);
  bit_reader_.get_bits(8);
  bit_reader_.get_bits(8);
}

void GzipReader::read_deflate_bit_stream() {
  while (true) {
    bool is_last_block {bit_reader_.get_single_bit()};
    auto block_type {bit_reader_.get_bits(2)};
    if (bit_reader_.eof()) {
      break;
    }

    // XXX: Read block of appropriate type based on value of `block_type`.

    // XXX: Sanity check. Remove later.
    assert(block_type == 0);

    read_block_type_0();

    if (is_last_block) {
      break;
    }
  }
}

void GzipReader::read_footer() {}

void GzipReader::read_block_type_0() {
  bit_reader_.align_to_byte();

  auto input_size {bit_reader_.get_bits(16)};
  if (bit_reader_.get_bits(16) != (~input_size & 0xFFFF)) {
    throw GzipReaderError("Invalid block type 0 header.");
  }

  for (unsigned int i {0}; i < input_size; i++) {
    output_.put(bit_reader_.get_bits(8));
  }
}

}  // namespace gzip
