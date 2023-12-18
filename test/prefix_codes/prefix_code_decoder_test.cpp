#include "bit_io/bit_reader.hpp"
#include "bit_io/bit_writer.hpp"
#include "prefix_codes/prefix_code_decoder.hpp"
#include "prefix_codes/prefix_code_encoder.hpp"
#include "prefix_codes/prefix_code_types.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <sstream>
#include <string>
#include <tuple>

TEST_CASE("Can decode a stream of encoded symbols", "[prefix_code_decoder]")
{
  // clang-format off
  auto [input, max_code_length] = GENERATE(
    std::make_tuple<std::string, unsigned int>("abcccdddddeeeeeefffffffffffggggggggggggg", 4),
    std::make_tuple<std::string, unsigned int>("hello", 15),
    std::make_tuple<std::string, unsigned int>("", 15)
  );
  // clang-format on

  CAPTURE(input, max_code_length);

  prefix_codes::FrequencyTable frequencies{};
  for (auto symbol : input) {
    if (!frequencies.contains(symbol)) {
      frequencies.insert({ symbol, 0 });
    }
    frequencies.at(symbol)++;
  }

  prefix_codes::PrefixCodeEncoder encoder{ max_code_length };
  encoder.encode(frequencies);

  std::ostringstream oss{};
  bit_io::BitWriter bit_writer{ oss };

  auto code_table{ encoder.get_code_table() };
  auto code_length_table{ encoder.get_code_length_table() };

  for (auto symbol : input) {
    bit_writer.put_bits(code_table.at(symbol), code_length_table.at(symbol), false);
  }
  bit_writer.finish();

  std::istringstream iss{ oss.str() };
  bit_io::BitReader bit_reader{ iss };

  prefix_codes::PrefixCodeDecoder decoder{ bit_reader, code_length_table };

  std::string symbols{};
  for (unsigned int i{ 0 }; i < input.length(); i++) {
    symbols += decoder.decode_symbol();
  }

  REQUIRE(symbols == input);
}
