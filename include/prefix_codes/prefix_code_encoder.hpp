#pragma once

#include <limits>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>
#include <map>

namespace prefix_codes {

class prefix_code_encoder {
 public:
  prefix_code_encoder(unsigned int max_code_length);

  using frequency_table = std::unordered_map<unsigned int, unsigned int>;

  void encode(const frequency_table& frequencies);

  const auto& get_code_table() const { return code_table_; }
  const auto& get_code_length_table() const { return code_length_table_; }

 private:
  void compute_code_length_table(const frequency_table& frequencies);
  void compute_code_table();

  using code_table = std::unordered_map<unsigned int, unsigned int>;
  using code_length_table = std::map<unsigned int, unsigned int>;

  static const unsigned int MAX_CODE_LENGTH_VALUE {std::numeric_limits<unsigned int>::digits};

  const unsigned int max_code_length_;
  code_length_table code_length_table_;
  code_table code_table_;

  struct package_node;
  using package_node_ptr = std::shared_ptr<package_node>;
  using package_list = std::vector<package_node_ptr>;

  struct package_node {
    std::optional<unsigned int> symbol;
    unsigned int weight;
    package_node_ptr left {nullptr};
    package_node_ptr right {nullptr};
  };

  void expand_package(package_node_ptr package);
  static package_list package(const package_list& list);
  static package_list merge(const package_list& list1, const package_list& list2);
};

}  // namespace prefix_codes
