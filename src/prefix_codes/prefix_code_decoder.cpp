#include "prefix_codes/prefix_code_decoder.hpp"
#include "prefix_codes/canonical_codes.hpp"
#include "prefix_codes/decoding_error.hpp"
#include "prefix_codes/types.hpp"

#include <cassert>
#include <memory>
#include <optional>

namespace prefix_codes {

prefix_code_decoder::prefix_code_decoder(
    std::istream& input, const types::code_length_table& code_length_table)
    : code_length_table_ {code_length_table}, bit_reader_ {input} {
  build_tree(compute_canonical_code_table(code_length_table_));
}

std::string prefix_code_decoder::decode_symbols(unsigned int num_symbols) {
  std::string result {};
  for (unsigned int i {0}; i < num_symbols; i++) {
    result += decode_symbol();
  }
  return result;
}

types::symbol prefix_code_decoder::decode_symbol() {
  node_ptr current {tree_root_};

  while (true) {
    if (current == nullptr) {
      throw errors::decoding_error("unable to decode symbol");
    }

    if (current->left == nullptr && current->right == nullptr) {
      assert(current->symbol.has_value() && "leaf node has no symbol");
      return current->symbol.value();
    }

    bool bit {bit_reader_.get_single_bit()};

    if (bit_reader_.eof()) {
      throw errors::decoding_error("reached end of input");
    }

    if (bit == false) {
      current = current->left;
    } else {
      current = current->right;
    }
  }
}

void prefix_code_decoder::build_tree(const types::code_table& code_table) {
  for (auto [symbol, code] : code_table) {
    auto length = code_length_table_.at(symbol);
    insert_symbol(symbol, code, length);
  }
}

void prefix_code_decoder::insert_symbol(types::symbol symbol,
                                        types::code_value code,
                                        types::code_length code_length) {
  if (tree_root_ == nullptr) {
    tree_root_ = make_node();
  }

  node_ptr current {tree_root_};

  for (unsigned int i {0}; i < code_length; i++) {
    bool bit = code & (1 << (code_length - i - 1));

    if (bit == false) {
      if (current->left == nullptr) {
        current->left = make_node();
      }
      current = current->left;
    } else {
      if (current->right == nullptr) {
        current->right = make_node();
      }
      current = current->right;
    }
  }

  current->symbol = symbol;
}

prefix_code_decoder::node_ptr prefix_code_decoder::make_node() {
  return std::make_shared<node>(std::nullopt, nullptr, nullptr);
}

}  // namespace prefix_codes
