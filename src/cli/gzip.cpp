#include "gzip/gzip_writer.hpp"

#include <iostream>

int main()
{
  gzip::GzipWriter{ std::cin, std::cout }.write();
}
