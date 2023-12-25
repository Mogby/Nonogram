#pragma once

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <istream>
#include <random>

struct Puzzle {
  int m_width;
  int m_height;
  std::vector<std::vector<int>> m_vertical_rules;
  std::vector<std::vector<int>> m_horizontal_rules;

  void resize(int width, int height);
};

void read_puzzle(std::istream &is, Puzzle &puzzle);
void print_puzzle(std::ostream &os, const Puzzle &puzzle);

struct Solution {
  Solution(int width, int height);

  void set_cell(int i, int j, bool value);

  const std::vector<bool> &get_row(int i) const;
  const std::vector<bool> &get_column(int j) const;

  std::vector<std::vector<bool>> m_cells_;
  std::vector<std::vector<bool>> m_cells_transposed_;
};

void print_solution(std::ostream &os, const Solution &solution);

Solution solve_puzzle(const Puzzle &puzzle);
