#include "nonogram.hpp"

#include <iterator>
#include <optional>

#include <cassert>
#include <sstream>
#include <string>

Puzzle::Puzzle(int width, int height)
    : m_width(width), m_height(height), m_vertical_rules(width),
      m_horizontal_rules(height) {}

std::istringstream read_next_line(std::istream &is) {
  std::string line;
  std::getline(is, line);
  return std::istringstream(line);
}

std::vector<std::vector<int>> read_rules(std::istream &is, int n_rules) {
  std::vector<std::vector<int>> rules(n_rules);
  for (int i = 0; i < rules.size(); ++i) {
    auto line = read_next_line(is);
    Rule rule;
    while (line >> rule) {
      rules[i].push_back(rule);
    }
  }
  return rules;
}

Puzzle read_puzzle(std::istream &is) {
  int width, height;
  read_next_line(is) >> width >> height;
  Puzzle puzzle(width, height);
  puzzle.m_vertical_rules = read_rules(is, width);
  puzzle.m_horizontal_rules = read_rules(is, height);
  return puzzle;
}

void print_rules(std::ostream &os, const std::vector<std::vector<int>> &rules) {
  int sum = 0;
  os << "[" << std::endl;
  for (const auto &line : rules) {
    os << " [";
    for (const auto value : line) {
      os << " " << value;
      sum += value;
    }
    os << " ]" << std::endl;
  }
  os << "]" << std::endl;
  os << "sum :" << sum << std::endl;
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
    : m_width(width), m_height(height), m_is_final(false),
      m_cells_(height, CellsLine(width, Cell::UNKNOWN)),
      m_cells_transposed_(width, CellsLine(height, Cell::UNKNOWN)) {}

Cell Solution::get_cell(int i, int j) const { return m_cells_[i][j]; }
void Solution::set_cell(int i, int j, Cell value) {
  m_cells_[i][j] = value;
  m_cells_transposed_[j][i] = value;
}

void Solution::set_row(int i, const CellsLine &line) {
  for (int j = 0; j < line.size(); ++j) {
    set_cell(i, j, line[j]);
  }
}

void Solution::set_column(int j, const CellsLine &line) {
  for (int i = 0; i < line.size(); ++i) {
    set_cell(i, j, line[i]);
  }
}

CellsLine &Solution::get_row(int i) { return m_cells_[i]; }
const CellsLine &Solution::get_row(int i) const { return m_cells_[i]; }

CellsLine &Solution::get_column(int j) { return m_cells_transposed_[j]; }
const CellsLine &Solution::get_column(int j) const {
  return m_cells_transposed_[j];
}

char print_cell(Cell c) {
  switch (c) {
  case Cell::UNKNOWN:
    return '~';
  case Cell::EMPTY:
    return '.';
  case Cell::FILLED:
    return 'X';
  }
}

void print_solution(std::ostream &os, const Solution &solution) {
  for (const auto &row : solution.m_cells_) {
    for (auto v : row) {
      os << print_cell(v)
         << print_cell(v); // print twice for better proportions
    }
    os << std::endl;
  }
}

template <typename I, typename V>
concept forward_iterator_for_value =
    std::forward_iterator<I> &&
    std::same_as<typename std::iterator_traits<I>::value_type, V>;

template <typename CellIter>
  requires forward_iterator_for_value<CellIter, Cell>
bool range_has_no_filled_cells(CellIter begin, CellIter end) {
  for (; begin < end; ++begin) {
    if (*begin == Cell::FILLED) {
      return false;
    }
  }
  return true;
}

template <typename CellIter>
  requires forward_iterator_for_value<CellIter, Cell>
bool range_has_no_empty_cells(CellIter begin, CellIter end) {
  for (; begin < end; ++begin) {
    if (*begin == Cell::EMPTY) {
      return false;
    }
  }
  return true;
}

template <typename RuleIter, typename CellIter>
  requires forward_iterator_for_value<RuleIter, int> &&
           forward_iterator_for_value<CellIter, Cell>
bool fit_iter(std::vector<int> &cur_fit, RuleIter rule_i, RuleIter rule_end,
              CellIter cell_i, CellIter cell_end) {
  if (rule_i == rule_end) {
    cur_fit.pop_back();
    return true;
  }

  bool is_last_rule = rule_i == rule_end - 1;
  auto min_required_space = is_last_rule ? *rule_i : (*rule_i + 1);

  int cur_pos = cur_fit.back();
  for (; cell_end - cell_i >= min_required_space;
       ++cell_i, ++cur_pos, ++cur_fit.back()) {

    bool can_place = range_has_no_empty_cells(cell_i, cell_i + *rule_i);
    if (!is_last_rule) {
      can_place = can_place && (*(cell_i + *rule_i) != Cell::FILLED);
    } else {
      can_place =
          can_place && range_has_no_filled_cells(cell_i + *rule_i, cell_end);
    }

    if (*cell_i == Cell::FILLED) {
      cur_fit.push_back(cur_pos + min_required_space);
      if (can_place && fit_iter(cur_fit, rule_i + 1, rule_end,
                                cell_i + min_required_space, cell_end)) {
        return true;
      }
      cur_fit.pop_back();
      return false;
    }

    if (!can_place) {
      continue;
    }

    cur_fit.push_back(cur_pos + *rule_i + 1);
    if (fit_iter(cur_fit, rule_i + 1, rule_end, cell_i + *rule_i + 1,
                 cell_end)) {
      return true;
    }
    cur_fit.pop_back();
  }

  return false;
}

std::optional<std::vector<int>> fit_left(const RulesLine &rules,
                                         const CellsLine &line) {
  std::vector<int> fit;
  fit.push_back(0);

  if (fit_iter(fit, rules.begin(), rules.end(), line.begin(), line.end())) {
    return fit;
  }
  return std::nullopt;
}

std::optional<std::vector<int>> fit_right(const RulesLine &rules,
                                          const CellsLine &line) {
  std::vector<int> fit;
  fit.push_back(0);

  if (fit_iter(fit, rules.rbegin(), rules.rend(), line.rbegin(), line.rend())) {
    std::reverse(fit.begin(), fit.end());
    for (int i = 0; i < fit.size(); ++i) {
      fit[i] = line.size() - fit[i] - rules[i];
    }
    return fit;
  }
  return std::nullopt;
}

UpdateResult update_cells(const RulesLine &rules, CellsLine &line) {
  bool line_updated = false;
  if (rules.size() == 0) {
    if (std::find(line.begin(), line.end(), Cell::FILLED) != line.end()) {
      return {.m_rules_fit = false, .m_line_updated = false};
    }

    for (int i = 0; i < line.size(); ++i) {
      if (line[i] == Cell::UNKNOWN) {
        line[i] = Cell::EMPTY;
        line_updated = true;
      }
    }
    return {.m_rules_fit = true, .m_line_updated = line_updated};
  }

  auto lfit_opt = fit_left(rules, line);
  if (!lfit_opt.has_value()) {
    return {.m_rules_fit = false, .m_line_updated = false};
  }
  auto rfit_opt = fit_right(rules, line);
  assert(rfit_opt.has_value());

  auto &lfit = lfit_opt.value();
  auto &rfit = rfit_opt.value();

  int rule_i = 0;
  int intersect_left = rfit[0];
  int intersect_right = lfit[0] + rules[0];
  int prev_rule_rightmost_i = 0;
  for (int i = 0; i < line.size(); ++i) {
    if (prev_rule_rightmost_i <= i) {
      if ((rule_i == rules.size() && i < line.size()) || (i < lfit[rule_i])) {
        // cell cannot covered by any rule
        if (line[i] == Cell::UNKNOWN) {
          line_updated = true;
          line[i] = Cell::EMPTY;
        }
      }
    }

    if (rule_i == rules.size()) {
      continue;
    }

    if (intersect_left <= i && i < intersect_right && line[i] != Cell::FILLED) {
      // cell is covered by current rule
      assert(line[i] == Cell::UNKNOWN);
      line_updated = true;
      line[i] = Cell::FILLED;
    }

    if (i == intersect_right - 1) {
      prev_rule_rightmost_i = rfit[rule_i] + rules[rule_i];
      ++rule_i;
      intersect_left = rfit[rule_i];
      intersect_right = lfit[rule_i] + rules[rule_i];
    }
  }

  return {.m_rules_fit = true, .m_line_updated = line_updated};
}

Solution solve_iter(const Puzzle &puzzle, Solution &solution) {
  bool updated = true;
  UpdateResult update_result;
  while (updated) {
    updated = false;

    for (int j = 0; j < puzzle.m_width; ++j) {
      update_result = update_cells(puzzle.m_vertical_rules[j],
                                   solution.m_cells_transposed_[j]);
      if (!update_result.m_rules_fit) {
        return solution;
      }
      updated = updated || update_result.m_line_updated;
      solution.set_column(j, solution.m_cells_transposed_[j]);
    }

    for (int i = 0; i < puzzle.m_height; ++i) {
      update_result =
          update_cells(puzzle.m_horizontal_rules[i], solution.m_cells_[i]);
      if (!update_result.m_rules_fit) {
        return solution;
      }
      updated = updated || update_result.m_line_updated;
      solution.set_row(i, solution.m_cells_[i]);
    }
  }

  for (int i = 0; i < solution.m_height; ++i) {
    for (int j = 0; j < solution.m_width; ++j) {
      if (solution.get_cell(i, j) == Cell::UNKNOWN) {
        for (auto bt_value : {Cell::FILLED, Cell::EMPTY}) {
          auto solution_bt = solution;
          solution_bt.set_cell(i, j, bt_value);
          auto next_solution = solve_iter(puzzle, solution_bt);
          if (next_solution.m_is_final) {
            return next_solution;
          }
        }
        solution.m_is_final = false;
        return solution;
      }
    }
  }

  solution.m_is_final = true;
  return solution;
}

Solution solve_puzzle(const Puzzle &puzzle) {
  Solution initial_solution(puzzle.m_width, puzzle.m_height);
  auto solution = solve_iter(puzzle, initial_solution);
  return solution;
}
