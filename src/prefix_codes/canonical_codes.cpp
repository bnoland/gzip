#include "prefix_codes/canonical_codes.hpp"
#include "prefix_codes/prefix_code_types.hpp"

#include <array>
#include <cassert>

namespace prefix_codes {

CodeTable compute_canonical_code_table(const CodeLengthTable &code_length_table)
{
  const unsigned int MAX_CODE_LENGTH_VALUE{ 15 };

  std::array<unsigned int, MAX_CODE_LENGTH_VALUE + 1> length_counts{ 0 };
  for (auto [symbol, length] : code_length_table) {
    assert(length <= MAX_CODE_LENGTH_VALUE);
    length_counts[length]++;
  }

  std::array<unsigned int, MAX_CODE_LENGTH_VALUE + 1> next_code;

  unsigned int code{ 0 };
  for (unsigned int bits{ 1 }; bits <= MAX_CODE_LENGTH_VALUE; bits++) {
    code = (code + length_counts[bits - 1]) << 1;
    next_code[bits] = code;
  }

  CodeTable code_table{};

  for (auto [symbol, length] : code_length_table) {
    assert(length <= MAX_CODE_LENGTH_VALUE);
    code_table.insert({ symbol, next_code[length] });
    next_code[length]++;
  }

  return code_table;
}

}  // namespace prefix_codes
