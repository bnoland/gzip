#include "gzip/gzip_writer.hpp"
#include "lzss/lzss_symbol.hpp"
#include "prefix_codes/prefix_code_encoder.hpp"

#include <cassert>
#include <string>
#include <string_view>
#include <unordered_map>

// XXX: Remove later. Next time use a debugger, you degenerate.
#include <iostream>

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
    write_block_type_2(input_buffer, is_last_block);

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

  // XXX: Should outputting the codes be managed by the fixed code table class?
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

    auto extra_bits {symbol.get_extra_bits()};
    if (extra_bits > 0) {
      auto offset {symbol.get_offset()};
      bit_writer_.put_bits(offset, extra_bits);
    }
  }
}

void gzip_writer::write_block_type_2(std::string_view input_buffer,
                                     bool is_last_block) {
  const unsigned int BLOCK_TYPE {2};
  bit_writer_.put_single_bit(is_last_block);
  bit_writer_.put_bits(BLOCK_TYPE, 2);

  lzss_encoder_.encode(input_buffer);

  auto symbol_list {lzss_encoder_.get_symbol_list()};
  symbol_list.add(lzss::END_OF_BLOCK_MARKER);

  std::vector<unsigned int> length_literal_codes {};
  std::vector<unsigned int> distance_codes {};
  for (auto symbol : symbol_list) {
    using enum lzss::lzss_symbol_type;

    if (symbol.get_type() == DISTANCE) {
      distance_codes.push_back(symbol.get_code());
    } else {
      length_literal_codes.push_back(symbol.get_code());
    }
  }

  std::unordered_map<unsigned int, unsigned int> length_literal_freqs {};
  for (auto code : length_literal_codes) {
    if (!length_literal_freqs.contains(code)) {
      length_literal_freqs.insert({code, 0});
    }
    length_literal_freqs.at(code)++;
  }

  std::unordered_map<unsigned int, unsigned int> distance_freqs {};
  for (auto code : distance_codes) {
    if (!distance_freqs.contains(code)) {
      distance_freqs.insert({code, 0});
    }
    distance_freqs.at(code)++;
  }

  // XXX: Do we need to handle the case when no LL symbols?
  prefix_codes::prefix_code_encoder length_literal_encoder {15};
  length_literal_encoder.encode(length_literal_freqs);

  // XXX: Separately handle case when no distance symbols.
  prefix_codes::prefix_code_encoder distance_encoder {15};
  distance_encoder.encode(distance_freqs);

  auto length_literal_lengths {length_literal_encoder.get_code_length_table()};
  auto distance_lengths {distance_encoder.get_code_length_table()};

  std::vector<unsigned int> code_length_buffer {};

  const unsigned int NUM_LENGTH_LITERAL_CODES {286};
  for (unsigned int code {0}; code < NUM_LENGTH_LITERAL_CODES; code++) {
    if (length_literal_lengths.contains(code)) {
      code_length_buffer.push_back(length_literal_lengths.at(code));
    } else {
      code_length_buffer.push_back(0);
    }
  }

  const unsigned int NUM_DISTANCE_CODES {30};
  for (unsigned int code {0}; code < NUM_DISTANCE_CODES; code++) {
    if (distance_lengths.contains(code)) {
      code_length_buffer.push_back(distance_lengths.at(code));
    } else {
      code_length_buffer.push_back(0);
    }
  }

  // XXX: Compute HLIT + HDIST.
  // XXX: Run specialized RLE on values in `code_lengths`.

  std::unordered_map<unsigned int, unsigned int> cl_freqs {};
  for (auto code : code_length_buffer) {
    if (!cl_freqs.contains(code)) {
      cl_freqs.insert({code, 0});
    }
    cl_freqs.at(code)++;
  }

  prefix_codes::prefix_code_encoder cl_encoder {7};
  cl_encoder.encode(cl_freqs);

  auto cl_lengths {cl_encoder.get_code_length_table()};

  // for (auto [code, length] : cl_lengths) {
  //   std::cout << code << ": " << length << std::endl;
  // }

  const unsigned int CL_CODE_LENGTH_ORDER[] {16, 17, 18, 0, 8,  7, 9,  6, 10, 5,
                                             11, 4,  12, 3, 13, 2, 14, 1, 15};

  std::vector<unsigned int> cl_code_length_buffer {};

  for (auto code : CL_CODE_LENGTH_ORDER) {
    if (cl_lengths.contains(code)) {
      cl_code_length_buffer.push_back(cl_lengths.at(code));
    } else {
      cl_code_length_buffer.push_back(0);
    }
  }

  // for (auto length : cl_code_length_buffer) {
  //   std::cout << length << std::endl;
  // }

  // XXX: Compute HCLEN.

  const unsigned int NUM_CL_CODES {19};

  // XXX: HLIT, HDIST, and HCLEN will be computed dynamically in the future.
  bit_writer_.put_bits(NUM_LENGTH_LITERAL_CODES - 257, 5);
  bit_writer_.put_bits(NUM_DISTANCE_CODES - 1, 5);
  bit_writer_.put_bits(NUM_CL_CODES - 4, 5);

  // XXX: This is outputting the wrong stuff according to gzstat...
  for (auto length : cl_code_length_buffer) {
    bit_writer_.put_bits(length, 3);
  }

  auto cl_code_table {cl_encoder.get_code_length_table()};

  for (auto length : code_length_buffer) {
    bit_writer_.put_bits(cl_code_table.at(length), 7, false);
  }

  auto length_literal_code_table {length_literal_encoder.get_code_table()};
  auto distance_code_table {distance_encoder.get_code_table()};

  for (auto symbol : symbol_list) {
    using enum lzss::lzss_symbol_type;

    if (symbol.get_type() == DISTANCE) {
      auto code {distance_code_table.at(symbol.get_code())};
      auto length {distance_lengths.at(symbol.get_code())};
      bit_writer_.put_bits(code, length, false);
    } else {
      auto code {length_literal_code_table.at(symbol.get_code())};
      auto length {length_literal_lengths.at(symbol.get_code())};
      bit_writer_.put_bits(code, length, false);
    }
  }
}

}  // namespace gzip
