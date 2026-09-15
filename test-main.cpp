#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "command_processor.h"
#include "interval_set.h"

namespace {

int failures = 0;

void expectTrue(bool condition, const std::string& what)
{
  if (!condition) {
    ++failures;
    std::cerr << "FAIL: " << what << '\n';
  }
}

void expectInt(int actual, int expected, const std::string& what)
{
  if (actual != expected) {
    ++failures;
    std::cerr << "FAIL: " << what
              << ": expected " << expected
              << ", got " << actual << '\n';
  }
}

void expect64(std::int64_t actual, std::int64_t expected,
              const std::string& what)
{
  if (actual != expected) {
    ++failures;
    std::cerr << "FAIL: " << what
              << ": expected " << expected
              << ", got " << actual << '\n';
  }
}

void expectStr(const std::string& actual, const std::string& expected,
               const std::string& what)
{
  if (actual != expected) {
    ++failures;
    std::cerr << "FAIL: " << what << '\n'
              << "  expected:\n" << expected
              << "  actual:\n" << actual;
  }
}

struct ProcessorResult
{
  int code;
  std::string out;
  std::string err;
};

ProcessorResult runProcessor(const std::string& input)
{
  std::istringstream in(input);
  std::ostringstream out;
  std::ostringstream err;

  novikov::CommandProcessor processor(in, out, err);
  const int code = processor.run();

  ProcessorResult result;
  result.code = code;
  result.out = out.str();
  result.err = err.str();
  return result;
}

void testEmptyTreeHas()
{
  const novikov::IntervalSet tree(100);
  expectTrue(!tree.has(0), "empty: !has(0)");
  expectTrue(!tree.has(50), "empty: !has(50)");
  expectTrue(!tree.has(99), "empty: !has(99)");
}

void testEmptyTreeLengthAndIntervals()
{
  const novikov::IntervalSet tree(100);
  expect64(tree.getLength(), 0, "empty: length 0");
  expectInt(static_cast<int>(tree.getIntervals().size()), 0,
            "empty: no intervals");
  expectInt(tree.getRangeSize(), 100, "empty: range size 100");
}

void testAddSingleInterval()
{
  novikov::IntervalSet tree(100);
  tree.add(10, 20);

  expectTrue(tree.has(10), "single: has(10)");
  expectTrue(tree.has(15), "single: has(15)");
  expectTrue(tree.has(20), "single: has(20)");
  expectTrue(!tree.has(9), "single: !has(9)");
  expectTrue(!tree.has(21), "single: !has(21)");
  expect64(tree.getLength(), 10, "single: length 10");

  const std::vector<novikov::IntervalSet::Interval> intervals =
      tree.getIntervals();
  expectInt(static_cast<int>(intervals.size()), 1, "single: 1 interval");
  expectInt(intervals[0].first, 10, "single: first");
  expectInt(intervals[0].second, 20, "single: second");
}

void testAddMergesOverlapping()
{
  novikov::IntervalSet tree(100);
  tree.add(10, 20);
  tree.add(15, 28);

  const std::vector<novikov::IntervalSet::Interval> intervals =
      tree.getIntervals();
  expectInt(static_cast<int>(intervals.size()), 1,
            "merge overlap: 1 interval");
  expectInt(intervals[0].first, 10, "merge overlap: first");
  expectInt(intervals[0].second, 28, "merge overlap: second");
}

void testAddMergesAdjacent()
{
  novikov::IntervalSet tree(100);
  tree.add(10, 20);
  tree.add(21, 30);

  const std::vector<novikov::IntervalSet::Interval> intervals =
      tree.getIntervals();
  expectInt(static_cast<int>(intervals.size()), 1,
            "merge adjacent: 1 interval");
  expectInt(intervals[0].first, 10, "merge adjacent: first");
  expectInt(intervals[0].second, 30, "merge adjacent: second");
}

void testAddKeepsSeparate()
{
  novikov::IntervalSet tree(100);
  tree.add(10, 14);
  tree.add(19, 30);

  const std::vector<novikov::IntervalSet::Interval> intervals =
      tree.getIntervals();
  expectInt(static_cast<int>(intervals.size()), 2,
            "separate: 2 intervals");
  expectInt(intervals[0].first, 10, "separate: f1");
  expectInt(intervals[0].second, 14, "separate: s1");
  expectInt(intervals[1].first, 19, "separate: f2");
  expectInt(intervals[1].second, 30, "separate: s2");
}

void testRemoveSplitsInterval()
{
  novikov::IntervalSet tree(100);
  tree.add(10, 30);
  tree.remove(15, 18);

  expectTrue(!tree.has(15), "remove: !has(15)");
  expectTrue(!tree.has(17), "remove: !has(17)");
  expectTrue(tree.has(14), "remove: has(14)");
  expectTrue(tree.has(19), "remove: has(19)");
  expect64(tree.getLength(), 15, "remove: length 15");

  const std::vector<novikov::IntervalSet::Interval> intervals =
      tree.getIntervals();
  expectInt(static_cast<int>(intervals.size()), 2, "remove: 2 intervals");
  expectInt(intervals[0].first, 10, "remove: left.first");
  expectInt(intervals[0].second, 14, "remove: left.second");
  expectInt(intervals[1].first, 19, "remove: right.first");
  expectInt(intervals[1].second, 30, "remove: right.second");
}

void testRemoveAll()
{
  novikov::IntervalSet tree(100);
  tree.add(10, 20);
  tree.remove(10, 20);

  expect64(tree.getLength(), 0, "remove all: length 0");
  expectTrue(tree.getIntervals().empty(), "remove all: empty");
}

void testRemoveFromEmpty()
{
  novikov::IntervalSet tree(100);
  tree.remove(10, 20);

  expect64(tree.getLength(), 0, "remove from empty: length 0");
  expectTrue(tree.getIntervals().empty(), "remove from empty: empty");
}

void testHasOutOfRange()
{
  novikov::IntervalSet tree(100);
  tree.add(0, 99);

  expectTrue(tree.has(0), "range: has(0)");
  expectTrue(tree.has(99), "range: has(99)");
  expectTrue(!tree.has(-1), "range: !has(-1)");
  expectTrue(!tree.has(100), "range: !has(100)");
}

void testAddOutOfRangeThrows()
{
  novikov::IntervalSet tree(100);

  bool thrown = false;
  try {
    tree.add(-1, 5);
  } catch (const std::out_of_range&) {
    thrown = true;
  }
  expectTrue(thrown, "add(-1,5) throws");

  thrown = false;
  try {
    tree.add(0, 100);
  } catch (const std::out_of_range&) {
    thrown = true;
  }
  expectTrue(thrown, "add(0,100) throws");

  thrown = false;
  try {
    tree.add(5, 4);
  } catch (const std::out_of_range&) {
    thrown = true;
  }
  expectTrue(thrown, "add(5,4) throws");

  thrown = false;
  try {
    tree.remove(-1, 5);
  } catch (const std::out_of_range&) {
    thrown = true;
  }
  expectTrue(thrown, "remove(-1,5) throws");
}

void testLengthExampleFromTask()
{
  novikov::IntervalSet tree(100);
  tree.add(10, 20);
  tree.add(25, 30);
  tree.add(15, 28);
  tree.remove(15, 18);

  expect64(tree.getLength(), 15, "example: length 15");
}

void testUnite()
{
  novikov::IntervalSet left(100);
  left.add(10, 14);
  left.add(19, 30);

  novikov::IntervalSet right(100);
  right.add(40, 50);

  const novikov::IntervalSet result =
      novikov::IntervalSet::unite(left, right);

  expectInt(result.getRangeSize(), 100, "unite: range");
  expectInt(static_cast<int>(result.getIntervals().size()), 3,
            "unite: 3 intervals");
  expect64(result.getLength(), 20, "unite: length 20");
}

void testUniteDifferentRangeSizes()
{
  novikov::IntervalSet small(50);
  small.add(0, 10);

  novikov::IntervalSet big(200);
  big.add(100, 150);

  const novikov::IntervalSet result =
      novikov::IntervalSet::unite(small, big);

  expectInt(result.getRangeSize(), 200, "unite diff: max range");
  expectTrue(result.has(0), "unite diff: has(0)");
  expectTrue(result.has(150), "unite diff: has(150)");
}

void testIntersect()
{
  novikov::IntervalSet left(100);
  left.add(10, 14);
  left.add(19, 30);

  novikov::IntervalSet right(100);
  right.add(12, 22);

  const novikov::IntervalSet result =
      novikov::IntervalSet::intersect(left, right);

  const std::vector<novikov::IntervalSet::Interval> intervals =
      result.getIntervals();
  expectInt(static_cast<int>(intervals.size()), 2,
            "intersect: 2 intervals");
  expectInt(intervals[0].first, 12, "intersect: f1");
  expectInt(intervals[0].second, 14, "intersect: s1");
  expectInt(intervals[1].first, 19, "intersect: f2");
  expectInt(intervals[1].second, 22, "intersect: s2");
}

void testIntersectEmpty()
{
  novikov::IntervalSet left(100);
  left.add(10, 20);

  novikov::IntervalSet right(100);
  right.add(50, 60);

  const novikov::IntervalSet result =
      novikov::IntervalSet::intersect(left, right);

  expect64(result.getLength(), 0, "intersect empty: length 0");
  expectTrue(result.getIntervals().empty(), "intersect empty: empty");
}

void testCopyAndMove()
{
  novikov::IntervalSet original(100);
  original.add(10, 20);
  original.add(30, 40);

  novikov::IntervalSet copy(original);
  expectTrue(copy.has(10), "copy: has(10)");
  expectTrue(copy.has(30), "copy: has(30)");
  expect64(copy.getLength(), 20, "copy: length 20");

  original.remove(10, 20);
  expectTrue(!original.has(10), "copy indep: original changed");
  expectTrue(copy.has(10), "copy indep: copy unchanged");

  novikov::IntervalSet moved(std::move(copy));
  expectTrue(moved.has(10), "move: has(10)");
  expectTrue(moved.has(30), "move: has(30)");
}

void testSaveLoadRoundTrip()
{
  const std::string filename = "test-roundtrip.tmp";
  std::remove(filename.c_str());

  {
    novikov::IntervalSet tree(100);
    tree.add(10, 14);
    tree.add(19, 30);
    expectTrue(tree.save(filename), "save: true");
  }

  {
    novikov::IntervalSet tree(1);
    expectTrue(tree.load(filename), "load: true");
    expectInt(tree.getRangeSize(), 100, "load: range 100");
    expectTrue(tree.has(10), "load: has(10)");
    expectTrue(!tree.has(15), "load: !has(15)");
    expectTrue(tree.has(30), "load: has(30)");
    expect64(tree.getLength(), 15, "load: length 15");
  }

  std::remove(filename.c_str());
}

void testLoadDanglingPair()
{
  const std::string filename = "test-dangling.tmp";
  std::remove(filename.c_str());

  {
    std::ofstream out(filename);
    out << "100\n";
    out << "10\n";
  }

  novikov::IntervalSet tree(1);
  expectTrue(!tree.load(filename), "load dangling: false");

  std::remove(filename.c_str());
}

void testLoadMalformedHeader()
{
  const std::string filename = "test-bad-header.tmp";
  std::remove(filename.c_str());

  {
    std::ofstream out(filename);
    out << "-5\n";
  }

  novikov::IntervalSet tree(1);
  expectTrue(!tree.load(filename), "load bad header: false");

  std::remove(filename.c_str());
}

void testLoadIntervalOutOfRange()
{
  const std::string filename = "test-bad-interval.tmp";
  std::remove(filename.c_str());

  {
    std::ofstream out(filename);
    out << "100\n";
    out << "10 150\n";
  }

  novikov::IntervalSet tree(1);
  expectTrue(!tree.load(filename), "load bad interval: false");

  std::remove(filename.c_str());
}

void testLoadMissingFile()
{
  novikov::IntervalSet tree(1);
  expectTrue(!tree.load("no-such-file.tmp"), "load missing: false");
}

void testScenarioFromTask()
{
  const std::string input =
      "create A 100\n"
      "add A 10 20\n"
      "add A 25 30\n"
      "add A 15 28\n"
      "show A\n"
      "remove A 15 18\n"
      "show A\n"
      "has A 12\n"
      "has A 17\n"
      "length A\n"
      "create B 100\n"
      "add B 40 50\n"
      "union C A B\n"
      "show C\n"
      "create D 100\n"
      "add D 12 22\n"
      "intersect E A D\n"
      "show E\n";

  const std::string expected =
      "OK\n"
      "[10,30]\n"
      "[10,14] [19,30]\n"
      "YES\n"
      "NO\n"
      "15\n"
      "[10,14] [19,30] [40,50]\n"
      "[12,14] [19,22]\n";

  const ProcessorResult result = runProcessor(input);
  expectInt(result.code, 0, "scenario: exit 0");
  expectStr(result.out, expected, "scenario: output");
}

void testShowEmptyTree()
{
  const ProcessorResult result = runProcessor(
      "create A 100\n"
      "show A\n");

  expectInt(result.code, 0, "show empty: exit 0");
  expectStr(result.out, "OK\n{}\n", "show empty: output");
}

void testCreateDuplicateFails()
{
  const ProcessorResult result = runProcessor(
      "create A 100\n"
      "create A 200\n");

  expectInt(result.code, 1, "duplicate create: exit 1");
  expectTrue(!result.err.empty(), "duplicate create: message");
}

void testCreateInvalidRangeFails()
{
  const ProcessorResult result = runProcessor("create A 0\n");
  expectInt(result.code, 1, "create 0: exit 1");
}

void testUnknownTreeFails()
{
  const ProcessorResult result = runProcessor("show Missing\n");
  expectInt(result.code, 1, "unknown tree: exit 1");
}

void testUnknownCommandFails()
{
  const ProcessorResult result = runProcessor("flibberflob A 10\n");
  expectInt(result.code, 1, "unknown command: exit 1");
}

void testAddOutOfRangeFails()
{
  const ProcessorResult result = runProcessor(
      "create A 100\n"
      "add A -1 5\n");

  expectInt(result.code, 1, "add out of range: exit 1");
}

void testHasOutOfRangeInCommand()
{
  const ProcessorResult result = runProcessor(
      "create A 100\n"
      "has A 500\n");

  expectInt(result.code, 0, "has out of range: exit 0");
  expectStr(result.out, "OK\nNO\n", "has out of range: NO");
}

void testInvalidIntegerFails()
{
  const ProcessorResult result = runProcessor("create A abc\n");
  expectInt(result.code, 1, "invalid integer: exit 1");
}

void testMissingArgumentFails()
{
  const ProcessorResult result = runProcessor("add A 10\n");
  expectInt(result.code, 1, "missing argument: exit 1");
}

void testEmptyLinesIgnored()
{
  const ProcessorResult result = runProcessor(
      "create A 100\n"
      "\n"
      "add A 10 20\n"
      "\n"
      "show A\n");

  expectInt(result.code, 0, "empty lines: exit 0");
  expectStr(result.out, "OK\n[10,20]\n", "empty lines: output");
}

void testUnionAndIntersectSameRange()
{
  const ProcessorResult result = runProcessor(
      "create A 100\n"
      "add A 10 20\n"
      "create B 100\n"
      "add B 15 25\n"
      "union C A B\n"
      "show C\n"
      "intersect D A B\n"
      "show D\n");

  expectInt(result.code, 0, "union/intersect: exit 0");
  expectStr(result.out, "OK\nOK\n[10,25]\n[15,20]\n",
            "union/intersect: output");
}

void testSaveAndLoadScenario()
{
  const std::string filename = "test-save-load.tmp";
  std::remove(filename.c_str());

  {
    const ProcessorResult result = runProcessor(
        "create A 100\n"
        "add A 10 20\n"
        "save A " + filename + "\n");

    expectInt(result.code, 0, "save: exit 0");
    expectStr(result.out, "OK\n", "save: only OK");
  }

  {
    const ProcessorResult result = runProcessor(
        "load B " + filename + "\n"
        "show B\n");

    expectInt(result.code, 0, "load: exit 0");
    expectStr(result.out, "OK\n[10,20]\n", "load: output");
  }

  std::remove(filename.c_str());
}

void testSaveOverwriteDeclined()
{
  const std::string filename = "test-overwrite-n.tmp";
  std::remove(filename.c_str());

  runProcessor(
      "create A 100\n"
      "add A 10 20\n"
      "save A " + filename + "\n");

  {
    const ProcessorResult result = runProcessor(
        "create B 100\n"
        "add B 50 60\n"
        "save B " + filename + "\n"
        "n\n");

    expectInt(result.code, 0, "overwrite n: exit 0");
    expectTrue(result.out.find("Overwrite?") != std::string::npos,
               "overwrite n: prompt");
    expectTrue(result.out.find("Cancelled") != std::string::npos,
               "overwrite n: cancelled");
  }

  {
    const ProcessorResult result = runProcessor(
        "load X " + filename + "\n"
        "show X\n");

    expectStr(result.out, "OK\n[10,20]\n", "overwrite n: preserved");
  }

  std::remove(filename.c_str());
}

void testSaveOverwriteConfirmed()
{
  const std::string filename = "test-overwrite-y.tmp";
  std::remove(filename.c_str());

  runProcessor(
      "create A 100\n"
      "add A 10 20\n"
      "save A " + filename + "\n");

  {
    const ProcessorResult result = runProcessor(
        "create B 100\n"
        "add B 50 60\n"
        "save B " + filename + "\n"
        "y\n");

    expectInt(result.code, 0, "overwrite y: exit 0");
    expectTrue(result.out.find("Overwrite?") != std::string::npos,
               "overwrite y: prompt");
  }

  {
    const ProcessorResult result = runProcessor(
        "load X " + filename + "\n"
        "show X\n");

    expectStr(result.out, "OK\n[50,60]\n", "overwrite y: new content");
  }

  std::remove(filename.c_str());
}

void testLoadMissingFileInCommand()
{
  const ProcessorResult result = runProcessor(
      "load A no-such-file.tmp\n");

  expectInt(result.code, 1, "load missing: exit 1");
}

}

int main()
{
  testEmptyTreeHas();
  testEmptyTreeLengthAndIntervals();
  testAddSingleInterval();
  testAddMergesOverlapping();
  testAddMergesAdjacent();
  testAddKeepsSeparate();
  testRemoveSplitsInterval();
  testRemoveAll();
  testRemoveFromEmpty();
  testHasOutOfRange();
  testAddOutOfRangeThrows();
  testLengthExampleFromTask();
  testUnite();
  testUniteDifferentRangeSizes();
  testIntersect();
  testIntersectEmpty();
  testCopyAndMove();
  testSaveLoadRoundTrip();
  testLoadDanglingPair();
  testLoadMalformedHeader();
  testLoadIntervalOutOfRange();
  testLoadMissingFile();

  testScenarioFromTask();
  testShowEmptyTree();
  testCreateDuplicateFails();
  testCreateInvalidRangeFails();
  testUnknownTreeFails();
  testUnknownCommandFails();
  testAddOutOfRangeFails();
  testHasOutOfRangeInCommand();
  testInvalidIntegerFails();
  testMissingArgumentFails();
  testEmptyLinesIgnored();
  testUnionAndIntersectSameRange();
  testSaveAndLoadScenario();
  testSaveOverwriteDeclined();
  testSaveOverwriteConfirmed();
  testLoadMissingFileInCommand();

  if (failures == 0) {
    std::cerr << "All tests passed\n";
    return 0;
  }

  std::cerr << "Failures: " << failures << '\n';
  return 1;
}
