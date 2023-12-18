#pragma once

#include <cassert>
#include <string>
#include <vector>

namespace lzss {

enum class LzssSymbolType { LITERAL, LENGTH, DISTANCE };

class LzssSymbol
{
public:
  LzssSymbol(LzssSymbolType type, unsigned int value);

  auto get_type() const { return type_; }
  auto get_value() const { return value_; }

  auto get_code() const { return code_; }
  auto get_extra_bits() const
  {
    assert(type_ != LzssSymbolType::LITERAL && "literals do not have extra bits");
    return extra_bits_;
  }
  auto get_offset() const
  {
    assert(type_ != LzssSymbolType::LITERAL && "literals do not have offsets");
    return offset_;
  }

private:
  void lookup_code_table_data();

  const LzssSymbolType type_;
  const unsigned int value_;

  unsigned int code_;
  unsigned int extra_bits_;
  unsigned int offset_;
};

extern const LzssSymbol END_OF_BLOCK_MARKER;

class LzssSymbolList
{
public:
  void add(const LzssSymbol &symbol);
  void clear();

  std::string to_string() const;

  auto begin() const { return list_.begin(); }
  auto end() const { return list_.end(); }

private:
  std::vector<LzssSymbol> list_{};
};

}  // namespace lzss
