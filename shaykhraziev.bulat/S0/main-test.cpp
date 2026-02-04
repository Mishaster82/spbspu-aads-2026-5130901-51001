#define BOOST_TEST_MODULE testing credentials
#include <boost/test/included/unit_test.hpp>
#include "creds.hpp"
#include <sstream>

BOOST_AUTO_TEST_CASE(test_creds)
{
  std::ostringstream out;
  shaykhraziev::print_creds(out);
  BOOST_TEST(out.str() == "shaykhraziev.bulat");
}
