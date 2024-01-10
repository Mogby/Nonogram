#include "nonogram.hpp"

#include <functional>
#include <iterator>
#include <optional>

#include <cassert>
#include <queue>
#include <sstream>
#include <string>
#include <utility>

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

template <typename I, typename V>
concept forward_iterator_for_value =
    std::forward_iterator<I> &&
    std::same_as<typename std::iterator_traits<I>::value_type, V>;

template <typename RuleIter>
  requires forward_iterator_for_value<RuleIter, Rule>
void reverse_fit(int line_size, RuleIter rules_begin, RuleIter rules_end,
                 std::vector<int> &fit) {
  assert(rules_end - rules_begin == fit.size());
  for (int i = 0; i < fit.size(); ++i) {
    fit[i] = line_size - fit[i] - *(rules_begin++);
  }
  std::reverse(fit.begin(), fit.end());
}

template <typename RuleIter>
  requires forward_iterator_for_value<RuleIter, Rule>
std::vector<int> make_lfit_from_rules_range(RuleIter begin, RuleIter end) {
  std::vector<int> lfit;
  int cur_pos = 0;
  for (; begin != end; ++begin) {
    lfit.push_back(cur_pos);
    cur_pos += *begin + 1;
  }
  return lfit;
}

std::vector<int> make_lfit_from_rules(const RulesLine &rules) {
  return make_lfit_from_rules_range(rules.begin(), rules.end());
}

std::vector<int> make_rfit_from_rules(const int line_size,
                                      const RulesLine &rules) {
  auto fit = make_lfit_from_rules_range(rules.rbegin(), rules.rend());
  reverse_fit(line_size, rules.rbegin(), rules.rend(), fit);
  return fit;
}

SolutionLine::SolutionLine(int size, const RulesLine &rules)
    : m_rules(rules), m_cells(size, Cell::UNKNOWN), m_solved_flg(false),
      m_lfit(rules.size()), m_rfit(rules.size()), m_lfit_reversed(rules.size()),
      m_rfit_reversed(rules.size()) {

  update_fits(make_lfit_from_rules(rules), make_rfit_from_rules(size, rules));
}

void SolutionLine::update_fits(std::vector<int> &&lfit,
                               std::vector<int> &&rfit) {
  m_lfit = std::move(lfit);
  m_rfit = std::move(rfit);
  m_lfit_reversed = m_rfit;
  reverse_fit(size(), m_rules.begin(), m_rules.end(), m_lfit_reversed);
  m_rfit_reversed = m_lfit;
  reverse_fit(size(), m_rules.begin(), m_rules.end(), m_rfit_reversed);
}

const size_t SolutionLine::size() const { return m_cells.size(); }

Solution::Solution(int width, int height,
                   const std::vector<RulesLine> &vertical_rules,
                   const std::vector<RulesLine> &horizontal_rules)
    : m_width(width), m_height(height), m_is_final(false), m_n_solved_cells(0) {
  for (int i = 0; i < m_height; ++i) {
    m_rows_.emplace_back(width, horizontal_rules[i]);
  }
  for (int i = 0; i < m_width; ++i) {
    m_columns_.emplace_back(height, vertical_rules[i]);
  }
}

const Cell Solution::get_cell(int i, int j) const {
  return m_rows_[i].m_cells[j];
}
void Solution::set_cell(int i, int j, Cell value) {
  if (value != Cell::UNKNOWN && (m_rows_[i].m_cells[j] == Cell::UNKNOWN ||
                                 m_columns_[j].m_cells[i] == Cell::UNKNOWN)) {
    ++m_n_solved_cells;
  }
  m_rows_[i].m_cells[j] = value;
  m_columns_[j].m_cells[i] = value;
}

void Solution::set_row(int i, const CellsLine &line, std::vector<int> &&lfit,
                       std::vector<int> &&rfit) {
  for (int j = 0; j < line.size(); ++j) {
    set_cell(i, j, line[j]);
  }
  m_rows_[i].update_fits(std::move(lfit), std::move(rfit));
}

void Solution::set_column(int j, const CellsLine &line, std::vector<int> &&lfit,
                          std::vector<int> &&rfit) {
  for (int i = 0; i < line.size(); ++i) {
    set_cell(i, j, line[i]);
  }
  m_columns_[j].update_fits(std::move(lfit), std::move(rfit));
}

const SolutionLine &Solution::get_row(int i) const { return m_rows_[i]; }

const SolutionLine &Solution::get_column(int j) const { return m_columns_[j]; }

void Solution::mark_row_solved(int i) { m_rows_[i].m_solved_flg = true; }

void Solution::mark_column_solved(int j) { m_columns_[j].m_solved_flg = true; }

const bool Solution::is_row_solved(int i) const {
  return m_rows_[i].m_solved_flg;
}

const bool Solution::is_column_solved(int j) const {
  return m_columns_[j].m_solved_flg;
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
  for (const auto &row : solution.m_rows_) {
    for (auto v : row.m_cells) {
      os << print_cell(v)
         << print_cell(v); // print twice for better proportions
    }
    os << std::endl;
  }
}

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

template <typename RuleIter, typename CellIter, typename FitIter>
  requires forward_iterator_for_value<RuleIter, Rule> &&
           forward_iterator_for_value<CellIter, Cell> &&
           forward_iterator_for_value<FitIter, int>
struct FitIterState {
  FitIterState(std::vector<int> *cur_fit, RuleIter rule_i, RuleIter rule_end,
               CellIter cell_i, CellIter cell_end, FitIter lfit_i,
               FitIter lfit_end, FitIter rfit_i, FitIter rfit_end)
      : m_cur_fit(cur_fit), m_rule_i(rule_i), m_rule_end(rule_end),
        m_cell_i(cell_i), m_cell_end(cell_end), m_lfit_i(lfit_i),
        m_lfit_end(lfit_end), m_rfit_i(rfit_i), m_rfit_end(rfit_end),
        prev_cell_was_filled(false) {
    process_new_rule();
  }

  std::vector<int> *m_cur_fit;

  RuleIter m_rule_i;
  RuleIter m_rule_end;
  CellIter m_cell_i;
  CellIter m_cell_end;

  FitIter m_lfit_i;
  FitIter m_lfit_end;
  FitIter m_rfit_i;
  FitIter m_rfit_end;

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
      auto next_lfit = *(m_lfit_i + 1);
      auto jump_length = std::max(*m_rule_i + 1, next_lfit - m_cur_fit->back());
      return !range_has_filled_cells(m_cell_i + *m_rule_i,
                                     m_cell_i + jump_length);
    }
  }

  bool should_stop_cell_iter() {
    return m_cell_end - m_cell_i < min_required_space || prev_cell_was_filled ||
           m_cur_fit->back() > *m_rfit_i;
  }

  void next_cell() {
    prev_cell_was_filled = *m_cell_i == Cell::FILLED;
    ++m_cell_i;
    ++m_cur_fit->back();
  }

  bool should_stop_rule_iter() { return m_rule_i == m_rule_end; }

  void next_rule() {
    ++m_rule_i;
    ++m_lfit_i;
    ++m_rfit_i;

    process_new_rule();
  }

private:
  void process_new_rule() {
    if (should_stop_rule_iter()) {
      return;
    }

    if (m_cur_fit->empty()) {
      m_cell_i += *m_lfit_i;
      m_cur_fit->push_back(*m_lfit_i);
    } else {
      auto prev_rule = *(m_rule_i - 1);
      auto jump_length = std::max(prev_rule + 1, *m_lfit_i - m_cur_fit->back());
      m_cell_i += jump_length;
      m_cur_fit->push_back(m_cur_fit->back() + jump_length);
    }
    prev_cell_was_filled = false;

    is_last_rule = m_rule_i == m_rule_end - 1;

    min_required_space = is_last_rule ? *m_rule_i : (*m_rule_i + 1);
  }
};

template <typename RuleIter, typename CellIter, typename FitIter>
  requires forward_iterator_for_value<RuleIter, Rule> &&
           forward_iterator_for_value<CellIter, Cell> &&
           forward_iterator_for_value<FitIter, int>
bool fit_iter(std::vector<int> &cur_fit,
              FitIterState<RuleIter, CellIter, FitIter> &state) {
  if (state.should_stop_rule_iter()) {
    return true;
  }

  for (; !state.should_stop_cell_iter(); state.next_cell()) {
    if (!state.rule_fits()) {
      continue;
    }

    auto new_state = state;
    new_state.next_rule();
    if (fit_iter(cur_fit, new_state)) {
      return true;
    }
    cur_fit.pop_back();
  }

  return false;
}

std::optional<std::vector<int>> fit_left(const RulesLine &rules,
                                         const SolutionLine &line) {
  std::vector<int> fit;
  auto state = FitIterState(&fit, line.m_rules.begin(), line.m_rules.end(),
                            line.m_cells.begin(), line.m_cells.end(),
                            line.m_lfit.begin(), line.m_lfit.end(),
                            line.m_rfit.begin(), line.m_rfit.end());
  if (fit_iter(fit, state)) {
    return fit;
  }
  return std::nullopt;
}

std::optional<std::vector<int>> fit_right(const RulesLine &rules,
                                          const SolutionLine &line) {
  std::vector<int> fit;
  auto state =
      FitIterState(&fit, line.m_rules.rbegin(), line.m_rules.rend(),
                   line.m_cells.rbegin(), line.m_cells.rend(),
                   line.m_lfit_reversed.begin(), line.m_lfit_reversed.end(),
                   line.m_rfit_reversed.begin(), line.m_rfit_reversed.end());
  if (fit_iter(fit, state)) {
    reverse_fit(line.size(), line.m_rules.rbegin(), line.m_rules.rend(), fit);
    return fit;
  }
  return std::nullopt;
}

UpdateResult update_cells_from_empty_rules(CellsLine &&line) {
  if (range_has_filled_cells(line.begin(), line.end())) {
    return {.m_rules_fit = false,
            .m_n_updated_cells = 0,
            .m_line_solved = false,
            .m_cells = std::move(line)};
  }

  int n_updated_cells = 0;
  for (int i = 0; i < line.size(); ++i) {
    if (line[i] == Cell::UNKNOWN) {
      line[i] = Cell::EMPTY;
      ++n_updated_cells;
    }
  }
  return {.m_rules_fit = true,
          .m_n_updated_cells = n_updated_cells,
          .m_line_solved = true,
          .m_cells = std::move(line),
          .m_lfit = std::vector<int>(),
          .m_rfit = std::vector<int>()};
}

UpdateResult update_cells_from_lfit_and_rfit(const RulesLine &rules,
                                             CellsLine &&line,
                                             std::vector<int> &&lfit,
                                             std::vector<int> &&rfit) {
  int rule_i = 0;
  int intersect_left = rfit[0];
  int intersect_right = lfit[0] + rules[0];
  int prev_rule_rightmost_i = 0;
  bool line_solved = true;
  int n_updated_cells = 0;
  for (int i = 0; i < line.size(); ++i) {
    if (line_solved && rule_i < rules.size() && lfit[rule_i] != rfit[rule_i]) {
      line_solved = false;
    }

    if (prev_rule_rightmost_i <= i) {
      if ((rule_i == rules.size() && i < line.size()) || (i < lfit[rule_i])) {
        // cell cannot covered by any rule
        if (line[i] == Cell::UNKNOWN) {
          ++n_updated_cells;
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
      ++n_updated_cells;
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
          .m_n_updated_cells = n_updated_cells,
          .m_line_solved = line_solved,
          .m_cells = std::move(line),
          .m_lfit = std::move(lfit),
          .m_rfit = std::move(rfit)};
}

UpdateResult update_cells(const RulesLine &rules, const SolutionLine &line) {
  auto cells = line.m_cells;
  if (rules.empty()) {
    return update_cells_from_empty_rules(std::move(cells));
  }

  auto lfit_opt = fit_left(rules, line);
  if (!lfit_opt.has_value()) {
    return {
        .m_rules_fit = false, .m_n_updated_cells = 0, .m_line_solved = false};
  }
  auto rfit_opt = fit_right(rules, line);
  assert(rfit_opt.has_value());

  return update_cells_from_lfit_and_rfit(rules, std::move(cells),
                                         std::move(lfit_opt.value()),
                                         std::move(rfit_opt.value()));
}

struct SolveIterResult {
  Solution m_solution;
  int m_n_updated_cells;
  bool m_rules_fit;

  int get_n_solved_cells_before_last_update() const {
    return m_solution.m_n_solved_cells - m_n_updated_cells;
  }
};

SolveIterResult solve_iter(const Puzzle &puzzle, Solution &solution,
                           std::optional<int> max_n_iter = std::nullopt) {
  int n_updated_cells = 0;
  bool stop_iter = false;
  UpdateResult update_result;
  while (!stop_iter) {
    bool updated = false;

    for (int j = 0; j < puzzle.m_width; ++j) {
      if (solution.is_column_solved(j)) {
        continue;
      }
      const auto &column = solution.get_column(j);
      update_result = update_cells(puzzle.m_vertical_rules[j], column);
      if (!update_result.m_rules_fit) {
        return {.m_solution = solution,
                .m_n_updated_cells = n_updated_cells,
                .m_rules_fit = false};
      }
      if (update_result.m_line_solved) {
        solution.mark_column_solved(j);
      }
      updated = updated || update_result.m_n_updated_cells;
      n_updated_cells += update_result.m_n_updated_cells;
      solution.set_column(j, update_result.m_cells,
                          std::move(update_result.m_lfit.value()),
                          std::move(update_result.m_rfit.value()));
    }

    for (int i = 0; i < puzzle.m_height; ++i) {
      if (solution.is_row_solved(i)) {
        continue;
      }
      const auto &row = solution.get_row(i);
      update_result = update_cells(puzzle.m_horizontal_rules[i], row);
      if (!update_result.m_rules_fit) {
        return {.m_solution = solution,
                .m_n_updated_cells = n_updated_cells,
                .m_rules_fit = false};
      }
      if (update_result.m_line_solved) {
        solution.mark_row_solved(i);
      }
      updated = updated || update_result.m_n_updated_cells;
      n_updated_cells += update_result.m_n_updated_cells;
      solution.set_row(i, update_result.m_cells,
                       std::move(update_result.m_lfit.value()),
                       std::move(update_result.m_rfit.value()));
    }

    stop_iter = !updated;
    if (max_n_iter.has_value()) {
      --max_n_iter.value();
      stop_iter = stop_iter || max_n_iter.value() == 0;
    }
  }

  solution.m_is_final =
      solution.m_n_solved_cells == solution.m_width * solution.m_height;

  return {.m_solution = solution,
          .m_n_updated_cells = n_updated_cells,
          .m_rules_fit = true};
}

bool compare_solve_iter_results(const SolveIterResult &lt,
                                const SolveIterResult &rt) {
  if (lt.m_rules_fit != rt.m_rules_fit) {
    return !lt.m_rules_fit;
  }

  if (lt.get_n_solved_cells_before_last_update() !=
      rt.get_n_solved_cells_before_last_update()) {
    return lt.get_n_solved_cells_before_last_update() <
           rt.get_n_solved_cells_before_last_update();
  }

  return lt.m_n_updated_cells < rt.m_n_updated_cells;
}

using SolutionQueue = std::priority_queue<
    SolveIterResult, std::vector<SolveIterResult>,
    std::function<bool(const SolveIterResult &, const SolveIterResult &)>>;

void bfs(const Puzzle &puzzle, const SolveIterResult &cur_iter,
         SolutionQueue &queue) {
  const auto &solution = cur_iter.m_solution;
  for (int i = 0; i < solution.m_height; ++i) {
    for (int j = 0; j < solution.m_width; ++j) {
      if (solution.get_cell(i, j) != Cell::UNKNOWN) {
        continue;
      }

      for (auto fill_value : {Cell::FILLED, Cell::EMPTY}) {
        auto new_solution = solution;
        new_solution.set_cell(i, j, fill_value);
        queue.push(solve_iter(puzzle, new_solution, 2));
      }
    }
  }
}

Solution solve_puzzle(const Puzzle &puzzle) {
  Solution initial_solution(puzzle.m_width, puzzle.m_height,
                            puzzle.m_vertical_rules, puzzle.m_horizontal_rules);
  SolutionQueue queue(compare_solve_iter_results);
  queue.push(solve_iter(puzzle, initial_solution));
  while (!queue.empty()) {
    auto next = queue.top();
    queue.pop();
    if (!next.m_rules_fit) {
      // Exhausted all branches
      return next.m_solution;
    }
    if (next.m_solution.m_is_final) {
      // Found a solution
      return next.m_solution;
    }
    bfs(puzzle, next, queue);
  }
  std::unreachable();
}
