#include "bit_io/bit_writer.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <initializer_list>
#include <sstream>
#include <string>
#include <vector>

bool stream_bytes_equal(std::ostringstream &oss, std::initializer_list<uint8_t> bytes)
{
  std::string content = oss.str();
  std::vector<uint8_t> buffer{ content.begin(), content.end() };
  return buffer == std::vector<uint8_t>{ bytes };
}

TEST_CASE("put_single_bit()", "[bit_writer][put_single_bit]")
{
  std::ostringstream oss{};
  bit_io::BitWriter bit_writer{ oss };

  SECTION("Short bit strings are padded with zeros")
  {
    bit_writer.put_single_bit(true);
    bit_writer.put_single_bit(false);
    bit_writer.put_single_bit(true);
    bit_writer.put_single_bit(false);
    bit_writer.finish();

    REQUIRE(stream_bytes_equal(oss, { 0b00000101 }));
  }

  SECTION("Properly handles byte boundary")
  {
    for (int i = 0; i < 9; i++) {
      bit_writer.put_single_bit(true);
    }

    bit_writer.finish();

    REQUIRE(stream_bytes_equal(oss, { 0b11111111, 0b00000001 }));
  }
}

TEST_CASE("put_bits()", "[bit_writer][put_bits]")
{
  std::ostringstream oss{};
  bit_io::BitWriter bit_writer{ oss };

  SECTION("Write low bit first")
  {
    SECTION("Bits written in correct order")
    {
      bit_writer.put_bits(0b1011, 4);
      bit_writer.finish();

      REQUIRE(stream_bytes_equal(oss, { 0b00001011 }));
    }

    SECTION("Properly handles byte boundary")
    {
      bit_writer.put_bits(0b111111111, 9);
      bit_writer.finish();

      REQUIRE(stream_bytes_equal(oss, { 0b11111111, 0b00000001 }));
    }

    SECTION("Output interpreted as unsigned")
    {
      bit_writer.put_bits(0xffffffff, 32);
      bit_writer.finish();

      REQUIRE(stream_bytes_equal(oss, { 0xff, 0xff, 0xff, 0xff }));
    }
  }

  SECTION("Write high bit first")
  {
    SECTION("Bits written in correct order")
    {
      bit_writer.put_bits(0b1011, 4, false);
      bit_writer.finish();

      REQUIRE(stream_bytes_equal(oss, { 0b00001101 }));
    }

    SECTION("Properly handles byte boundary")
    {
      bit_writer.put_bits(0b111111111, 9, true);
      bit_writer.finish();

      REQUIRE(stream_bytes_equal(oss, { 0b11111111, 0b00000001 }));
    }

    SECTION("Output interpreted as unsigned")
    {
      bit_writer.put_bits(0xffffffff, 32, true);
      bit_writer.finish();

      REQUIRE(stream_bytes_equal(oss, { 0xff, 0xff, 0xff, 0xff }));
    }
  }
}
