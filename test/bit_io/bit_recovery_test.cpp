#include "bit_io/bit_reader.hpp"
#include "bit_io/bit_writer.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <sstream>

TEST_CASE("Can read back written bits", "[bit_writer][bit_reader]") {
  std::ostringstream oss {};
  bit_io::bit_writer bit_writer {oss};

  std::istringstream iss {};
  bit_io::bit_reader bit_reader {iss};

  // XXX: Can we improve how this lambda function is written?
  auto recover_bits = [&](uint64_t value, int num_bits,
                          bool low_bit_first = true) {
    bit_writer.put_bits(value, num_bits, low_bit_first);
    bit_writer.finish();
    iss.str(oss.str());
  };

  SECTION("Read and write low bit first") {
    SECTION("Bits written and read in correct order") {
      recover_bits(0b1011, 2);
      REQUIRE(bit_reader.get_bits(2) == 0b0011);
    }

    SECTION("Bits written and read as unsigned integer") {
      recover_bits(0xffffffff, 32);
      REQUIRE(bit_reader.get_bits(32) == 0xffffffff);
    }
  }

  SECTION("Read and write high bit first") {
    SECTION("Bits written and read in correct order") {
      recover_bits(0b1110, 2, false);
      REQUIRE(bit_reader.get_bits(2, false) == 0b10);
    }

    SECTION("Bits written and read as unsigned integer") {
      recover_bits(0xffffffff, 32, false);
      REQUIRE(bit_reader.get_bits(32, false) == 0xffffffff);
    }
  }
}
