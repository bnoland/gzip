#include "lzss/lzss_encoder.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <string>

TEST_CASE("Correctly encodes input symbols", "[encoder]") {
  auto [input_buffer,
        expected_output] = GENERATE(table<std::string, std::string>({
    {"", ""},
    {"abcdef", "abcdef"},
    {"banana", "ban<3:2>"},
    {"aaaaaa", "a<5:1>"},
    {"ababab", "ab<4:2>"},
    {"a lass; a lad; a salad; alaska", "a lass; <4:8>d<4:7>sala<4:9><3:23>ka"},
  }));

  CAPTURE(input_buffer);

  lzss::lzss_encoder encoder {};
  encoder.encode(input_buffer);

  auto symbol_list {encoder.get_symbol_list()};

  REQUIRE(symbol_list.to_string() == expected_output);
}
