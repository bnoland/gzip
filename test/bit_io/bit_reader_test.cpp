#include "bit_io/bit_reader.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <initializer_list>
#include <sstream>
#include <string>
#include <vector>

void set_stream_bytes(std::istringstream& iss,
                      std::initializer_list<uint8_t> bytes) {
  std::vector<uint8_t> buffer {bytes};
  std::string content {buffer.begin(), buffer.end()};
  iss.str(content);
}

TEST_CASE("get_single_bit()", "[bit_reader][get_single_bit]") {
  std::istringstream iss {};
  bit_io::bit_reader bit_reader {iss};

  SECTION("Reads bits in correct order") {
    set_stream_bytes(iss, {0b00000101});

    REQUIRE(bit_reader.get_single_bit() == true);
    REQUIRE(bit_reader.get_single_bit() == false);
    REQUIRE(bit_reader.get_single_bit() == true);
    REQUIRE(bit_reader.get_single_bit() == false);
  }

  SECTION("Properly handles byte boundary") {
    set_stream_bytes(iss, {0b11111111, 0b00000001});
    for (int i = 0; i < 9; i++) {
      REQUIRE(bit_reader.get_single_bit() == true);
    }
  }
}

TEST_CASE("get_bits()", "[bit_reader][get_bits]") {
  std::istringstream iss {};
  bit_io::bit_reader bit_reader {iss};

  SECTION("Read low bit first") {
    SECTION("Reads bits in correct order") {
      set_stream_bytes(iss, {0b10101110});
      REQUIRE(bit_reader.get_bits(4) == 0b1110);
    }

    SECTION("Properly handles byte boundary") {
      set_stream_bytes(iss, {0b11111111, 0b00000001});
      REQUIRE(bit_reader.get_bits(9) == 0b111111111);
    }

    SECTION("Input interpreted as unsigned") {
      set_stream_bytes(iss, {0xff, 0xff, 0xff, 0xff});
      REQUIRE(bit_reader.get_bits(32) == 0xffffffff);
    }
  }

  SECTION("Read high bit first") {
    SECTION("Reads bits in correct order") {
      set_stream_bytes(iss, {0b10101110});
      REQUIRE(bit_reader.get_bits(4, false) == 0b0111);
    }

    SECTION("Properly handles byte boundary") {
      set_stream_bytes(iss, {0b11111111, 0b00000001});
      REQUIRE(bit_reader.get_bits(9, false) == 0b111111111);
    }

    SECTION("Input interpreted as unsigned") {
      set_stream_bytes(iss, {0xff, 0xff, 0xff, 0xff});
      REQUIRE(bit_reader.get_bits(32, false) == 0xffffffff);
    }
  }
}

TEST_CASE("eof()", "[bit_reader][eof]") {
  std::istringstream iss {};
  bit_io::bit_reader bit_reader {iss};

  SECTION("Reading from empty stream yields EOF") {
    bit_reader.get_bits(1);
    REQUIRE(bit_reader.eof());
  }

  SECTION("Reading from non-empty stream doesn't yield EOF") {
    set_stream_bytes(iss, {0xff});
    REQUIRE(!bit_reader.eof());
  }

  SECTION("Depleting bits in stream yields EOF") {
    set_stream_bytes(iss, {0xff});

    bit_reader.get_bits(8);
    REQUIRE(!bit_reader.eof());

    bit_reader.get_bits(1);
    REQUIRE(bit_reader.eof());
  }
}
