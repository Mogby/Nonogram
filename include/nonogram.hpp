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

enum class Cell : int { UNKNOWN = 0, FILLED = 1, EMPTY = 2 };

char print_cell(Cell c);

struct CellsLine : public std::vector<Cell> {
  template <typename... Args>
  CellsLine(Args &&...args) : std::vector<Cell>(std::forward<Args>(args)...) {}

  std::array<int, 3> cells_counts{0, 0, 0};
};

struct Solution {
  Solution(int width, int height);

  Cell get_cell(int i, int j) const;
  void set_cell(int i, int j, Cell value);

  void set_row(int i, const CellsLine &line);
  void set_column(int j, const CellsLine &line);

  CellsLine &get_row(int i);
  CellsLine &get_column(int j);

  const CellsLine &get_row(int i) const;
  const CellsLine &get_column(int j) const;

  std::vector<int> get_rows_solve_order() const;
  std::vector<int> get_columns_solve_order() const;

  int m_width;
  int m_height;

  bool m_is_final;

  std::vector<CellsLine> m_cells_;
  std::vector<CellsLine> m_cells_transposed_;
};

void print_solution(std::ostream &os, const Solution &solution);

std::optional<std::vector<int>> fit_left(const RulesLine &rules,
                                         const CellsLine &line);

std::optional<std::vector<int>> fit_right(const RulesLine &rules,
                                          const CellsLine &line);

struct UpdateResult {
  bool m_rules_fit;
  bool m_line_updated;
};

UpdateResult update_cells(const RulesLine &rules, CellsLine &line);

Solution solve_puzzle(const Puzzle &puzzle);
