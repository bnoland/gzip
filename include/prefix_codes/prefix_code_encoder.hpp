#pragma once

#include <limits>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace prefix_codes {

class prefix_code_encoder {
 public:
  // XXX: Put custom types in separate file for use by decoder?
  using symbol_value = unsigned int;
  using code_value = uint64_t;
  using code_length = unsigned int;
  using code_table = std::unordered_map<symbol_value, code_value>;
  using code_length_table = std::unordered_map<symbol_value, code_length>;
  // using code_length_list = std::vector<std::pair<symbol_value, code_length>>;
  using frequency_table = std::unordered_map<symbol_value, unsigned int>;

  prefix_code_encoder(code_length max_code_length);

  void encode(const frequency_table& frequencies);

  const auto& get_code_table() const { return code_table_; }
  const auto& get_code_length_table() const { return code_length_table_; }

 private:
  void compute_code_length_table(const frequency_table& frequencies);
  void compute_code_table();

  struct package_node;
  using package_node_ptr = std::shared_ptr<package_node>;
  using package_list = std::vector<package_node_ptr>;

  struct package_node {
    std::optional<symbol_value> symbol;
    code_length weight;
    package_node_ptr left {nullptr};
    package_node_ptr right {nullptr};
  };

  void expand_package(package_node_ptr package);

  static package_list package(const package_list& list);

  static package_list merge(const package_list& list1,
                            const package_list& list2);

  static const unsigned int MAX_CODE_LENGTH_VALUE {
    std::numeric_limits<code_value>::digits};

  const code_length max_code_length_;
  code_length_table code_length_table_;
  code_table code_table_;
};

}  // namespace prefix_codes
