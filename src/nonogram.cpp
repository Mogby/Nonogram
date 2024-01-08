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
  os << "sum: " << sum << std::endl;
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
      m_cells_transposed_(width, CellsLine(height, Cell::UNKNOWN)),
      m_row_solved_flg(height, false), m_column_solved_flg(width, false) {}

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

CellsLine Solution::get_row(int i) const { return m_cells_[i]; }

CellsLine Solution::get_column(int j) const { return m_cells_transposed_[j]; }

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
bool range_has_filled_cells(CellIter begin, CellIter end) {
  return std::find(begin, end, Cell::FILLED) != end;
}

template <typename CellIter>
  requires forward_iterator_for_value<CellIter, Cell>
bool range_has_empty_cells(CellIter begin, CellIter end) {
  return std::find(begin, end, Cell::EMPTY) != end;
}

template <typename RuleIter, typename CellIter>
  requires forward_iterator_for_value<RuleIter, int> &&
           forward_iterator_for_value<CellIter, Cell>
struct FitIterState {
  // FitIter m_prev_lfit_i;
  // FitIter m_prev_lfit_end;
  // FitIter m_prev_rfit_i;
  // FitIter m_prev_rfit_end;

  FitIterState(RuleIter rule_i, RuleIter rule_end, CellIter cell_i,
               CellIter cell_end)
      : m_rule_i(rule_i), m_rule_end(rule_end), m_cell_i(cell_i),
        m_cell_end(cell_end), prev_cell_was_filled(false) {
    update();
  }

  RuleIter m_rule_i;
  RuleIter m_rule_end;
  CellIter m_cell_i;
  CellIter m_cell_end;

  bool prev_cell_was_filled;

  bool is_last_rule;
  int min_required_space;

  bool rule_fits() {
    if (range_has_empty_cells(m_cell_i, m_cell_i + *m_rule_i)) {
      return false;
    }

    if (is_last_rule) {
      return !range_has_filled_cells(m_cell_i + *m_rule_i, m_cell_end);
    } else {
      return *(m_cell_i + *m_rule_i) != Cell::FILLED;
    }
  }

  bool should_stop_cell_iter() {
    return m_cell_end - m_cell_i < min_required_space || prev_cell_was_filled;
    // || m_cur_fit.back() >= *m_prev_rfit_i;
  }

  void next_cell() {
    prev_cell_was_filled = *m_cell_i == Cell::FILLED;
    ++m_cell_i;
    // ++m_prev_lfit_i;
    // ++m_prev_rfit_i;
  }

  bool should_stop_rule_iter() { return m_rule_i == m_rule_end; }

  void next_rule() {
    m_cell_i += *m_rule_i + 1;
    ++m_rule_i;
    // ++m_prev_lfit_i;
    // ++m_prev_rfit_i;

    update();
    // m_cur_fit.push_back(*m_prev_lfit_i);
    // m_cur_fit.push_back(cur_cell_index() + *(m_rule_i - 1) + 1);
  }

private:
  void update() {
    if (should_stop_rule_iter()) {
      return;
    }
    is_last_rule = m_rule_i == m_rule_end - 1;
    min_required_space = is_last_rule ? *m_rule_i : (*m_rule_i + 1);
  }
};

template <typename RuleIter, typename CellIter>
bool fit_iter(std::vector<int> &cur_fit,
              FitIterState<RuleIter, CellIter> &state) {
  if (state.should_stop_rule_iter()) {
    cur_fit.pop_back();
    return true;
  }

  for (; !state.should_stop_cell_iter(); state.next_cell(), ++cur_fit.back()) {
    if (!state.rule_fits()) {
      continue;
    }

    auto new_state = state;
    new_state.next_rule();
    cur_fit.push_back(cur_fit.back() + *state.m_rule_i + 1);
    if (fit_iter(cur_fit, new_state)) {
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

  auto state =
      FitIterState(rules.begin(), rules.end(), line.begin(), line.end());
  if (fit_iter(fit, state)) {
    return fit;
  }
  return std::nullopt;
}

std::optional<std::vector<int>> fit_right(const RulesLine &rules,
                                          const CellsLine &line) {
  std::vector<int> fit;
  fit.push_back(0);

  auto state =
      FitIterState(rules.rbegin(), rules.rend(), line.rbegin(), line.rend());
  if (fit_iter(fit, state)) {
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
    if (range_has_filled_cells(line.begin(), line.end())) {
      return {.m_rules_fit = false,
              .m_line_updated = false,
              .m_line_solved = false};
    }

    for (int i = 0; i < line.size(); ++i) {
      if (line[i] == Cell::UNKNOWN) {
        line[i] = Cell::EMPTY;
        line_updated = true;
      }
    }
    return {.m_rules_fit = true,
            .m_line_updated = line_updated,
            .m_line_solved = true};
  }

  auto lfit_opt = fit_left(rules, line);
  if (!lfit_opt.has_value()) {
    return {
        .m_rules_fit = false, .m_line_updated = false, .m_line_solved = false};
  }
  auto rfit_opt = fit_right(rules, line);
  assert(rfit_opt.has_value());

  auto &lfit = lfit_opt.value();
  auto &rfit = rfit_opt.value();

  int rule_i = 0;
  int intersect_left = rfit[0];
  int intersect_right = lfit[0] + rules[0];
  int prev_rule_rightmost_i = 0;
  bool line_solved = true;
  for (int i = 0; i < line.size(); ++i) {
    if (line_solved && rule_i < rules.size() && lfit[rule_i] != rfit[rule_i]) {
      line_solved = false;
    }

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

  return {.m_rules_fit = rule_i == rules.size(),
          .m_line_updated = line_updated,
          .m_line_solved = line_solved};
}

Solution solve_iter(const Puzzle &puzzle, Solution &solution) {
  bool updated = true;
  UpdateResult update_result;
  while (updated) {
    updated = false;

    for (int j = 0; j < puzzle.m_width; ++j) {
      if (solution.m_column_solved_flg[j]) {
        continue;
      }
      auto column = solution.get_column(j);
      update_result = update_cells(puzzle.m_vertical_rules[j], column);
      if (!update_result.m_rules_fit) {
        return solution;
      }
      if (update_result.m_line_solved) {
        solution.m_column_solved_flg[j] = true;
      }
      updated = updated || update_result.m_line_updated;
      solution.set_column(j, column);
    }

    for (int i = 0; i < puzzle.m_height; ++i) {
      if (solution.m_row_solved_flg[i]) {
        continue;
      }
      auto row = solution.get_row(i);
      update_result = update_cells(puzzle.m_horizontal_rules[i], row);
      if (!update_result.m_rules_fit) {
        return solution;
      }
      if (update_result.m_line_solved) {
        solution.m_row_solved_flg[i] = true;
      }
      updated = updated || update_result.m_line_updated;
      solution.set_row(i, row);
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
