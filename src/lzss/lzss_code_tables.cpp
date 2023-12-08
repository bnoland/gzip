#include "lzss/lzss_code_tables.hpp"

#include <array>
#include <cassert>

namespace lzss::code_tables {

namespace {

const std::array LENGTH_CODE_TABLE {
  Entry {257, 0, 3, 3},     Entry {258, 0, 4, 4},     Entry {259, 0, 5, 5},     Entry {260, 0, 6, 6},
  Entry {261, 0, 7, 7},     Entry {262, 0, 8, 8},     Entry {263, 0, 9, 9},     Entry {264, 0, 10, 10},
  Entry {265, 1, 11, 12},   Entry {266, 1, 13, 14},   Entry {267, 1, 15, 16},   Entry {268, 1, 17, 18},
  Entry {269, 2, 19, 22},   Entry {270, 2, 23, 26},   Entry {271, 2, 27, 30},   Entry {272, 2, 31, 34},
  Entry {273, 3, 35, 42},   Entry {274, 3, 43, 50},   Entry {275, 3, 51, 58},   Entry {276, 3, 59, 66},
  Entry {277, 4, 67, 82},   Entry {278, 4, 83, 98},   Entry {279, 4, 99, 114},  Entry {280, 4, 115, 130},
  Entry {281, 5, 131, 162}, Entry {282, 5, 163, 194}, Entry {283, 5, 195, 226}, Entry {284, 5, 227, 257},
  Entry {285, 0, 258, 258},
};

const std::array DISTANCE_CODE_TABLE {
  Entry {0, 0, 1, 1},           Entry {1, 0, 2, 2},           Entry {2, 0, 3, 3},          Entry {3, 0, 4, 4},
  Entry {4, 1, 5, 6},           Entry {5, 1, 7, 8},           Entry {6, 2, 9, 12},         Entry {7, 2, 13, 16},
  Entry {8, 3, 17, 24},         Entry {9, 3, 25, 32},         Entry {10, 4, 33, 48},       Entry {11, 4, 49, 64},
  Entry {12, 5, 65, 96},        Entry {13, 5, 97, 128},       Entry {14, 6, 129, 192},     Entry {15, 6, 193, 256},
  Entry {16, 7, 257, 384},      Entry {17, 7, 385, 512},      Entry {18, 8, 513, 768},     Entry {19, 8, 769, 1024},
  Entry {20, 9, 1025, 1536},    Entry {21, 9, 1537, 2048},    Entry {22, 10, 2049, 3072},  Entry {23, 10, 3073, 4096},
  Entry {24, 11, 4097, 6144},   Entry {25, 11, 6145, 8192},   Entry {26, 12, 8193, 12288}, Entry {27, 12, 12289, 16384},
  Entry {28, 13, 16385, 24576}, Entry {29, 13, 24577, 32768},
};

}  // namespace

const Entry& get_length_entry(unsigned int length) {
  for (const auto& entry : LENGTH_CODE_TABLE) {
    if (length >= entry.lower_bound && length <= entry.upper_bound) {
      return entry;
    }
  }

  assert(false && "searching for invalid length in code table");
}

const Entry& get_distance_entry(unsigned int distance) {
  for (const auto& entry : DISTANCE_CODE_TABLE) {
    if (distance >= entry.lower_bound && distance <= entry.upper_bound) {
      return entry;
    }
  }

  assert(false && "searching for invalid distance in code table");
}

}  // namespace lzss::code_tables
