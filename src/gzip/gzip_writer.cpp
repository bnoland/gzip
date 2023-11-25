#include "gzip/gzip_writer.hpp"
#include "lzss/lzss_symbol.hpp"

#include <cassert>
#include <string>
#include <string_view>

namespace gzip {

gzip_writer::gzip_writer(std::istream& input, std::ostream& output)
    : input_ {input}, output_ {output}, bit_writer_ {output} {}

void gzip_writer::write() {
  write_header();
  write_deflate_bit_stream();
  write_footer();
}

void gzip_writer::write_header() {
  const unsigned int MAGIC1 {0x1f};
  const unsigned int MAGIC2 {0x8b};
  const unsigned int COMPRESSION_METHOD {0x08};
  const unsigned int FLAGS {0};
  // XXX: Set this to the current time.
  const unsigned int MODIFICATION_TIME {0};
  const unsigned int EXTRA_FLAGS {0};
  const unsigned int OPERATING_SYSTEM {0x03};

  bit_writer_.put_bits(MAGIC1, 8);
  bit_writer_.put_bits(MAGIC2, 8);
  bit_writer_.put_bits(COMPRESSION_METHOD, 8);
  bit_writer_.put_bits(FLAGS, 8);
  bit_writer_.put_bits(MODIFICATION_TIME, 32);
  bit_writer_.put_bits(EXTRA_FLAGS, 8);
  bit_writer_.put_bits(OPERATING_SYSTEM, 8);
}

void gzip_writer::write_deflate_bit_stream() {
  while (true) {
    auto input_buffer {read_input_chunk()};
    input_size_ += input_buffer.length();
    bool is_last_block {input_.eof()};

    // XXX: Strategically choose block type.
    write_block_type_1(input_buffer, is_last_block);

    if (is_last_block) {
      break;
    }
  }
}

void gzip_writer::write_footer() {
  bit_writer_.pad_to_byte();
  bit_writer_.put_bits(0, 32);  // XXX: Write CRC32.
  bit_writer_.put_bits(input_size_, 32);
  bit_writer_.finish();
}

std::string gzip_writer::read_input_chunk() {
  std::string input_buffer {};
  char byte;

  for (unsigned int i {0}; i < INPUT_CHUNK_SIZE; i++) {
    if (!input_.get(byte)) {
      break;
    }
    input_buffer += byte;
  }

  return input_buffer;
}

void gzip_writer::write_block_type_0(std::string_view input_buffer,
                                     bool is_last_block) {
  const unsigned int MAX_BLOCK_LENGTH {65535};
  const unsigned int BLOCK_TYPE {0};

  assert(input_buffer.length() <= MAX_BLOCK_LENGTH);

  bit_writer_.put_single_bit(is_last_block);
  bit_writer_.put_bits(BLOCK_TYPE, 2);
  bit_writer_.pad_to_byte();

  bit_writer_.put_bits(input_buffer.length(), 16);
  bit_writer_.put_bits(~input_buffer.length(), 16);

  for (auto byte : input_buffer) {
    bit_writer_.put_bits(byte, 8);
  }
}

void gzip_writer::write_block_type_1(std::string_view input_buffer,
                                     bool is_last_block) {
  const unsigned int BLOCK_TYPE {1};

  bit_writer_.put_single_bit(is_last_block);
  bit_writer_.put_bits(BLOCK_TYPE, 2);

  lzss_encoder_.encode(input_buffer);

  auto symbol_list {lzss_encoder_.get_symbol_list()};
  symbol_list.add(lzss::END_OF_BLOCK_MARKER);

  for (const auto& symbol : symbol_list) {
    using enum lzss::lzss_symbol_type;

    switch (symbol.get_type()) {
      case LITERAL:
      case LENGTH: {
        auto [prefix_code, num_bits] {
          fixed_code_table_.get_length_literal_entry(symbol.get_code())};
        bit_writer_.put_bits(prefix_code, num_bits, false);
        break;
      }
      case DISTANCE: {
        auto [prefix_code, num_bits] {
          fixed_code_table_.get_distance_entry(symbol.get_code())};
        bit_writer_.put_bits(prefix_code, num_bits, false);
        break;
      }
    }

    if (symbol.get_type() == LITERAL) {
      continue;
    }

    auto extraBits {symbol.get_extra_bits()};
    if (extraBits > 0) {
      auto offset {symbol.get_offset()};
      bit_writer_.put_bits(offset, extraBits);
    }
  }
}

void gzip_writer::write_block_type_2(std::string_view input_buffer,
                                     bool is_last_block) {}

}  // namespace gzip
