#include "gzip/gzip_writer.hpp"
#include "lzss/lzss_symbol.hpp"
#include "prefix_codes/prefix_code_encoder.hpp"

#include <array>
#include <cassert>
#include <string>
#include <string_view>
#include <unordered_map>

namespace gzip {

GzipWriter::GzipWriter(std::istream& input, std::ostream& output)
    : input_ {input}, output_ {output}, bit_writer_ {output} {}

void GzipWriter::write() {
  write_header();
  write_deflate_bit_stream();
  write_footer();
}

void GzipWriter::write_header() {
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

void GzipWriter::write_deflate_bit_stream() {
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

void GzipWriter::write_footer() {
  bit_writer_.pad_to_byte();
  bit_writer_.put_bits(0, 32);  // XXX: Write CRC32.
  bit_writer_.put_bits(input_size_, 32);
  bit_writer_.finish();
}

std::string GzipWriter::read_input_chunk() {
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

void GzipWriter::write_block_type_0(std::string_view input_buffer, bool is_last_block) {
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

void GzipWriter::write_block_type_1(std::string_view input_buffer, bool is_last_block) {
  const unsigned int BLOCK_TYPE {1};
  bit_writer_.put_single_bit(is_last_block);
  bit_writer_.put_bits(BLOCK_TYPE, 2);

  lzss_encoder_.encode(input_buffer);

  auto symbol_list {lzss_encoder_.get_symbol_list()};
  symbol_list.add(lzss::END_OF_BLOCK_MARKER);

  for (const auto& symbol : symbol_list) {
    using enum lzss::LzssSymbolType;

    switch (symbol.get_type()) {
      case LITERAL:
      case LENGTH: {
        auto [prefix_code, num_bits] {fixed_code_table_.get_length_literal_entry(symbol.get_code())};
        bit_writer_.put_bits(prefix_code, num_bits, false);
        break;
      }
      case DISTANCE: {
        auto [prefix_code, num_bits] {fixed_code_table_.get_distance_entry(symbol.get_code())};
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

void GzipWriter::write_block_type_2(std::string_view input_buffer, bool is_last_block) {
  const unsigned int BLOCK_TYPE {2};
  bit_writer_.put_single_bit(is_last_block);
  bit_writer_.put_bits(BLOCK_TYPE, 2);

  lzss_encoder_.encode(input_buffer);

  auto symbol_list {lzss_encoder_.get_symbol_list()};
  symbol_list.add(lzss::END_OF_BLOCK_MARKER);

  // Divide the input symbols into length/literal symbols and distance symbols.

  std::vector<unsigned int> input_ll_codes {};
  std::vector<unsigned int> input_distance_codes {};
  for (auto symbol : symbol_list) {
    using enum lzss::LzssSymbolType;

    if (symbol.get_type() == DISTANCE) {
      input_distance_codes.push_back(symbol.get_code());
    } else {
      input_ll_codes.push_back(symbol.get_code());
    }
  }

  // Compute the frequency of each symbol in the input.

  std::unordered_map<unsigned int, unsigned int> input_ll_freqs {};
  for (auto code : input_ll_codes) {
    if (!input_ll_freqs.contains(code)) {
      input_ll_freqs.insert({code, 0});
    }
    input_ll_freqs.at(code)++;
  }

  std::unordered_map<unsigned int, unsigned int> input_distance_freqs {};
  for (auto code : input_distance_codes) {
    if (!input_distance_freqs.contains(code)) {
      input_distance_freqs.insert({code, 0});
    }
    input_distance_freqs.at(code)++;
  }

  // Compute separate prefix codes for length/literal symbols and distance symbols.

  const unsigned int MAX_LL_DISTANCE_CODE_LENGTH {15};
  prefix_codes::PrefixCodeEncoder ll_encoder {MAX_LL_DISTANCE_CODE_LENGTH};
  ll_encoder.encode(input_ll_freqs);
  // XXX: Handle case when no distance symbols present.
  prefix_codes::PrefixCodeEncoder distance_encoder {MAX_LL_DISTANCE_CODE_LENGTH};
  distance_encoder.encode(input_distance_freqs);

  // Put all the length/literal and distance code lengths into a contiguous buffer.

  auto ll_code_lengths {ll_encoder.get_code_length_table()};
  auto distance_code_lengths {distance_encoder.get_code_length_table()};

  std::vector<unsigned int> ll_distance_code_length_buffer {};

  const unsigned int MAX_LL_CODE {285};
  unsigned int num_ll_codes {0};

  for (int code {MAX_LL_CODE}; code >= 0; code--) {
    if (ll_code_lengths.contains(code)) {
      num_ll_codes = code + 1;
      break;
    }
  }

  for (unsigned int code {0}; code < num_ll_codes; code++) {
    if (ll_code_lengths.contains(code)) {
      ll_distance_code_length_buffer.push_back(ll_code_lengths.at(code));
    } else {
      ll_distance_code_length_buffer.push_back(0);
    }
  }

  const unsigned int MAX_DISTANCE_CODE {29};
  unsigned int num_distance_codes {0};

  for (int code {MAX_DISTANCE_CODE}; code >= 0; code--) {
    if (distance_code_lengths.contains(code)) {
      num_distance_codes = code + 1;
      break;
    }
  }

  for (unsigned int code {0}; code < num_distance_codes; code++) {
    if (distance_code_lengths.contains(code)) {
      ll_distance_code_length_buffer.push_back(distance_code_lengths.at(code));
    } else {
      ll_distance_code_length_buffer.push_back(0);
    }
  }

  // Run-length encode the length/literal and distance length buffer.

  struct ClSymbol {
    unsigned int code;
    unsigned int repeat_count {0};
  };

  std::vector<ClSymbol> rle_output {};

  unsigned int repeat_count {0};
  unsigned int i;

  for (i = 0; i < ll_distance_code_length_buffer.size(); i++) {
    if (i == 0) {
      rle_output.push_back({ll_distance_code_length_buffer[i]});
      continue;
    }

    if (ll_distance_code_length_buffer[i - 1] == ll_distance_code_length_buffer[i]) {
      repeat_count++;
      if (ll_distance_code_length_buffer[i] == 0) {
        if (repeat_count == 138) {
          rle_output.push_back({18, repeat_count});
          repeat_count = 0;
        }
      } else {
        if (repeat_count == 6) {
          rle_output.push_back({16, repeat_count});
          repeat_count = 0;
        }
      }
      continue;
    }

    if (repeat_count >= 3) {
      if (ll_distance_code_length_buffer[i - 1] == 0) {
        if (repeat_count <= 10) {
          rle_output.push_back({17, repeat_count});
        } else {
          rle_output.push_back({18, repeat_count});
        }
      } else {
        rle_output.push_back({16, repeat_count});
      }
    } else if (repeat_count > 0) {
      for (unsigned int j {0}; j < repeat_count; j++) {
        rle_output.push_back({ll_distance_code_length_buffer[i - 1]});
      }
    }

    rle_output.push_back({ll_distance_code_length_buffer[i]});
    repeat_count = 0;
  }

  // XXX: Can we avoid this repetition?
  if (repeat_count >= 3) {
    if (ll_distance_code_length_buffer[i - 1] == 0) {
      if (repeat_count <= 10) {
        rle_output.push_back({17, repeat_count});
      } else {
        rle_output.push_back({18, repeat_count});
      }
    } else {
      rle_output.push_back({16, repeat_count});
    }
  } else if (repeat_count > 0) {
    for (unsigned int j {0}; j < repeat_count; j++) {
      rle_output.push_back({ll_distance_code_length_buffer[i - 1]});
    }
  }

  // Compute the frequency of each code length that occurs in the length/literal and distance code length buffer.

  std::unordered_map<unsigned int, unsigned int> ll_distance_code_length_freqs {};
  for (const auto& symbol : rle_output) {
    if (!ll_distance_code_length_freqs.contains(symbol.code)) {
      ll_distance_code_length_freqs.insert({symbol.code, 0});
    }
    ll_distance_code_length_freqs.at(symbol.code)++;
  }

  // Compute the CL codes.

  const unsigned int MAX_CL_CODE_LENGTH {7};
  prefix_codes::PrefixCodeEncoder cl_encoder {MAX_CL_CODE_LENGTH};
  cl_encoder.encode(ll_distance_code_length_freqs);

  // Put the CL code lengths into a contiguous buffer. The codes go into the buffer in a weird order.

  const std::array<unsigned int, 19> CL_CODE_LENGTH_ORDER {16, 17, 18, 0, 8,  7, 9,  6, 10, 5,
                                                           11, 4,  12, 3, 13, 2, 14, 1, 15};
  auto cl_code_lengths {cl_encoder.get_code_length_table()};

  std::vector<unsigned int> cl_code_length_buffer {};
  unsigned int num_cl_codes {0};

  for (unsigned int i {0}; i < CL_CODE_LENGTH_ORDER.size(); i++) {
    unsigned int code {CL_CODE_LENGTH_ORDER[i]};

    if (cl_code_lengths.contains(code)) {
      cl_code_length_buffer.push_back(cl_code_lengths.at(code));
      num_cl_codes = i + 1;
    } else {
      cl_code_length_buffer.push_back(0);
    }
  }

  // Write the number of codes of each type.

  bit_writer_.put_bits(num_ll_codes - 257, 5);
  bit_writer_.put_bits(num_distance_codes - 1, 5);
  bit_writer_.put_bits(num_cl_codes - 4, 4);

  // Write the CL code length table.

  for (unsigned int i {0}; i < num_cl_codes; i++) {
    auto length {cl_code_length_buffer[i]};
    bit_writer_.put_bits(length, 3);
  }

  // Write the length/literal and distance code length tables.

  auto cl_codes {cl_encoder.get_code_table()};

  for (const auto& symbol : rle_output) {
    auto code {cl_codes.at(symbol.code)};
    auto length {cl_code_lengths.at(symbol.code)};
    bit_writer_.put_bits(code, length, false);

    if (symbol.code == 16) {
      bit_writer_.put_bits(symbol.repeat_count - 3, 2);
    } else if (symbol.code == 17) {
      bit_writer_.put_bits(symbol.repeat_count - 3, 3);
    } else if (symbol.code == 18) {
      bit_writer_.put_bits(symbol.repeat_count - 11, 7);
    }
  }

  // Write the compressed data.

  auto ll_codes {ll_encoder.get_code_table()};
  auto distance_codes {distance_encoder.get_code_table()};

  for (const auto& symbol : symbol_list) {
    using enum lzss::LzssSymbolType;

    switch (symbol.get_type()) {
      case LITERAL:
      case LENGTH: {
        auto code {ll_codes.at(symbol.get_code())};
        auto length {ll_code_lengths.at(symbol.get_code())};
        bit_writer_.put_bits(code, length, false);
        break;
      }
      case DISTANCE: {
        auto code {distance_codes.at(symbol.get_code())};
        auto length {distance_code_lengths.at(symbol.get_code())};
        bit_writer_.put_bits(code, length, false);
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

}  // namespace gzip