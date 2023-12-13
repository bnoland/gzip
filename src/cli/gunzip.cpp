#include "gzip/gzip_reader.hpp"

#include <cstdlib>
#include <iostream>

int main() {
  try {
    gzip::GzipReader {std::cin, std::cout}.read();
  } catch (const gzip::GzipReaderError& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    std::exit(1);
  }
}
