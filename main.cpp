#include <exception>
#include <iostream>

#include "command_processor.h"

int main()
{
  try {
    novikov::CommandProcessor processor(std::cin, std::cout, std::cerr);
    return processor.run();
  } catch (const std::exception& error) {
    std::cerr << "Internal error: " << error.what() << '\n';
    return 2;
  }
}
