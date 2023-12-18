#include "prefix_codes/prefix_code_decoder.hpp"
#include "bit_io/bit_reader.hpp"
#include "prefix_codes/canonical_codes.hpp"

#include <memory>

namespace prefix_codes {

PrefixCodeDecoder::PrefixCodeDecoder(bit_io::BitReader &bit_reader, const CodeLengthTable &code_length_table)
  : code_length_table_{ code_length_table }, bit_reader_{ bit_reader }
{}

void PrefixCodeDecoder::initialize()
{
  compute_code_table();
  build_tree();
}

unsigned int PrefixCodeDecoder::decode_symbol()
{
  NodePtr current{ tree_root_ };
  while (true) {
    if (current == nullptr) {
      throw DecodingError("Unable to decode symbol.");
    }

    if (current->left == nullptr && current->right == nullptr) {
      return current->symbol.value();
    }

    bool bit{ bit_reader_.get_single_bit() };
    if (bit_reader_.eof()) {
      throw DecodingError("Reached end of input while decoding.");
    }

    if (bit) {
      current = current->right;
    } else {
      current = current->left;
    }
  }
}

void PrefixCodeDecoder::compute_code_table()
{
  code_table_ = compute_canonical_code_table(code_length_table_);
}

void PrefixCodeDecoder::build_tree()
{
  for (auto [symbol, code] : code_table_) {
    insert_symbol(symbol, code);
  }
}

void PrefixCodeDecoder::insert_symbol(unsigned int symbol, unsigned int code)
{
  if (tree_root_ == nullptr) {
    tree_root_ = std::make_shared<Node>(symbol);
  }

  NodePtr current{ tree_root_ };
  unsigned int length{ code_length_table_.at(symbol) };

  for (int bit = length - 1; bit >= 0; bit--) {
    unsigned int mask{ 1U << bit };
    if (code & mask) {
      if (current->right == nullptr) {
        current->right = std::make_shared<Node>();
      }
      current = current->right;
    } else {
      if (current->left == nullptr) {
        current->left = std::make_shared<Node>();
      }
      current = current->left;
    }
  }

  current->symbol = symbol;
}

}  // namespace prefix_codes
