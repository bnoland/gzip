#include "lzss/lzss_string_table.hpp"
#include "lzss/lzss_constants.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace lzss {

LzssStringTable::LzssStringTable(unsigned int max_chain_length) : max_chain_length_{ max_chain_length } {}

void LzssStringTable::insert(std::string_view string, unsigned int position)
{
  auto index{ get_chain_index(string) };
  auto &chain{ chains_[index] };
  chain.push_front({ std::string{ string }, position });
  if (chain.size() > max_chain_length_) {
    chain.pop_back();
  }
}

std::optional<BackReference> LzssStringTable::get_back_reference(std::string_view string) const
{
  auto index{ get_chain_index(string) };

  for (const auto &[search, position] : chains_[index]) {
    unsigned int length{ 0 };

    for (unsigned int i{ 0 }; i < string.length(); i++) {
      if (i == search.length() || string[i] != search[i]) {
        break;
      }
      length++;
    }

    if (length >= constants::MIN_BACKREF_LENGTH) {
      return BackReference{ position, length };
    }
  }

  return {};
}

std::size_t LzssStringTable::get_chain_index(std::string_view string) const
{
  std::string key{ string.substr(0, 3) };
  return hash_(key) % std::size(chains_);
}

unsigned int LzssStringTable::get_average_chain_length() const
{
  unsigned int total{ 0 };
  for (const auto &chain : chains_) {
    total += chain.size();
  }
  return total / std::size(chains_);
}

std::string LzssStringTable::to_string() const
{
  std::string result{};

  for (const auto &chain : chains_) {
    for (const auto &[string, position] : chain) {
      result += "(" + string + ", " + std::to_string(position) + ")\n";
    }
  }

  return result;
}

}  // namespace lzss
