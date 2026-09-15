#include <cstdint>
#include <cstdio>
#include <iostream>
#include <sstream>

#include "command_processor.h"
#include "interval_set.h"

namespace {

int failures = 0;
bool verbose = false;

void logTest(const std::string& name)
{
  if (verbose) {
    std::cout << "[ RUN  ] " << name << '\n';
  }
}

void expectTrue(bool condition, const std::string& what)
{
  if (!condition) {
    ++failures;
    std::cout << "  [FAIL] " << what << '\n';
  }
}

void expectInt(int actual, int expected, const std::string& what)
{
  if (actual != expected) {
    ++failures;
    std::cout << "  [FAIL] " << what
              << ": expected " << expected
              << ", got " << actual << '\n';
  }
}

void expect64(std::int64_t actual, std::int64_t expected,
              const std::string& what)
{
  if (actual != expected) {
    ++failures;
    std::cout << "  [FAIL] " << what
              << ": expected " << expected
              << ", got " << actual << '\n';
  }
}

void expectStr(const std::string& actual, const std::string& expected,
               const std::string& what)
{
  if (actual != expected) {
    ++failures;
    std::cout << "  [FAIL] " << what << '\n'
              << "    expected:\n" << expected
              << "    actual:\n" << actual;
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
  const novikov::IntIntervalSet tree(100);
  expectTrue(!tree.has(0), "empty: !has(0)");
  expectTrue(!tree.has(50), "empty: !has(50)");
  expectTrue(!tree.has(99), "empty: !has(99)");
}

void testEmptyTreeLengthAndIntervals()
{
  const novikov::IntIntervalSet tree(100);
  expect64(tree.getLength(), 0, "empty: length 0");
  expectInt(static_cast<int>(tree.getIntervals().size()), 0,
            "empty: no intervals");
  expectInt(tree.getRangeSize(), 100, "empty: range size 100");
}

void testAddSingleInterval()
{
  novikov::IntIntervalSet tree(100);
  tree.add(10, 20);

  expectTrue(tree.has(10), "single: has(10)");
  expectTrue(tree.has(15), "single: has(15)");
  expectTrue(tree.has(20), "single: has(20)");
  expectTrue(!tree.has(9), "single: !has(9)");
  expectTrue(!tree.has(21), "single: !has(21)");
  expect64(tree.getLength(), 10, "single: length 10");

  const std::vector<novikov::IntIntervalSet::Interval> intervals =
      tree.getIntervals();
  expectInt(static_cast<int>(intervals.size()), 1, "single: 1 interval");
  expectInt(intervals[0].first, 10, "single: first");
  expectInt(intervals[0].second, 20, "single: second");
}

void testAddMergesOverlapping()
{
  novikov::IntIntervalSet tree(100);
  tree.add(10, 20);
  tree.add(15, 28);

  const std::vector<novikov::IntIntervalSet::Interval> intervals =
      tree.getIntervals();
  expectInt(static_cast<int>(intervals.size()), 1,
            "merge overlap: 1 interval");
  expectInt(intervals[0].first, 10, "merge overlap: first");
  expectInt(intervals[0].second, 28, "merge overlap: second");
}

void testAddMergesAdjacent()
{
  novikov::IntIntervalSet tree(100);
  tree.add(10, 20);
  tree.add(21, 30);

  const std::vector<novikov::IntIntervalSet::Interval> intervals =
      tree.getIntervals();
  expectInt(static_cast<int>(intervals.size()), 1,
            "merge adjacent: 1 interval");
  expectInt(intervals[0].first, 10, "merge adjacent: first");
  expectInt(intervals[0].second, 30, "merge adjacent: second");
}

void testAddKeepsSeparate()
{
  novikov::IntIntervalSet tree(100);
  tree.add(10, 14);
  tree.add(19, 30);

  const std::vector<novikov::IntIntervalSet::Interval> intervals =
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
  novikov::IntIntervalSet tree(100);
  tree.add(10, 30);
  tree.remove(15, 18);

  expectTrue(!tree.has(15), "remove: !has(15)");
  expectTrue(!tree.has(17), "remove: !has(17)");
  expectTrue(tree.has(14), "remove: has(14)");
  expectTrue(tree.has(19), "remove: has(19)");
  expect64(tree.getLength(), 15, "remove: length 15");

  const std::vector<novikov::IntIntervalSet::Interval> intervals =
      tree.getIntervals();
  expectInt(static_cast<int>(intervals.size()), 2, "remove: 2 intervals");
  expectInt(intervals[0].first, 10, "remove: left.first");
  expectInt(intervals[0].second, 14, "remove: left.second");
  expectInt(intervals[1].first, 19, "remove: right.first");
  expectInt(intervals[1].second, 30, "remove: right.second");
}

void testRemoveAll()
{
  novikov::IntIntervalSet tree(100);
  tree.add(10, 20);
  tree.remove(10, 20);

  expect64(tree.getLength(), 0, "remove all: length 0");
  expectTrue(tree.getIntervals().empty(), "remove all: empty");
}

void testRemoveFromEmpty()
{
  novikov::IntIntervalSet tree(100);
  tree.remove(10, 20);

  expect64(tree.getLength(), 0, "remove from empty: length 0");
  expectTrue(tree.getIntervals().empty(), "remove from empty: empty");
}

void testHasOutOfRange()
{
  novikov::IntIntervalSet tree(100);
  tree.add(0, 99);

  expectTrue(tree.has(0), "range: has(0)");
  expectTrue(tree.has(99), "range: has(99)");
  expectTrue(!tree.has(-1), "range: !has(-1)");
  expectTrue(!tree.has(100), "range: !has(100)");
}

void testAddOutOfRangeThrows()
{
  novikov::IntIntervalSet tree(100);

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
  novikov::IntIntervalSet tree(100);
  tree.add(10, 20);
  tree.add(25, 30);
  tree.add(15, 28);
  tree.remove(15, 18);

  expect64(tree.getLength(), 15, "example: length 15");
}

void testUnite()
{
  novikov::IntIntervalSet left(100);
  left.add(10, 14);
  left.add(19, 30);

  novikov::IntIntervalSet right(100);
  right.add(40, 50);

  const novikov::IntIntervalSet result =
      novikov::IntIntervalSet::unite(left, right);

  expectInt(result.getRangeSize(), 100, "unite: range");
  expectInt(static_cast<int>(result.getIntervals().size()), 3,
            "unite: 3 intervals");
  expect64(result.getLength(), 25, "unite: length 25");
}

void testUniteDifferentRangeSizes()
{
  novikov::IntIntervalSet small(50);
  small.add(0, 10);

  novikov::IntIntervalSet big(200);
  big.add(100, 150);

  const novikov::IntIntervalSet result =
      novikov::IntIntervalSet::unite(small, big);

  expectInt(result.getRangeSize(), 200, "unite diff: max range");
  expectTrue(result.has(0), "unite diff: has(0)");
  expectTrue(result.has(150), "unite diff: has(150)");
}

void testIntersect()
{
  novikov::IntIntervalSet left(100);
  left.add(10, 14);
  left.add(19, 30);

  novikov::IntIntervalSet right(100);
  right.add(12, 22);

  const novikov::IntIntervalSet result =
      novikov::IntIntervalSet::intersect(left, right);

  const std::vector<novikov::IntIntervalSet::Interval> intervals =
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
  novikov::IntIntervalSet left(100);
  left.add(10, 20);

  novikov::IntIntervalSet right(100);
  right.add(50, 60);

  const novikov::IntIntervalSet result =
      novikov::IntIntervalSet::intersect(left, right);

  expect64(result.getLength(), 0, "intersect empty: length 0");
  expectTrue(result.getIntervals().empty(), "intersect empty: empty");
}

void testCopyAndMove()
{
  novikov::IntIntervalSet original(100);
  original.add(10, 20);
  original.add(30, 40);

  novikov::IntIntervalSet copy(original);
  expectTrue(copy.has(10), "copy: has(10)");
  expectTrue(copy.has(30), "copy: has(30)");
  expect64(copy.getLength(), 20, "copy: length 20");

  original.remove(10, 20);
  expectTrue(!original.has(10), "copy indep: original changed");
  expectTrue(copy.has(10), "copy indep: copy unchanged");

  novikov::IntIntervalSet moved(std::move(copy));
  expectTrue(moved.has(10), "move: has(10)");
  expectTrue(moved.has(30), "move: has(30)");
}

void testSaveLoadRoundTrip()
{
  const std::string filename = "test-roundtrip.tmp";
  std::remove(filename.c_str());

  {
    novikov::IntIntervalSet tree(100);
    tree.add(10, 14);
    tree.add(19, 30);
    expectTrue(tree.save(filename), "save: true");
  }

  {
    novikov::IntIntervalSet tree(1);
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

  novikov::IntIntervalSet tree(1);
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

  novikov::IntIntervalSet tree(1);
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

  novikov::IntIntervalSet tree(1);
  expectTrue(!tree.load(filename), "load bad interval: false");

  std::remove(filename.c_str());
}

void testLoadMissingFile()
{
  novikov::IntIntervalSet tree(1);
  expectTrue(!tree.load("no-such-file.tmp"), "load missing: false");
}

void testLongLongIndexType()
{
  using LongSet = novikov::IntervalSet<long long>;

  LongSet tree(1000000LL);
  tree.add(100LL, 200LL);
  tree.add(500000LL, 600000LL);

  expectTrue(tree.has(150LL), "long long: has inside");
  expectTrue(!tree.has(300LL), "long long: has gap");
  expectTrue(tree.has(550000LL), "long long: has second");
  expect64(tree.getLength(), 100100LL, "long long: length");

  const std::vector<LongSet::Interval> intervals = tree.getIntervals();
  expectInt(static_cast<int>(intervals.size()), 2,
            "long long: 2 intervals");
  expect64(intervals[0].first, 100LL, "long long: f1");
  expect64(intervals[0].second, 200LL, "long long: s1");
  expect64(intervals[1].first, 500000LL, "long long: f2");
  expect64(intervals[1].second, 600000LL, "long long: s2");
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
      "OK\n"
      "[10,14] [19,30] [40,50]\n"
      "OK\n"
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

#define RUN_TEST(fn) \
  do { \
    logTest(#fn); \
    fn(); \
  } while (0)

}

int main(int argc, char** argv)
{
  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "-v") {
      verbose = true;
    }
  }

  RUN_TEST(testEmptyTreeHas);
  RUN_TEST(testEmptyTreeLengthAndIntervals);
  RUN_TEST(testAddSingleInterval);
  RUN_TEST(testAddMergesOverlapping);
  RUN_TEST(testAddMergesAdjacent);
  RUN_TEST(testAddKeepsSeparate);
  RUN_TEST(testRemoveSplitsInterval);
  RUN_TEST(testRemoveAll);
  RUN_TEST(testRemoveFromEmpty);
  RUN_TEST(testHasOutOfRange);
  RUN_TEST(testAddOutOfRangeThrows);
  RUN_TEST(testLengthExampleFromTask);
  RUN_TEST(testUnite);
  RUN_TEST(testUniteDifferentRangeSizes);
  RUN_TEST(testIntersect);
  RUN_TEST(testIntersectEmpty);
  RUN_TEST(testCopyAndMove);
  RUN_TEST(testSaveLoadRoundTrip);
  RUN_TEST(testLoadDanglingPair);
  RUN_TEST(testLoadMalformedHeader);
  RUN_TEST(testLoadIntervalOutOfRange);
  RUN_TEST(testLoadMissingFile);
  RUN_TEST(testLongLongIndexType);

  RUN_TEST(testScenarioFromTask);
  RUN_TEST(testShowEmptyTree);
  RUN_TEST(testCreateDuplicateFails);
  RUN_TEST(testCreateInvalidRangeFails);
  RUN_TEST(testUnknownTreeFails);
  RUN_TEST(testUnknownCommandFails);
  RUN_TEST(testAddOutOfRangeFails);
  RUN_TEST(testHasOutOfRangeInCommand);
  RUN_TEST(testInvalidIntegerFails);
  RUN_TEST(testMissingArgumentFails);
  RUN_TEST(testEmptyLinesIgnored);
  RUN_TEST(testUnionAndIntersectSameRange);
  RUN_TEST(testSaveAndLoadScenario);
  RUN_TEST(testSaveOverwriteDeclined);
  RUN_TEST(testSaveOverwriteConfirmed);
  RUN_TEST(testLoadMissingFileInCommand);

  if (failures == 0) {
    std::cout << "All tests passed\n";
    return 0;
  }

  std::cout << "Failures: " << failures << '\n';
  return 1;
}
