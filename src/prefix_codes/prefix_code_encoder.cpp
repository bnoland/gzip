#include "prefix_codes/prefix_code_encoder.hpp"
#include "prefix_codes/canonical_codes.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <map>
#include <memory>
#include <optional>

namespace prefix_codes {

PrefixCodeEncoder::PrefixCodeEncoder(unsigned int max_code_length) : max_code_length_{ max_code_length } {}

void PrefixCodeEncoder::encode(const FrequencyTable &frequencies)
{
  compute_code_length_table(frequencies);
  compute_code_table();
}

void PrefixCodeEncoder::compute_code_length_table(const FrequencyTable &frequencies)
{
  if (frequencies.size() == 0) {
    // Block type 2 seems to insist on having at least 1 distance code, so we put a dummy code here.
    code_length_table_ = { { 0, 0 } };
    return;
  }

  if (frequencies.size() == 1) {
    code_length_table_ = { { frequencies.begin()->first, 1 } };
    return;
  }

  assert(max_code_length_ >= std::ceil(std::log2(frequencies.size())) && "max code length is too small");

  PackageList singletons{};
  for (auto [symbol, freq] : frequencies) {
    singletons.push_back(std::make_shared<PackageNode>(symbol, freq));
  }

  auto compare = [](const auto &lhs, const auto &rhs) { return lhs->weight < rhs->weight; };

  std::sort(singletons.begin(), singletons.end(), compare);

  PackageList packages{ singletons };
  for (unsigned int i{ 0 }; i < max_code_length_ - 1; i++) {
    packages = merge(package(packages), singletons);
  }

  for (unsigned int i{ 0 }; i < 2 * frequencies.size() - 2; i++) {
    expand_package(packages[i]);
  }
}

void PrefixCodeEncoder::compute_code_table()
{
  code_table_ = compute_canonical_code_table(code_length_table_);
}

void PrefixCodeEncoder::expand_package(PackageNodePtr package)
{
  if (package->left == nullptr && package->right == nullptr) {
    assert(package->symbol.has_value() && "package has leaf node with no symbol");

    auto symbol{ package->symbol.value() };
    if (!code_length_table_.contains(symbol)) {
      code_length_table_.insert({ symbol, 0 });
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

PrefixCodeEncoder::PackageList PrefixCodeEncoder::package(const PackageList &list)
{
  PackageList result{};

  for (unsigned int i{ 0 }; i < list.size() / 2; i++) {
    auto left{ list[2 * i] };
    auto right{ list[2 * i + 1] };
    result.push_back(std::make_shared<PackageNode>(std::nullopt, left->weight + right->weight, left, right));
  }

  return result;
}

PrefixCodeEncoder::PackageList PrefixCodeEncoder::merge(const PackageList &list1, const PackageList &list2)
{
  PackageList result{};
  unsigned int i{ 0 }, j{ 0 };

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
