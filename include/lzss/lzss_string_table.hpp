#pragma once

#include <functional>
#include <list>
#include <optional>
#include <string>
#include <string_view>

namespace lzss {

struct back_reference {
  unsigned int position;
  unsigned int length;
};

class lzss_string_table {
 public:
  lzss_string_table(unsigned int max_chain_length = 10);

  void insert(std::string_view string, unsigned int position);
  std::optional<back_reference> get_back_reference(std::string_view string) const;

  unsigned int get_average_chain_length() const;
  std::string to_string() const;

 private:
  std::size_t get_chain_index(std::string_view string) const;

  struct entry {
    std::string string;
    unsigned int position;
  };

  static const unsigned int MAX_CHAINS {65536};
  std::list<entry> chains_[MAX_CHAINS];
  std::hash<std::string> hash_ {};
  const unsigned int max_chain_length_;
};

}  // namespace lzss
