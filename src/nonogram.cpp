#include "nonogram.hpp"

void Puzzle::resize(int width, int height) {
  m_width = width;
  m_height = height;
  m_vertical_rules.resize(width);
  m_horizontal_rules.resize(height);
}

void read_rules(std::istream &is, std::vector<std::vector<int>> &rules) {
  int line_size;
  for (int i = 0; i < rules.size(); ++i) {
    is >> line_size;
    rules[i].resize(line_size);
    for (int j = 0; j < line_size; ++j) {
      is >> rules[i][j];
    }
  }
}

void read_puzzle(std::istream &is, Puzzle &puzzle) {
  int width, height;
  is >> width;
  is >> height;

  puzzle.resize(width, height);
  read_rules(is, puzzle.m_vertical_rules);
  read_rules(is, puzzle.m_horizontal_rules);
}

void print_rules(std::ostream &os, const std::vector<std::vector<int>> &rules) {
  os << "[" << std::endl;
  for (const auto &line : rules) {
    os << " [";
    for (const auto value : line) {
      os << " " << value;
    }
    os << " ]" << std::endl;
  }
  os << "]" << std::endl;
}

void print_puzzle(std::ostream &os, const Puzzle &puzzle) {
  os << "width: " << puzzle.m_width << " height: " << puzzle.m_height
     << std::endl;
  os << "columns: ";
  print_rules(os, puzzle.m_vertical_rules);
  os << "rows: ";
  print_rules(os, puzzle.m_horizontal_rules);
}

Solution::Solution(int width, int height)
    : m_cells_(height, std::vector<bool>(width, false)),
      m_cells_transposed_(width, std::vector<bool>(height, false)) {}

void Solution::set_cell(int i, int j, bool value) {
  m_cells_[i][j] = value;
  m_cells_transposed_[j][i] = value;
}

const std::vector<bool> &Solution::get_row(int i) const { return m_cells_[i]; }
const std::vector<bool> &Solution::get_column(int j) const {
  return m_cells_transposed_[j];
}

void print_solution(std::ostream &os, const Solution &solution) {
  for (const auto &row : solution.m_cells_) {
    for (auto v : row) {
      if (v) {
        os << "X";
      } else {
        os << ".";
      }
    }
    os << std::endl;
  }
}

bool match_pattern(const std::vector<bool> &pattern,
                   const std::vector<int> &rules) {
  int rule_i = 0;
  int block_length = 0;
  for (auto v : pattern) {
    if (rule_i == rules.size()) {
      break;
    }

    if (v) {
      ++block_length;
      if (block_length > rules[rule_i]) {
        return false;
      }
    } else if (block_length > 0) {
      if (block_length != rules[rule_i]) {
        return false;
      }
      block_length = 0;
      ++rule_i;
    }
  }

  if (block_length > 0) {
    if (rule_i != rules.size() - 1) {
      return false;
    }

    if (rules.back() != block_length) {
      return false;
    }

    ++rule_i;
  }

  if (rule_i != rules.size()) {
    return false;
  }

  return true;
}

bool is_valid_solution(const Solution &solution, const Puzzle &puzzle) {
  for (int i = 0; i < puzzle.m_height; ++i) {
    if (!match_pattern(solution.get_row(i), puzzle.m_horizontal_rules[i])) {
      return false;
    }
  }

  for (int j = 0; j < puzzle.m_width; ++j) {
    if (!match_pattern(solution.get_column(j), puzzle.m_vertical_rules[j])) {
      return false;
    }
  }

  return true;
}

bool solve_iter(int i, int j, Solution &solution, const Puzzle &puzzle) {
  if (i == puzzle.m_height) {
    return is_valid_solution(solution, puzzle);
  }

  assert(!solution.m_cells_[i][j]);

  int next_i;
  int next_j;
  if (j == puzzle.m_width - 1) {
    next_i = i + 1;
    next_j = 0;
  } else {
    next_i = i;
    next_j = j + 1;
  }

  if (solve_iter(next_i, next_j, solution, puzzle)) {
    return true;
  }

  solution.set_cell(i, j, true);
  if (solve_iter(next_i, next_j, solution, puzzle)) {
    return true;
  }

  solution.set_cell(i, j, false);

  return false;
}

Solution solve_puzzle(const Puzzle &puzzle) {
  Solution solution(puzzle.m_width, puzzle.m_height);
  auto solve_ok = solve_iter(0, 0, solution, puzzle);
  assert(solve_ok);
  return solution;
}
