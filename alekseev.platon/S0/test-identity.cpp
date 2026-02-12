#define BOOST_TEST_MODULE S0
#include <boost/test/included/unit_test.hpp>
#include "identity.hpp"
#include <sstream>

BOOST_AUTO_TEST_CASE(prints_identity)
{
  std::ostringstream out;
  alekseev::print_identity(out);
  BOOST_TEST(out.str() == "alekseev.platon");
}
