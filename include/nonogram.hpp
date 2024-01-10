#pragma once

#include <iostream>
#include <optional>
#include <vector>

using Rule = int;

using RulesLine = std::vector<Rule>;

struct Puzzle {
  Puzzle(int width, int height);

  int m_width;
  int m_height;
  std::vector<RulesLine> m_vertical_rules;
  std::vector<RulesLine> m_horizontal_rules;
};

Puzzle read_puzzle(std::istream &is);
void print_puzzle(std::ostream &os, const Puzzle &puzzle);

enum class Cell { UNKNOWN, FILLED, EMPTY };

char print_cell(Cell c);

using CellsLine = std::vector<Cell>;

struct SolutionLine {
  RulesLine m_rules;
  CellsLine m_cells;
  bool m_solved_flg;
  std::vector<int> m_lfit;
  std::vector<int> m_rfit;
  std::vector<int> m_lfit_reversed;
  std::vector<int> m_rfit_reversed;

  SolutionLine(int size, const RulesLine &rules);
  void update_fits(std::vector<int> &&lfit, std::vector<int> &&rfit);
  const size_t size() const;
};

struct Solution {
  Solution(int width, int height, const std::vector<RulesLine> &vertical_rules,
           const std::vector<RulesLine> &horizontal_rules);

  const Cell get_cell(int i, int j) const;
  void set_cell(int i, int j, Cell value);

  void set_row(int i, const CellsLine &line, std::vector<int> &&lfit,
               std::vector<int> &&rfit);
  void set_column(int j, const CellsLine &line, std::vector<int> &&lfit,
                  std::vector<int> &&rfit);

  const SolutionLine &get_row(int i) const;
  const SolutionLine &get_column(int j) const;

  void mark_row_solved(int i);
  void mark_column_solved(int j);

  const bool is_row_solved(int i) const;
  const bool is_column_solved(int j) const;

  int m_width;
  int m_height;

  bool m_is_final;
  int m_n_solved_cells;

  std::vector<SolutionLine> m_rows_;
  std::vector<SolutionLine> m_columns_;
};

void print_solution(std::ostream &os, const Solution &solution);

std::optional<std::vector<int>> fit_left(const RulesLine &rules,
                                         const SolutionLine &line);

std::optional<std::vector<int>> fit_right(const RulesLine &rules,
                                          const SolutionLine &line);

struct UpdateResult {
  bool m_rules_fit;
  int m_n_updated_cells;
  bool m_line_solved;

  CellsLine m_cells;
  std::optional<std::vector<int>> m_lfit{std::nullopt};
  std::optional<std::vector<int>> m_rfit{std::nullopt};
};

UpdateResult update_cells(const RulesLine &rules, const SolutionLine &line);

Solution solve_puzzle(const Puzzle &puzzle);
