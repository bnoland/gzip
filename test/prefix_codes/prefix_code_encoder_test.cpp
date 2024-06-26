#include "prefix_codes/prefix_code_encoder.hpp"
#include "prefix_codes/prefix_code_types.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <tuple>

// XXX: Check that assertions are thrown when preconditions are invalid.
// I don't think Catch2 supports this. Might want to try out GoogleTest instead.

TEST_CASE("Generates single code of length 1 when input has single symbol", "[prefix_code_encoder]")
{
  // clang-format off
  auto [frequencies, max_code_length] = GENERATE(
    std::make_tuple(prefix_codes::FrequencyTable{{'a', 1}}, 1U),
    std::make_tuple(prefix_codes::FrequencyTable{{'a', 5}}, 1U)
  );
  // clang-format on

  CAPTURE(frequencies, max_code_length);

  prefix_codes::PrefixCodeEncoder encoder{ max_code_length };

  encoder.encode(frequencies);

  auto code_length_table{ encoder.get_code_length_table() };
  REQUIRE(code_length_table.size() == 1);
  REQUIRE(code_length_table.at(frequencies.begin()->first) == 1);

  auto code_table{ encoder.get_code_table() };
  REQUIRE(code_table.size() == 1);
}

TEST_CASE("Generates valid code lengths when input has multiple symbols", "[prefix_code_encoder]")
{
  // clang-format off
  auto [frequencies, max_code_length] = GENERATE(
    std::make_tuple(prefix_codes::FrequencyTable{{'a', 1}, {'b', 1}, {'c', 3}, {'d', 5}, {'e', 6}, {'f', 11}, {'g', 13}}, 3U),
    std::make_tuple(prefix_codes::FrequencyTable{{'a', 1}, {'b', 1}, {'c', 3}, {'d', 5}, {'e', 6}, {'f', 11}, {'g', 13}}, 4U),
    std::make_tuple(prefix_codes::FrequencyTable{{'a', 1}, {'b', 1}, {'c', 3}, {'d', 5}, {'e', 6}, {'f', 11}, {'g', 13}}, 5U),
    std::make_tuple(prefix_codes::FrequencyTable{{'a', 1}, {'b', 1}, {'c', 3}, {'d', 5}, {'e', 6}, {'f', 11}, {'g', 13}}, 15U),
    std::make_tuple(prefix_codes::FrequencyTable{{'a', 1}, {'b', 1}}, 1U)
  );
  // clang-format on

  CAPTURE(frequencies, max_code_length);

  prefix_codes::PrefixCodeEncoder encoder{ max_code_length };

  encoder.encode(frequencies);
  auto code_length_table{ encoder.get_code_length_table() };

  SECTION("Code lengths satisfy Kraft-McMillan with equality")
  {
    unsigned int kraft_sum{ 0 };
    for (auto [symbol, length] : code_length_table) {
      kraft_sum += (1 << (max_code_length - length));
    }
    REQUIRE(kraft_sum == (1U << max_code_length));
  }

  SECTION("Code lengths do not exceed maximum length")
  {
    for (auto [symbol, length] : code_length_table) {
      DYNAMIC_SECTION("Symbol " << symbol << " has valid length")
      {
        REQUIRE(length <= max_code_length);
      }
    }
  }
}
