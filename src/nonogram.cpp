#include "nonogram.hpp"

#include <optional>

#include <cassert>
#include <ranges>
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

template <typename Range, typename Value>
concept array_like_range_for_value =
    std::ranges::random_access_range<Range> &&
    std::ranges::sized_range<Range> &&
    std::same_as<std::ranges::range_value_t<Range>, Value>;

template <typename RulesRange>
  requires array_like_range_for_value<RulesRange, Rule>
void reverse_fit(int line_size, const RulesRange &rules,
                 std::vector<int> &fit) {
  assert(rules.size() == fit.size());
  for (int i = 0; i < fit.size(); ++i) {
    fit[i] = line_size - fit[i] - rules[i];
  }
  std::reverse(fit.begin(), fit.end());
}

template <typename RulesRange>
  requires array_like_range_for_value<RulesRange, Rule>
std::vector<int> make_lfit_from_rules(const RulesRange &rules) {
  std::vector<int> lfit;
  int cur_pos = 0;
  for (auto rule : rules) {
    lfit.push_back(cur_pos);
    cur_pos += rule + 1;
  }
  return lfit;
}

std::vector<int> make_rfit_from_rules(const int line_size,
                                      const RulesLine &rules) {
  auto reversed_rules = rules | std::views::reverse;
  auto fit = make_lfit_from_rules(reversed_rules);
  reverse_fit(line_size, reversed_rules, fit);
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
  reverse_fit(size(), m_rules, m_lfit_reversed);
  m_rfit_reversed = m_lfit;
  reverse_fit(size(), m_rules, m_rfit_reversed);
}

const size_t SolutionLine::size() const { return m_cells.size(); }

Solution::Solution(int width, int height,
                   const std::vector<RulesLine> &vertical_rules,
                   const std::vector<RulesLine> &horizontal_rules)
    : m_width(width), m_height(height), m_is_final(false) {
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

template <typename CellsRange>
  requires array_like_range_for_value<CellsRange, Cell>
bool range_has_filled_cells(CellsRange &&cells) {
  return std::ranges::find(std::forward<CellsRange>(cells), Cell::FILLED) !=
         std::ranges::end(cells);
}

template <typename CellsRange>
  requires array_like_range_for_value<CellsRange, Cell>
bool range_has_empty_cells(CellsRange &&cells) {
  return std::ranges::find(std::forward<CellsRange>(cells), Cell::EMPTY) !=
         std::ranges::end(cells);
}

struct FitDpTable {
  struct DpValue {
    bool can_fit;
    int index;
  };

  using DpValueOpt = std::optional<DpValue>;
  using DpRow = std::vector<DpValueOpt>;
  using DpTable = std::vector<DpRow>;

  FitDpTable(int n_rules, int n_cells)
      : values(n_rules + 1, DpRow(n_cells + 1, std::nullopt)) {}

  DpTable values;
};

// Right fit rules {rule_i, rule_{i+1}, ..., rule_{N-1}} into cells {cell_i,
// cell_{i+1}, ..., cell_{M-1}}
template <typename RulesRange, typename CellsRange, typename FitRange>
  requires array_like_range_for_value<RulesRange, Rule> &&
           array_like_range_for_value<CellsRange, Cell> &&
           array_like_range_for_value<FitRange, int>
void fit_dp_iter(FitDpTable &table, const RulesRange &rules,
                 const CellsRange &cells, const FitRange &lfit,
                 const FitRange &rfit, int rule_i, int cell_i) {
  auto &table_value = table.values[rule_i][cell_i];
  assert(!table_value.has_value());

  if (rule_i == rules.size()) {
    table_value = {
        .can_fit = !range_has_filled_cells(cells | std::views::drop(cell_i)),
        .index = -1}; // index does not matter
    return;
  }

  if (cell_i > rfit[rule_i]) {
    // not enough space for this block
    table_value = {.can_fit = false};
    return;
  }

  auto lower_bound = std::max(cell_i, lfit[rule_i]);
  if (range_has_filled_cells(cells | std::views::take(lower_bound) |
                             std::views::drop(cell_i))) {
    // got filled cells that cannot be covered
    table_value = {.can_fit = false};
    return;
  }

  auto current_rule = rules[rule_i];
  auto next_rule_i = rule_i + 1;
  auto is_last_rule = rule_i == rules.size() - 1;
  auto next_cell_i = rfit[rule_i] + current_rule + (is_last_rule ? 0 : 1);
  assert(next_cell_i <= cells.size());
  assert(next_rule_i <= rules.size());
  for (int i = rfit[rule_i]; i >= lower_bound; --i, --next_cell_i) {
    assert(cells.size() - i >= current_rule);
    if (range_has_filled_cells(cells | std::views::take(i) |
                               std::views::drop(cell_i))) {
      // uncovered filled cells remaining before block
      continue;
    }
    if (range_has_empty_cells(cells | std::views::drop(i) |
                              std::views::take(current_rule))) {
      // block covers empty cell
      continue;
    }
    if (i + current_rule < cells.size() &&
        cells[i + current_rule] == Cell::FILLED) {
      // block is next to a filled cell
      continue;
    }

    // block fits, trying to satisfy remaining rules
    auto &next_dp_value = table.values[next_rule_i][next_cell_i];
    if (!next_dp_value.has_value()) {
      fit_dp_iter(table, rules, cells, lfit, rfit, next_rule_i, next_cell_i);
    }
    assert(next_dp_value.has_value());

    if (next_dp_value->can_fit) {
      table_value = {.can_fit = true, .index = i};
      return;
    }
  }

  table_value = {.can_fit = false};
  return;
}

template <typename RulesRange>
  requires array_like_range_for_value<RulesRange, Rule>
std::vector<int> fit_dp_construct(const FitDpTable &table,
                                  const RulesRange &rules) {
  std::vector<int> fit;
  int rule_i = 0;
  int cell_i = 0;
  while (rule_i < rules.size()) {
    const auto &dp_value = table.values[rule_i][cell_i];
    assert(dp_value.has_value() && dp_value->can_fit);
    fit.push_back(dp_value->index);
    cell_i = dp_value->index + rules[rule_i] + 1;
    ++rule_i;
  }
  return fit;
}

std::optional<std::vector<int>> fit_left(const RulesLine &rules,
                                         const SolutionLine &line) {
  FitDpTable table(rules.size(), line.size());
  auto rules_reversed = rules | std::views::reverse;
  fit_dp_iter(table, rules_reversed, line.m_cells | std::views::reverse,
              line.m_lfit_reversed, line.m_rfit_reversed, 0, 0);
  assert(table.values[0][0].has_value());
  if (table.values[0][0]->can_fit) {
    auto fit = fit_dp_construct(table, rules_reversed);
    reverse_fit(line.size(), rules_reversed, fit);
    return fit;
  }
  return std::nullopt;
}

std::optional<std::vector<int>> fit_right(const RulesLine &rules,
                                          const SolutionLine &line) {
  FitDpTable table(rules.size(), line.size());
  fit_dp_iter(table, rules, line.m_cells, line.m_lfit, line.m_rfit, 0, 0);
  assert(table.values[0][0].has_value());
  if (table.values[0][0]->can_fit) {
    return fit_dp_construct(table, rules);
  }
  return std::nullopt;
}

UpdateResult update_cells_from_empty_rules(CellsLine &&line) {
  if (range_has_filled_cells(line)) {
    return {.m_rules_fit = false,
            .m_line_updated = false,
            .m_line_solved = false,
            .m_cells = std::move(line)};
  }

  bool line_updated = false;
  for (int i = 0; i < line.size(); ++i) {
    if (line[i] == Cell::UNKNOWN) {
      line[i] = Cell::EMPTY;
      line_updated = true;
    }
  }
  return {.m_rules_fit = true,
          .m_line_updated = line_updated,
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
  bool line_updated = false;
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
      if (rule_i < rules.size()) {
        intersect_left = rfit[rule_i];
        intersect_right = lfit[rule_i] + rules[rule_i];
      }
    }
  }

  return {.m_rules_fit = rule_i == rules.size(),
          .m_line_updated = line_updated,
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
        .m_rules_fit = false, .m_line_updated = false, .m_line_solved = false};
  }
  auto rfit_opt = fit_right(rules, line);
  assert(rfit_opt.has_value());

  return update_cells_from_lfit_and_rfit(rules, std::move(cells),
                                         std::move(lfit_opt.value()),
                                         std::move(rfit_opt.value()));
}

Solution solve_iter(const Puzzle &puzzle, Solution &solution) {
  bool updated = true;
  UpdateResult update_result;
  while (updated) {
    updated = false;

    for (int j = 0; j < puzzle.m_width; ++j) {
      if (solution.is_column_solved(j)) {
        continue;
      }
      const auto &column = solution.get_column(j);
      update_result = update_cells(puzzle.m_vertical_rules[j], column);
      if (!update_result.m_rules_fit) {
        return solution;
      }
      if (update_result.m_line_solved) {
        solution.mark_column_solved(j);
      }
      updated = updated || update_result.m_line_updated;
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
        return solution;
      }
      if (update_result.m_line_solved) {
        solution.mark_row_solved(i);
      }
      updated = updated || update_result.m_line_updated;
      solution.set_row(i, update_result.m_cells,
                       std::move(update_result.m_lfit.value()),
                       std::move(update_result.m_rfit.value()));
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
  Solution initial_solution(puzzle.m_width, puzzle.m_height,
                            puzzle.m_vertical_rules, puzzle.m_horizontal_rules);
  auto solution = solve_iter(puzzle, initial_solution);
  return solution;
}
