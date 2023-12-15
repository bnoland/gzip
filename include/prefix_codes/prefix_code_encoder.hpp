#pragma once

#include "prefix_codes/prefix_code_types.hpp"

#include <memory>
#include <optional>
#include <vector>

namespace prefix_codes {

class PrefixCodeEncoder {
 public:
  PrefixCodeEncoder(unsigned int max_code_length);

  void encode(const FrequencyTable& frequencies);

  const auto& get_code_table() const { return code_table_; }
  const auto& get_code_length_table() const { return code_length_table_; }

 private:
  struct PackageNode;
  using PackageNodePtr = std::shared_ptr<PackageNode>;
  using PackageList = std::vector<PackageNodePtr>;

  struct PackageNode {
    std::optional<unsigned int> symbol;
    unsigned int weight;
    PackageNodePtr left {nullptr};
    PackageNodePtr right {nullptr};
  };

  void compute_code_length_table(const FrequencyTable& frequencies);
  void compute_code_table();

  void expand_package(PackageNodePtr package);
  static PackageList package(const PackageList& list);
  static PackageList merge(const PackageList& list1, const PackageList& list2);

  const unsigned int max_code_length_;
  CodeLengthTable code_length_table_;
  CodeTable code_table_;
};

}  // namespace prefix_codes
