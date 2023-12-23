#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <istream>
#include <random>

struct Puzzle {
  int m_width;
  int m_height;
  std::vector<std::vector<int>> m_cols;
  std::vector<std::vector<int>> m_rows;

  void resize(int width, int height) {
    m_width = width;
    m_height = height;
    m_cols.resize(width);
    m_rows.resize(height);
  }
};

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
  read_rules(is, puzzle.m_cols);
  read_rules(is, puzzle.m_rows);
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
  print_rules(os, puzzle.m_cols);
  os << "rows: ";
  print_rules(os, puzzle.m_rows);
}

struct Solution {
  Solution(int width, int height)
      : m_cells(height, std::vector<bool>(width, false)) {}

  std::vector<bool> get_row(int i) const { return m_cells[i]; }

  std::vector<bool> get_column(int j) const {
    std::vector<bool> column;
    for (const auto &row : m_cells) {
      column.push_back(row[j]);
    }
    return column;
  }

  std::vector<std::vector<bool>> m_cells;
};

void print_solution(std::ostream &os, const Solution &solution) {
  for (const auto &row : solution.m_cells) {
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

std::vector<int> encode_line(const std::vector<bool> &line) {
  std::vector<int> pattern;
  int len = 0;
  for (auto c : line) {
    if (c) {
      ++len;
    } else if (len > 0) {
      pattern.push_back(len);
      len = 0;
    }
  }
  if (len > 0) {
    pattern.push_back(len);
  }
  return pattern;
}

bool match_pattern(const std::vector<int> &pattern,
                   const std::vector<int> &rules) {
  if (pattern.size() != rules.size()) {
    return false;
  }

  for (int i = 0; i < pattern.size(); ++i) {
    if (pattern[i] != rules[i]) {
      return false;
    }
  }

  return true;
}

bool is_valid_solution(const Solution &solution, const Puzzle &puzzle) {
  for (int i = 0; i < puzzle.m_height; ++i) {
    auto row = solution.get_row(i);
    auto row_pattern = encode_line(row);
    if (!match_pattern(row_pattern, puzzle.m_rows[i])) {
      return false;
    }
  }

  for (int j = 0; j < puzzle.m_width; ++j) {
    auto col = solution.get_column(j);
    auto col_pattern = encode_line(col);
    if (!match_pattern(col_pattern, puzzle.m_cols[j])) {
      return false;
    }
  }

  return true;
}

bool solve_iter(int i, int j, Solution &solution, const Puzzle &puzzle) {
  if (i == puzzle.m_height) {
    return is_valid_solution(solution, puzzle);
  }

  assert(!solution.m_cells[i][j]);

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

  solution.m_cells[i][j] = true;
  if (solve_iter(next_i, next_j, solution, puzzle)) {
    return true;
  }

  solution.m_cells[i][j] = false;

  return false;
}

Solution solve_puzzle(const Puzzle &puzzle) {
  Solution solution(puzzle.m_width, puzzle.m_height);
  auto solve_ok = solve_iter(0, 0, solution, puzzle);
  assert(solve_ok);
  return solution;
}

int main(int argc, char **argv) {
  assert(argc >= 2);

  std::ifstream file(argv[1]);
  bool quiet = false;
  if (argc == 3) {
    assert(argv[2] == "-q" || argv[2] == "--quiet");
    quiet = true;
  }

  Puzzle p;
  read_puzzle(file, p);
  if (!quiet) {
    print_puzzle(std::cout, p);
  }

  auto s = solve_puzzle(p);
  if (!quiet) {
    print_solution(std::cout, s);
  }

  return 0;
}
