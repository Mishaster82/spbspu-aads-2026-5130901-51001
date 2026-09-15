#ifndef NOVIKOV_COMMAND_PROCESSOR_H
#define NOVIKOV_COMMAND_PROCESSOR_H

#include <istream>
#include <ostream>
#include <string>

namespace novikov {

class CommandProcessor
{
public:
  CommandProcessor(std::istream &in, std::ostream &out, std::ostream &err);

  int run();

private:
  std::istream &in_;
  std::ostream &out_;
  std::ostream &err_;

  bool processLine(const std::string &line);
};

}

#endif
