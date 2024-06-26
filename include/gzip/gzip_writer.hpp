#pragma once

#include "bit_io/bit_writer.hpp"
#include "lzss/lzss_encoder.hpp"
#include "prefix_codes/fixed_code_table.hpp"

#include <istream>
#include <ostream>
#include <string>
#include <string_view>

namespace gzip {

class GzipWriter
{
public:
  GzipWriter(std::istream &input, std::ostream &output);

  void write();

private:
  void write_header();
  void write_deflate_bit_stream();
  void write_footer();

  // XXX: Put this stuff in a separate deflate writer class?
  static const unsigned int INPUT_CHUNK_SIZE{ 65535 };
  std::string read_input_chunk();
  void write_block_type_0(std::string_view input_buffer, bool is_last_block);
  void write_block_type_1(std::string_view input_buffer, bool is_last_block);
  void write_block_type_2(std::string_view input_buffer, bool is_last_block);

  unsigned int input_size_{ 0 };
  std::istream &input_;
  std::ostream &output_;
  bit_io::BitWriter bit_writer_;

  prefix_codes::FixedCodeTable fixed_code_table_{};
  lzss::LzssEncoder lzss_encoder_{};
};

}  // namespace gzip
