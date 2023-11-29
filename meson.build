project('gzip', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++20'])

include_dir = include_directories('include')

catch2_dep = dependency('catch2-with-main')

# --- Bit I/O Tests ---

bit_reader_test = executable(
  'bit_reader_test',
  sources : [
    'test/bit_io/bit_reader_test.cpp',
    'src/bit_io/bit_reader.cpp'
  ],
  include_directories : include_dir,
  dependencies : catch2_dep
)

bit_writer_test = executable(
  'bit_writer_test',
  sources : [
    'test/bit_io/bit_writer_test.cpp',
    'src/bit_io/bit_writer.cpp'
  ],
  include_directories : include_dir,
  dependencies : catch2_dep
)

bit_recovery_test = executable(
  'bit_recovery_test',
  sources : [
    'test/bit_io/bit_recovery_test.cpp',
    'src/bit_io/bit_writer.cpp',
    'src/bit_io/bit_reader.cpp'
  ],
  include_directories : include_dir,
  dependencies : catch2_dep
)

test('bit_recovery_test', bit_recovery_test)
test('bit_reader_test', bit_reader_test)
test('bit_writer_test', bit_writer_test)

# --- LZSS Tests ---

lzss_encoder_test = executable(
  'lzss_encoder_test',
  sources : [
    'test/lzss/lzss_encoder_test.cpp',
    'src/lzss/lzss_encoder.cpp',
    'src/lzss/lzss_symbol.cpp',
    'src/lzss/lzss_string_table.cpp',
    'src/lzss/lzss_code_tables.cpp'
  ],
  include_directories : include_dir,
  dependencies : catch2_dep
)

test('lzss_encoder_test', lzss_encoder_test)

# --- Prefix Encoder / Decoder Tests ---

prefix_code_encoder_test = executable(
  'prefix_code_encoder_test',
  sources : [
    'test/prefix_codes/prefix_code_encoder_test.cpp',
    'src/prefix_codes/prefix_code_encoder.cpp'
  ],
  include_directories : include_dir,
  dependencies : catch2_dep
)

test('prefix_code_encoder_test', prefix_code_encoder_test)

# --- Command-Line Tools ---

executable(
  'gzip',
  sources: [
    'src/cli/gzip.cpp',
    'src/bit_io/bit_writer.cpp',
    'src/prefix_codes/fixed_code_table.cpp',
    'src/prefix_codes/prefix_code_encoder.cpp',
    'src/lzss/lzss_encoder.cpp',
    'src/lzss/lzss_string_table.cpp',
    'src/lzss/lzss_symbol.cpp',
    'src/lzss/lzss_code_tables.cpp',
    'src/gzip/gzip_writer.cpp'
  ],
  include_directories : include_dir
)

executable(
  'gunzip',
  sources: [
    'src/cli/gunzip.cpp'
  ],
  include_directories : include_dir
)
