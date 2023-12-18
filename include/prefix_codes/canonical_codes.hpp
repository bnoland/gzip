#pragma once

#include "prefix_codes/prefix_code_types.hpp"

namespace prefix_codes {

CodeTable compute_canonical_code_table(const CodeLengthTable &code_length_table);

}
