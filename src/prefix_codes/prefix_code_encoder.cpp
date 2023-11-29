#include "prefix_codes/prefix_code_encoder.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <memory>
#include <optional>
#include <map>
#include <unordered_map>

namespace prefix_codes {

prefix_code_encoder::prefix_code_encoder(unsigned int max_code_length)
    : max_code_length_ {max_code_length} {
  assert(max_code_length <= MAX_CODE_LENGTH_VALUE &&
         "max code length is too large");
}

void prefix_code_encoder::encode(const frequency_table& frequencies) {
  compute_code_length_table(frequencies);
  compute_code_table();
}

void prefix_code_encoder::compute_code_length_table(
    const frequency_table& frequencies) {
  if (frequencies.size() == 1) {
    code_length_table_ = {{frequencies.at(0), 1}};
    return;
  }

  assert(max_code_length_ >= std::ceil(std::log2(frequencies.size())) &&
         "max code length is too small");

  package_list singletons {};
  for (auto [symbol, freq] : frequencies) {
    singletons.push_back(std::make_shared<package_node>(symbol, freq));
  }

  auto compare = [](const auto& lhs, const auto& rhs) {
    return lhs->weight < rhs->weight;
  };

  std::sort(singletons.begin(), singletons.end(), compare);

  package_list packages {singletons};
  for (unsigned int i {0}; i < max_code_length_ - 1; i++) {
    packages = merge(package(packages), singletons);
  }

  for (unsigned int i {0}; i < 2 * frequencies.size() - 2; i++) {
    expand_package(packages[i]);
  }
}

void prefix_code_encoder::compute_code_table() {
  std::map<unsigned int, unsigned int> length_counts {};

  for (auto [symbol, length] : code_length_table_) {
    if (!length_counts.contains(length)) {
      length_counts.insert({length, 0});
    }
    length_counts.at(length)++;
  }

  std::unordered_map<unsigned int, unsigned int> next_code {};
  unsigned int code {0};

  for (auto [length, count] : length_counts) {
    if (length_counts.contains(length - 1)) {
      code = (code + length_counts.at(length - 1)) << 1;
    }
    next_code.insert({length, code});
  }

  for (auto [symbol, length] : code_length_table_) {
    assert(length > 0 && "got a zero length when generating codes");
    code_table_.insert({symbol, next_code.at(length)});
    next_code.at(length)++;
  }
}

void prefix_code_encoder::expand_package(package_node_ptr package) {
  if (package->left == nullptr && package->right == nullptr) {
    assert(package->symbol.has_value() &&
           "package has leaf node with no symbol");

    auto symbol {package->symbol.value()};
    if (!code_length_table_.contains(symbol)) {
      code_length_table_.insert({symbol, 0});
    }
    code_length_table_.at(symbol)++;
    return;
  }

  if (package->left != nullptr) {
    expand_package(package->left);
  }

  if (package->right != nullptr) {
    expand_package(package->right);
  }
}

prefix_code_encoder::package_list prefix_code_encoder::package(
    const package_list& list) {
  package_list result {};

  for (unsigned int i {0}; i < list.size() / 2; i++) {
    auto left {list[2 * i]};
    auto right {list[2 * i + 1]};
    result.push_back(std::make_shared<package_node>(
        std::nullopt, left->weight + right->weight, left, right));
  }

  return result;
}

prefix_code_encoder::package_list prefix_code_encoder::merge(
    const package_list& list1, const package_list& list2) {
  package_list result {};
  unsigned int i {0}, j {0};

  while (i < list1.size() && j < list2.size()) {
    if (list1[i]->weight < list2[j]->weight) {
      result.push_back(list1[i++]);
    } else {
      result.push_back(list2[j++]);
    }
  }

  while (i < list1.size()) {
    result.push_back(list1[i++]);
  }

  while (j < list2.size()) {
    result.push_back(list2[j++]);
  }

  return result;
}

}  // namespace prefix_codes
