#pragma once

#include "bit_io/bit_reader.hpp"

#include <string>
#include <exception>
#include <istream>
#include <ostream>

namespace gzip {

class GzipReader {
 public:
  GzipReader(std::istream& input, std::ostream& output);

  void read();

 private:
  void read_header();
  void read_deflate_bit_stream();
  void read_footer();

  void read_block_type_0();

  std::istream& input_;
  std::ostream& output_;
  bit_io::BitReader bit_reader_;
};

class GzipReaderError : public std::exception {
 public:
  GzipReaderError(const std::string& message) : message_ {message} {}

  const char* what() const noexcept { return message_.c_str(); }

 private:
  const std::string message_;
};

}  // namespace gzip
