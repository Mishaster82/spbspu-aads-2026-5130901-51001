#include "command_processor.h"

#include <sstream>
#include <stdexcept>
#include <string>

namespace novikov {

namespace {

std::string readToken(std::istream& args, const std::string& commandName)
{
  std::string token;

  if (!(args >> token)) {
    throw std::runtime_error("missing argument for command '"
                             + commandName + "'");
  }
  return token;
}

int readInt(std::istream& args, const std::string& commandName)
{
  const std::string token = readToken(args, commandName);
  std::size_t consumed = 0;
  int value = 0;

  try {
    value = std::stoi(token, &consumed);
  } catch (const std::exception&) {
    throw std::runtime_error("expected integer, got '" + token + "'");
  }

  if (consumed != token.size()) {
    throw std::runtime_error("expected integer, got '" + token + "'");
  }
  return value;
}

}

CommandProcessor::CommandProcessor(std::istream& in, std::ostream& out,
                                    std::ostream& err)
  : in_(in),
    out_(out),
    err_(err),
    trees_()
{
}

IntervalSet& CommandProcessor::getTree(const std::string& name)
{
  const auto found = trees_.find(name);

  if (found == trees_.end()) {
    throw std::runtime_error("unknown tree '" + name + "'");
  }
  return found->second;
}

const IntervalSet& CommandProcessor::getTree(const std::string& name) const
{
  const auto found = trees_.find(name);

  if (found == trees_.end()) {
    throw std::runtime_error("unknown tree '" + name + "'");
  }
  return found->second;
}

void CommandProcessor::printIntervals(const IntervalSet& tree) const
{
  const std::vector<IntervalSet::Interval> result = tree.getIntervals();
  bool first = true;

  for (const auto& interval : result) {
    if (!first) {
      out_ << ' ';
    }
    out_ << '[' << interval.first << ',' << interval.second << ']';
    first = false;
  }
  out_ << '\n';
}

void CommandProcessor::handleCreate(std::istream& args)
{
  const std::string name = readToken(args, "create");
  const int rangeSize = readInt(args, "create");

  if (rangeSize <= 0) {
    throw std::runtime_error("range size must be positive");
  }
  if (trees_.count(name) != 0) {
    throw std::runtime_error("tree already exists: '" + name + "'");
  }

  trees_.emplace(name, IntervalSet(rangeSize));
  out_ << "OK\n";
}

void CommandProcessor::handleAdd(std::istream& args)
{
  const std::string name = readToken(args, "add");
  const int left = readInt(args, "add");
  const int right = readInt(args, "add");

  IntervalSet& tree = getTree(name);

  tree.add(left, right);
}

void CommandProcessor::handleRemove(std::istream& args)
{
  const std::string name = readToken(args, "remove");
  const int left = readInt(args, "remove");
  const int right = readInt(args, "remove");

  IntervalSet& tree = getTree(name);

  tree.remove(left, right);
}

void CommandProcessor::handleHas(std::istream& args)
{
  const std::string name = readToken(args, "has");
  const int point = readInt(args, "has");

  const IntervalSet& tree = getTree(name);

  out_ << (tree.has(point) ? "YES" : "NO") << '\n';
}

void CommandProcessor::handleLength(std::istream& args)
{
  const std::string name = readToken(args, "length");
  const IntervalSet& tree = getTree(name);

  out_ << tree.getLength() << '\n';
}

void CommandProcessor::handleShow(std::istream& args)
{
  const std::string name = readToken(args, "show");
  const IntervalSet& tree = getTree(name);

  printIntervals(tree);
}

void CommandProcessor::handleUnion(std::istream& args)
{
  const std::string newName = readToken(args, "union");
  const std::string firstName = readToken(args, "union");
  const std::string secondName = readToken(args, "union");

  const IntervalSet& first = getTree(firstName);
  const IntervalSet& second = getTree(secondName);

  trees_.erase(newName);
  trees_.emplace(newName, IntervalSet::unite(first, second));
}

void CommandProcessor::handleIntersect(std::istream& args)
{
  const std::string newName = readToken(args, "intersect");
  const std::string firstName = readToken(args, "intersect");
  const std::string secondName = readToken(args, "intersect");

  const IntervalSet& first = getTree(firstName);
  const IntervalSet& second = getTree(secondName);

  trees_.erase(newName);
  trees_.emplace(newName, IntervalSet::intersect(first, second));
}

bool CommandProcessor::processLine(const std::string& line)
{
  std::istringstream args(line);
  std::string command;

  if (!(args >> command)) {
    return true;
  }

  if (command == "create") {
    handleCreate(args);
  } else if (command == "add") {
    handleAdd(args);
  } else if (command == "remove") {
    handleRemove(args);
  } else if (command == "has") {
    handleHas(args);
  } else if (command == "length") {
    handleLength(args);
  } else if (command == "show") {
    handleShow(args);
  } else if (command == "union") {
    handleUnion(args);
  } else if (command == "intersect") {
    handleIntersect(args);
  } else {
    throw std::runtime_error("unknown command '" + command + "'");
  }

  return true;
}

int CommandProcessor::run()
{
  std::string line;

  while (std::getline(in_, line)) {
    try {
      processLine(line);
    } catch (const std::exception& error) {
      err_ << "Error: " << error.what() << '\n';
      return 1;
    }
  }
  return 0;
}

}
