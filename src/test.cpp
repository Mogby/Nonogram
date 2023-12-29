#include <gtest/gtest.h>

#include "nonogram.hpp"

RulesLine read_rules_line(const std::string &s) {
  RulesLine result;
  std::stringstream helper;
  helper << s;
  Rule rule;
  while (helper >> rule) {
    result.push_back(rule);
  }
  return result;
}

Cell read_cell(char c) {
  switch (c) {
  case '~':
    return Cell::UNKNOWN;
  case '.':
    return Cell::EMPTY;
  case 'X':
    return Cell::FILLED;
  default:
    throw std::exception{};
  }
}

CellsLine read_cells_line(const std::string &s) {
  CellsLine result;
  for (auto c : s) {
    result.push_back(read_cell(c));
  }
  return result;
}

std::string print_cells_line(const CellsLine &cells) {
  std::string s;
  for (auto c : cells) {
    s += print_cell(c);
  }
  return s;
}

TEST(TestParsing, TestReadRulesLine) {
  std::string s = "3 1 1";
  RulesLine rules;
  rules = read_rules_line(s);
  ASSERT_EQ(rules.size(), 3);
  ASSERT_EQ(rules[0], 3);
  ASSERT_EQ(rules[1], 1);
  ASSERT_EQ(rules[2], 1);
}

TEST(TestParsing, TestReadCellsLine) {
  std::string s = "~.X";
  CellsLine cells;
  cells = read_cells_line(s);
  ASSERT_EQ(cells.size(), 3);
  ASSERT_EQ(cells[0], Cell::UNKNOWN);
  ASSERT_EQ(cells[1], Cell::EMPTY);
  ASSERT_EQ(cells[2], Cell::FILLED);
}

TEST(TestSolver, TestFitLeftSimple) {
  std::string rules_str = "3 1";
  std::string cells_str = "~~~~~~";
  auto rules = read_rules_line(rules_str);
  auto cells = read_cells_line(cells_str);
  auto fit = fit_left(rules, cells);
  ASSERT_TRUE(fit.has_value());
  ASSERT_EQ(fit.value().size(), 2);
  ASSERT_EQ(fit.value()[0], 0);
  ASSERT_EQ(fit.value()[1], 4);
}

TEST(TestSolver, TestFitLeftCannotCoverEmptyCell) {
  std::string rules_str = "3 1";
  std::string cells_str = ".~~~~~";
  auto rules = read_rules_line(rules_str);
  auto cells = read_cells_line(cells_str);
  auto fit = fit_left(rules, cells);
  ASSERT_TRUE(fit.has_value());
  ASSERT_EQ(fit.value().size(), 2);
  ASSERT_EQ(fit.value()[0], 1);
  ASSERT_EQ(fit.value()[1], 5);
}

TEST(TestSolver, TestFitLeftCoversFilledCells) {
  std::string rules_str = "3 1";
  std::string cells_str = "~~XX~~";
  auto rules = read_rules_line(rules_str);
  auto cells = read_cells_line(cells_str);
  auto fit = fit_left(rules, cells);
  ASSERT_TRUE(fit.has_value());
  ASSERT_EQ(fit.value().size(), 2);
  ASSERT_EQ(fit.value()[0], 1);
  ASSERT_EQ(fit.value()[1], 5);
}

TEST(TestSolver, TestFitLeftCellsAfterLastRuleAreEmpty) {
  std::string rules_str = "3 1";
  std::string cells_str = "~~~~~X";
  auto rules = read_rules_line(rules_str);
  auto cells = read_cells_line(cells_str);
  auto fit = fit_left(rules, cells);
  ASSERT_TRUE(fit.has_value());
  ASSERT_EQ(fit.value().size(), 2);
  ASSERT_EQ(fit.value()[0], 0);
  ASSERT_EQ(fit.value()[1], 5);
}

TEST(TestSolver, TestFitLeftReturnsNulloptWhenFitIsImpossible) {
  std::string rules_str = "3 1 1";
  std::string cells_str = "~~~~~X";
  auto rules = read_rules_line(rules_str);
  auto cells = read_cells_line(cells_str);
  auto fit = fit_left(rules, cells);
  ASSERT_TRUE(!fit.has_value());
}

TEST(TestSolver, TestFitLeftEmptyRules) {
  std::string rules_str = "";
  std::string cells_str = "~~~~~~";
  auto rules = read_rules_line(rules_str);
  auto cells = read_cells_line(cells_str);
  auto fit = fit_right(rules, cells);
  ASSERT_TRUE(fit.has_value());
  ASSERT_EQ(fit.value().size(), 0);
}

TEST(TestSolver, TestFitRightSimple) {
  std::string rules_str = "3 1";
  std::string cells_str = "~~~~~~";
  auto rules = read_rules_line(rules_str);
  auto cells = read_cells_line(cells_str);
  auto fit = fit_right(rules, cells);
  ASSERT_TRUE(fit.has_value());
  ASSERT_EQ(fit.value().size(), 2);
  ASSERT_EQ(fit.value()[0], 1);
  ASSERT_EQ(fit.value()[1], 5);
}

TEST(TestSolver, TestUpdateCellsSimple) {
  std::string rules_str = "3 1";
  std::string cells_str = "~~~~~~";
  auto rules = read_rules_line(rules_str);
  auto cells = read_cells_line(cells_str);
  auto update = update_cells(rules, cells);
  ASSERT_TRUE(update.m_rules_fit);
  ASSERT_TRUE(update.m_line_updated);
  auto update_str = print_cells_line(cells);
  ASSERT_EQ(update_str, "~XX~~~");
}

TEST(TestSolver, TestUpdateCellsOneSolution) {
  std::string rules_str = "3 1";
  std::string cells_str = "~~~X~~";
  auto rules = read_rules_line(rules_str);
  auto cells = read_cells_line(cells_str);
  auto update = update_cells(rules, cells);
  ASSERT_TRUE(update.m_rules_fit);
  ASSERT_TRUE(update.m_line_updated);
  auto update_str = print_cells_line(cells);
  ASSERT_EQ(update_str, ".XXX.X");
}

TEST(TestSolver, TestUpdateCellsPartialUpdateFromFills) {
  std::string rules_str = "2 2";
  std::string cells_str = "~X~~~~~X~";
  auto rules = read_rules_line(rules_str);
  auto cells = read_cells_line(cells_str);
  auto update = update_cells(rules, cells);
  ASSERT_TRUE(update.m_rules_fit);
  ASSERT_TRUE(update.m_line_updated);
  auto update_str = print_cells_line(cells);
  ASSERT_EQ(update_str, "~X~...~X~");
}

TEST(TestSolver, TestUpdateCellsPartialUpdateFromBlanks) {
  std::string rules_str = "2 2";
  std::string cells_str = "~~~...~~~";
  auto rules = read_rules_line(rules_str);
  auto cells = read_cells_line(cells_str);
  auto update = update_cells(rules, cells);
  ASSERT_TRUE(update.m_rules_fit);
  ASSERT_TRUE(update.m_line_updated);
  auto update_str = print_cells_line(cells);
  ASSERT_EQ(update_str, "~X~...~X~");
}

TEST(TestSolver, TestUpdateCellsPartialUpdateFromBlanks2) {
  std::string rules_str = "3";
  std::string cells_str = "~XXX~";
  auto rules = read_rules_line(rules_str);
  auto cells = read_cells_line(cells_str);
  auto update = update_cells(rules, cells);
  ASSERT_TRUE(update.m_rules_fit);
  ASSERT_TRUE(update.m_line_updated);
  auto update_str = print_cells_line(cells);
  ASSERT_EQ(update_str, ".XXX.");
}

TEST(TestSolver, TestUpdateCellsIdempotency) {
  std::string rules_str = "2 2";
  std::string cells_str = "~~~...~~~";
  auto rules = read_rules_line(rules_str);
  auto cells = read_cells_line(cells_str);
  auto update = update_cells(rules, cells);
  update = update_cells(rules, cells);
  ASSERT_FALSE(update.m_line_updated);
}

TEST(TestSolver, TestUpdateCellsEmptyRule) {
  std::string rules_str = "";
  std::string cells_str = "~~~~~~~~~";
  auto rules = read_rules_line(rules_str);
  auto cells = read_cells_line(cells_str);
  auto update = update_cells(rules, cells);
  ASSERT_TRUE(update.m_rules_fit);
  ASSERT_TRUE(update.m_line_updated);
  auto update_str = print_cells_line(cells);
  ASSERT_EQ(update_str, ".........");
}

TEST(TestSolver, TestUpdateCellsRulesDoNotFit) {
  std::string rules_str = "";
  std::string cells_str = "X";
  auto rules = read_rules_line(rules_str);
  auto cells = read_cells_line(cells_str);
  auto update = update_cells(rules, cells);
  ASSERT_FALSE(update.m_rules_fit);
  ASSERT_FALSE(update.m_line_updated);
}
