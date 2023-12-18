#include "gzip/gzip_reader.hpp"
#include "lzss/lzss_code_tables.hpp"
#include "prefix_codes/prefix_code_decoder.hpp"
#include "prefix_codes/prefix_code_types.hpp"

#include <array>
#include <cassert>
#include <deque>
#include <string>

namespace gzip {

GzipReader::GzipReader(std::istream &input, std::ostream &output)
  : input_{ input }, output_{ output }, bit_reader_{ input }
{
  compute_fixed_code_tables();
}

void GzipReader::read()
{
  read_header();
  read_deflate_bit_stream();
  read_footer();
}

void GzipReader::read_header()
{
  const unsigned int MAGIC1{ 0x1f };
  const unsigned int MAGIC2{ 0x8b };
  const unsigned int COMPRESSION_METHOD{ 0x08 };

  if (bit_reader_.get_bits(8) != MAGIC1 || bit_reader_.get_bits(8) != MAGIC2) {
    throw GzipReaderError("Invalid gzip file.");
  }
  if (bit_reader_.get_bits(8) != COMPRESSION_METHOD) {
    throw GzipReaderError("Invalid compression method.");
  }

  // Read the rest of the header. We currently don't do anything with this data.
  bit_reader_.get_bits(8);
  bit_reader_.get_bits(32);
  bit_reader_.get_bits(8);
  bit_reader_.get_bits(8);
}

void GzipReader::read_deflate_bit_stream()
{
  while (true) {
    bool is_last_block{ bit_reader_.get_single_bit() };
    auto block_type{ bit_reader_.get_bits(2) };
    if (bit_reader_.eof()) {
      break;
    }

    // XXX: Read block of appropriate type based on value of `block_type`.

    // XXX: Sanity check. Remove later.
    assert(block_type == 2);

    read_block_type_2();

    if (is_last_block) {
      break;
    }
  }
}

void GzipReader::read_footer() {}

void GzipReader::read_block_type_0()
{
  bit_reader_.align_to_byte();

  auto input_size{ bit_reader_.get_bits(16) };
  if (bit_reader_.get_bits(16) != (~input_size & 0xFFFF)) {
    throw GzipReaderError("Invalid block type 0 header.");
  }

  for (unsigned int i{ 0 }; i < input_size; i++) {
    output_.put(bit_reader_.get_bits(8));
  }
}

void GzipReader::read_block_type_1()
{
  while (true) {
    auto ll_code{ fixed_ll_code_decoder_.decode_symbol() };
    if (ll_code == 256) {
      break;
    }

    if (ll_code < 256) {
      output_.put(ll_code);

      history_.push_back(ll_code);
      if (history_.size() > 32768) {
        history_.pop_front();
      }
    } else {
      auto ll_entry{ lzss::code_tables::get_length_entry_by_code(ll_code) };

      unsigned int length{ ll_entry.lower_bound };
      if (ll_entry.extra_bits > 0) {
        length += bit_reader_.get_bits(ll_entry.extra_bits);
      }

      auto distance_code{ fixed_distance_code_decoder_.decode_symbol() };
      auto distance_entry{ lzss::code_tables::get_distance_entry_by_code(distance_code) };

      unsigned int distance{ distance_entry.lower_bound };
      if (distance_entry.extra_bits > 0) {
        distance += bit_reader_.get_bits(distance_entry.extra_bits);
      }

      if (distance > history_.size()) {
        throw GzipReaderError("Got invalid back-reference: (" + std::to_string(length) + ", " + std::to_string(distance)
                              + "), history size is " + std::to_string(history_.size()));
      }

      for (unsigned int i{ 0 }; i < length; i++) {
        unsigned int symbol{ history_.at(history_.size() - distance) };
        output_.put(symbol);

        history_.push_back(symbol);
        if (history_.size() > 32768) {
          history_.pop_front();
        }
      }
    }
  }
}

void GzipReader::read_block_type_2()
{
  auto num_ll_codes{ bit_reader_.get_bits(5) + 257 };
  auto num_distance_codes{ bit_reader_.get_bits(5) + 1 };
  auto num_cl_codes{ bit_reader_.get_bits(4) + 4 };

  const std::array<unsigned int, 19> CL_CODE_LENGTH_ORDER{
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
  };

  prefix_codes::CodeLengthTable cl_code_lengths{};

  for (unsigned int i{ 0 }; i < num_cl_codes; i++) {
    auto code{ CL_CODE_LENGTH_ORDER[i] };
    auto length{ bit_reader_.get_bits(3) };
    cl_code_lengths.insert({ code, length });
  }

  prefix_codes::PrefixCodeDecoder cl_code_decoder{ bit_reader_, cl_code_lengths };
  cl_code_decoder.initialize();

  prefix_codes::CodeLengthTable ll_code_lengths{};
  prefix_codes::CodeLengthTable distance_code_lengths{};

  unsigned int codes_read{ 0 };
  unsigned int last_code;

  while (codes_read < num_ll_codes + num_distance_codes) {
    unsigned int code{ cl_code_decoder.decode_symbol() };

    if (code <= 15) {
      if (codes_read < num_ll_codes) {
        ll_code_lengths.insert({ codes_read, code });
      } else {
        distance_code_lengths.insert({ codes_read - num_ll_codes, code });
      }
      codes_read++;
      last_code = code;
      continue;
    }

    if (codes_read == 0) {
      throw GzipReaderError("Got repeat code " + std::to_string(code) + " as first symbol in LL/distance code table.");
    }

    unsigned int repeat_count;
    if (code == 16) {
      repeat_count = bit_reader_.get_bits(2) + 3;
    } else if (code == 17) {
      repeat_count = bit_reader_.get_bits(3) + 3;
    } else {
      repeat_count = bit_reader_.get_bits(7) + 11;
    }

    for (unsigned int i{ 0 }; i < repeat_count; i++) {
      if (codes_read < num_ll_codes) {
        ll_code_lengths.insert({ codes_read, last_code });
      } else {
        distance_code_lengths.insert({ codes_read - num_ll_codes, last_code });
      }
      codes_read++;
    }
  }

  prefix_codes::PrefixCodeDecoder ll_code_decoder{ bit_reader_, ll_code_lengths };
  ll_code_decoder.initialize();
  prefix_codes::PrefixCodeDecoder distance_code_decoder{ bit_reader_, distance_code_lengths };
  distance_code_decoder.initialize();

  while (true) {
    auto ll_code{ ll_code_decoder.decode_symbol() };
    if (ll_code == 256) {
      break;
    }

    if (ll_code < 256) {
      output_.put(ll_code);

      history_.push_back(ll_code);
      if (history_.size() > 32768) {
        history_.pop_front();
      }
    } else {
      auto ll_entry{ lzss::code_tables::get_length_entry_by_code(ll_code) };

      unsigned int length{ ll_entry.lower_bound };
      if (ll_entry.extra_bits > 0) {
        length += bit_reader_.get_bits(ll_entry.extra_bits);
      }

      auto distance_code{ distance_code_decoder.decode_symbol() };
      auto distance_entry{ lzss::code_tables::get_distance_entry_by_code(distance_code) };

      unsigned int distance{ distance_entry.lower_bound };
      if (distance_entry.extra_bits > 0) {
        distance += bit_reader_.get_bits(distance_entry.extra_bits);
      }

      if (distance > history_.size()) {
        throw GzipReaderError("Got invalid back-reference: (" + std::to_string(length) + ", " + std::to_string(distance)
                              + "), history size is " + std::to_string(history_.size()));
      }

      for (unsigned int i{ 0 }; i < length; i++) {
        unsigned int symbol{ history_.at(history_.size() - distance) };
        output_.put(symbol);

        history_.push_back(symbol);
        if (history_.size() > 32768) {
          history_.pop_front();
        }
      }
    }
  }
}

void GzipReader::compute_fixed_code_tables()
{
  for (unsigned int code{ 0 }; code <= 143; code++) {
    fixed_ll_code_lengths_.insert({ code, 8 });
  }
  for (unsigned int code{ 144 }; code <= 255; code++) {
    fixed_ll_code_lengths_.insert({ code, 9 });
  }
  for (unsigned int code{ 256 }; code <= 279; code++) {
    fixed_ll_code_lengths_.insert({ code, 7 });
  }
  for (unsigned int code{ 280 }; code <= 287; code++) {
    fixed_ll_code_lengths_.insert({ code, 8 });
  }

  for (unsigned int code{ 0 }; code <= 29; code++) {
    fixed_distance_code_lengths_.insert({ code, 5 });
  }

  fixed_ll_code_decoder_.initialize();
  fixed_distance_code_decoder_.initialize();
}

}  // namespace gzip
