#ifndef NOVIKOV_COMMAND_PROCESSOR_H
#define NOVIKOV_COMMAND_PROCESSOR_H

#include <istream>
#include <map>
#include <ostream>
#include <string>

#include "interval_set.h"

namespace novikov {

class CommandProcessor
{
public:
  CommandProcessor(std::istream& in, std::ostream& out, std::ostream& err);

  int run();

private:
  std::istream& in_;
  std::ostream& out_;
  std::ostream& err_;
  std::map<std::string, IntervalSet> trees_;

  bool processLine(const std::string& line);

  void handleCreate(std::istream& args);
  void handleAdd(std::istream& args);
  void handleRemove(std::istream& args);
  void handleHas(std::istream& args);
  void handleLength(std::istream& args);
  void handleShow(std::istream& args);
  void handleUnion(std::istream& args);
  void handleIntersect(std::istream& args);

  IntervalSet& getTree(const std::string& name);
  const IntervalSet& getTree(const std::string& name) const;
  void printIntervals(const IntervalSet& tree) const;
};

}

#endif
