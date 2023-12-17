#pragma once

#include <map>
#include <unordered_map>

namespace prefix_codes {

using FrequencyTable = std::unordered_map<unsigned int, unsigned int>;
using CodeTable = std::unordered_map<unsigned int, unsigned int>;
using CodeLengthTable = std::map<unsigned int, unsigned int>;

}  // namespace prefix_codes
