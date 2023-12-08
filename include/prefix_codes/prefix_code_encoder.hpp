#pragma once

#include <limits>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>
#include <map>

namespace prefix_codes {

class PrefixCodeEncoder {
 public:
  PrefixCodeEncoder(unsigned int max_code_length);

  using FrequencyTable = std::unordered_map<unsigned int, unsigned int>;

  void encode(const FrequencyTable& frequencies);

  const auto& get_code_table() const { return code_table_; }
  const auto& get_code_length_table() const { return code_length_table_; }

 private:
  void compute_code_length_table(const FrequencyTable& frequencies);
  void compute_code_table();

  using CodeTable = std::unordered_map<unsigned int, unsigned int>;
  using CodeLengthTable = std::map<unsigned int, unsigned int>;

  static const unsigned int MAX_CODE_LENGTH_VALUE {std::numeric_limits<unsigned int>::digits};

  const unsigned int max_code_length_;
  CodeLengthTable code_length_table_;
  CodeTable code_table_;

  struct PackageNode;
  using PackageNodePtr = std::shared_ptr<PackageNode>;
  using PackageList = std::vector<PackageNodePtr>;

  struct PackageNode {
    std::optional<unsigned int> symbol;
    unsigned int weight;
    PackageNodePtr left {nullptr};
    PackageNodePtr right {nullptr};
  };

  void expand_package(PackageNodePtr package);
  static PackageList package(const PackageList& list);
  static PackageList merge(const PackageList& list1, const PackageList& list2);
};

}  // namespace prefix_codes
