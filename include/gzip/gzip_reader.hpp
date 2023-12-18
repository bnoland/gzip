#pragma once

#include "bit_io/bit_reader.hpp"
#include "prefix_codes/prefix_code_decoder.hpp"
#include "prefix_codes/prefix_code_types.hpp"

#include <deque>
#include <exception>
#include <istream>
#include <ostream>
#include <string>

namespace gzip {

class GzipReader
{
public:
  GzipReader(std::istream &input, std::ostream &output);

  void read();

private:
  void read_header();
  void read_deflate_bit_stream();
  void read_footer();

  void read_block_type_0();
  void read_block_type_1();
  void read_block_type_2();

  // XXX: All this stuff should go elsewhere.
  void compute_fixed_code_tables();
  prefix_codes::CodeLengthTable fixed_ll_code_lengths_{};
  prefix_codes::CodeLengthTable fixed_distance_code_lengths_{};
  prefix_codes::PrefixCodeDecoder fixed_ll_code_decoder_{ bit_reader_, fixed_ll_code_lengths_ };
  prefix_codes::PrefixCodeDecoder fixed_distance_code_decoder_{ bit_reader_, fixed_distance_code_lengths_ };

  std::istream &input_;
  std::ostream &output_;
  bit_io::BitReader bit_reader_;

  // XXX: This should be managed by an LZSS decoder class.
  std::deque<unsigned int> history_{};
};

class GzipReaderError : public std::exception
{
public:
  GzipReaderError(const std::string &message) : message_{ message } {}

  const char *what() const noexcept { return message_.c_str(); }

private:
  const std::string message_;
};

}  // namespace gzip
