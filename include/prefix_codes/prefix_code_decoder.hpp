#pragma once

#include "bit_io/bit_reader.hpp"
#include "prefix_codes/prefix_code_types.hpp"

#include <exception>
#include <memory>
#include <optional>

namespace prefix_codes {

class PrefixCodeDecoder
{
public:
  PrefixCodeDecoder(bit_io::BitReader &bit_reader, const CodeLengthTable &code_length_table);

  void initialize();
  unsigned int decode_symbol();

private:
  struct Node;
  using NodePtr = std::shared_ptr<Node>;

  struct Node
  {
    std::optional<unsigned int> symbol;
    NodePtr left{ nullptr };
    NodePtr right{ nullptr };
  };

  void compute_code_table();
  void build_tree();
  void insert_symbol(unsigned int symbol, unsigned int code);

  NodePtr tree_root_;
  CodeTable code_table_;
  const CodeLengthTable &code_length_table_;
  bit_io::BitReader &bit_reader_;
};

class DecodingError : public std::exception
{
public:
  DecodingError(const std::string &message) : message_{ message } {}

  const char *what() const noexcept { return message_.c_str(); }

private:
  const std::string message_;
};

}  // namespace prefix_codes
