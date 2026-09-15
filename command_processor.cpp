#include "command_processor.h"

namespace novikov {

CommandProcessor::CommandProcessor(std::istream &in, std::ostream &out,
                                    std::ostream &err)
  : in_(in),
    out_(out),
    err_(err)
{
}

bool CommandProcessor::processLine(const std::string &line)
{
  (void)line;
  return true;
}

int CommandProcessor::run()
{
  return 0;
}

}
