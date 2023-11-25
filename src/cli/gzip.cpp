#include "gzip/gzip_writer.hpp"

#include <iostream>

int main() {
  gzip::gzip_writer {std::cin, std::cout}.write();
}
