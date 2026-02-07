#include "identity.hpp"
#include <ostream>

namespace alekseev
{
  std::ostream & print_identity(std::ostream & out)
  {
    return out << "alekseev.platon";
  }
}
