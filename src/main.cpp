#include "nonogram.hpp"

#include <boost/program_options.hpp>

#include <cstdlib>

namespace po = boost::program_options;

struct Options {
  bool quiet;
  bool benchmark;
  std::string input_file;
};

Options parse_options(int argc, char **argv) {
  po::variables_map vm;
  try {
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", po::bool_switch()->default_value(false),
                       "produce help message")(
        "quiet,q", po::bool_switch()->default_value(false), "quiet mode")(
        "benchmark,b", po::bool_switch()->default_value(false),
        "benchmark mode")("input-file", po::value<std::string>()->required(),
                          "input file");

    po::positional_options_description pos_desc;
    pos_desc.add("input-file", 1);

    po::store(po::command_line_parser(argc, argv)
                  .options(desc)
                  .positional(pos_desc)
                  .run(),
              vm);

    if (vm["help"].as<bool>()) {
      std::cout << desc << std::endl;
      exit(0);
    }

    po::notify(vm);
  } catch (const po::error &e) {
    std::cerr << e.what() << std::endl;
    exit(1);
  }

  return {
      .quiet = vm["quiet"].as<bool>(),
      .benchmark = vm["benchmark"].as<bool>(),
      .input_file = vm["input-file"].as<std::string>(),
  };
}

int main(int argc, char **argv) {
  auto options = parse_options(argc, argv);

  std::ifstream file(options.input_file);

  auto p = read_puzzle(file);
  if (!options.quiet) {
    print_puzzle(std::cout, p);
  }

  std::optional<Solution> s;
  if (options.benchmark) {
    auto begin = std::chrono::high_resolution_clock::now();
    s = solve_puzzle(p);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "solve_puzzle took "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end -
                                                                      begin)
                     .count()
              << " ns" << std::endl;
  } else {
    s = solve_puzzle(p);
  }

  assert(s.has_value());
  if (!options.quiet) {
    print_solution(std::cout, s.value());
  }

  return 0;
}
