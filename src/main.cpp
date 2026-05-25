#include "TspSolver.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct CliOptions {
    std::string method = "auto";
    std::string input_path;
    std::string batch_path;
    std::size_t exact_max_n = 12;
    bool debug = false;
    std::size_t debug_interval = 1000;
    tsp::BranchStrategy branch_strategy = tsp::BranchStrategy::Smart;
    tsp::HeuristicOptions heuristic;
};

struct RunResult {
    tsp::SolveResult result;
    std::string method;
    std::string problem_name;
    int dimension = 0;
    bool exact = false;
};

// 去掉 batch 清单行首尾空白，便于处理空行和注释行。
std::string trim(const std::string& text)
{
    const std::string whitespace = " \t\r\n";
    const std::size_t begin = text.find_first_not_of(whitespace);
    if (begin == std::string::npos) {
        return {};
    }
    const std::size_t end = text.find_last_not_of(whitespace);
    return text.substr(begin, end - begin + 1);
}

// CSV 字段中包含逗号、换行或双引号时，需要按 CSV 规则加引号并转义。
std::string csvQuote(const std::string& value)
{
    bool need_quote = false;
    for (char ch : value) {
        if (ch == '"' || ch == ',' || ch == '\n' || ch == '\r') {
            need_quote = true;
            break;
        }
    }

    if (!need_quote) {
        return value;
    }

    std::string quoted = "\"";
    for (char ch : value) {
        if (ch == '"') {
            quoted += "\"\"";
        } else {
            quoted += ch;
        }
    }
    quoted += '"';
    return quoted;
}

// 批处理 CSV 中用空字段表示 infinity，避免把不可行值写成普通数字。
std::string formatDouble(double value)
{
    if (!std::isfinite(value)) {
        return {};
    }
    std::ostringstream out;
    out << std::setprecision(10) << value;
    return out.str();
}

std::string formatTourLimited(const std::vector<int>& tour, std::size_t max_vertices = 80)
{
    if (tour.size() <= max_vertices) {
        return tsp::formatTour(tour);
    }

    std::ostringstream out;
    const std::size_t prefix = max_vertices / 2;
    const std::size_t suffix = max_vertices - prefix;
    for (std::size_t i = 0; i < prefix; ++i) {
        if (i != 0) {
            out << " -> ";
        }
        out << tour[i];
    }
    out << " -> ... -> ";
    for (std::size_t i = tour.size() - suffix; i < tour.size(); ++i) {
        if (i != tour.size() - suffix) {
            out << " -> ";
        }
        out << tour[i];
    }
    out << " -> " << tour.front();
    return out.str();
}

// 统一的单实例求解入口：自动识别矩阵或 TSPLIB，并按选项选择精确/启发式求解。
RunResult solveInput(std::istream& input, const CliOptions& options)
{
    tsp::TspProblem problem = tsp::readTspProblem(input);
    const int dimension = problem.dimension();
    const bool use_exact = options.method == "exact"
        || (options.method == "auto" && static_cast<std::size_t>(dimension) <= options.exact_max_n);

    RunResult output;
    output.method = use_exact ? "exact" : "heuristic";
    output.problem_name = problem.name;
    output.dimension = dimension;
    output.exact = use_exact;

    if (options.debug) {
        std::cerr << "[tsp-debug] problem loaded: name=" << output.problem_name
                  << " dimension=" << output.dimension
                  << " method=" << output.method << '\n';
    }

    if (use_exact) {
        auto distance = problem.toDenseMatrix(options.exact_max_n);
        tsp::BranchBoundSolver solver(std::move(distance));
        if (options.debug) {
            solver.setDebugOutput(std::cerr, options.debug_interval);
        }
        output.result = solver.solve(options.branch_strategy);
    } else {
        tsp::HeuristicOptions heuristic_options = options.heuristic;
        if (options.debug) {
            heuristic_options.debug.output = &std::cerr;
            heuristic_options.debug.interval = options.debug_interval;
        }
        output.result = tsp::solveHeuristic(problem, heuristic_options);
    }
    return output;
}

// 单实例模式使用人类可读输出，方便手动观察搜索统计。
void printHumanResult(const RunResult& run)
{
    const tsp::SolveResult& result = run.result;
    std::cout << std::setprecision(10);
    std::cout << "Problem: " << run.problem_name << '\n';
    std::cout << "Dimension: " << run.dimension << '\n';
    std::cout << "Method: " << run.method << '\n';

    if (run.exact) {
        std::cout << "Root lower bound: " << result.stats.root_lower_bound << '\n';
        std::cout << "Initial upper bound: " << result.stats.initial_upper_bound << '\n';
        std::cout << "Nodes created: " << result.stats.nodes_created << '\n';
        std::cout << "Nodes expanded: " << result.stats.nodes_expanded << '\n';
        std::cout << "Pruned by bound: " << result.stats.nodes_pruned_by_bound << '\n';
        std::cout << "Pruned infeasible: " << result.stats.nodes_pruned_infeasible << '\n';
    }

    if (!result.feasible) {
        std::cout << "No feasible Hamiltonian tour found.\n";
        return;
    }

    if (run.exact) {
        std::cout << "Optimal cost: " << result.cost << '\n';
    } else {
        std::cout << "Heuristic cost: " << result.cost << '\n';
    }
    std::cout << "Tour: " << formatTourLimited(result.tour) << '\n';
}

// 从文件读取一个实例并求解。
int runSingleFile(const std::string& path, const CliOptions& options)
{
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Failed to open input file: " << path << '\n';
        return 2;
    }
    std::cout << "Solving instance from file: " << path << '\n';
    std::cout.flush();
    const RunResult run = solveInput(file, options);
    printHumanResult(run);
    return run.result.feasible ? 0 : 1;
}

// 未传入文件时，从标准输入读取一个实例。
int runSingleStdin(const CliOptions& options)
{
    const RunResult run = solveInput(std::cin, options);
    printHumanResult(run);
    return run.result.feasible ? 0 : 1;
}

// 批处理模式输出 CSV 表头，便于重定向到结果文件后做统计分析。
void printBatchHeader()
{
    std::cout
        << "instance,status,method,dimension,cost,root_lower_bound,initial_upper_bound,"
        << "nodes_created,nodes_expanded,pruned_by_bound,pruned_infeasible,tour,message\n";
}

// 输出一条批处理记录。result 为空表示文件读取或解析阶段已经失败。
void printBatchRow(const std::string& path,
                   const std::string& status,
                   const RunResult* run,
                   const std::string& message)
{
    std::cout << csvQuote(path) << ','
              << status << ',';

    if (run == nullptr) {
        // 读取失败、解析失败等情况没有求解统计，只保留错误信息。
        std::cout << ",,,,,,,,,,"
                  << csvQuote(message) << '\n';
        return;
    }

    const tsp::SolveResult& result = run->result;
    // 有求解结果时，把成本、搜索统计和回路统一写成一行 CSV。
    std::cout << run->method << ','
              << run->dimension << ','
              << formatDouble(result.cost) << ','
              << formatDouble(result.stats.root_lower_bound) << ','
              << formatDouble(result.stats.initial_upper_bound) << ','
              << result.stats.nodes_created << ','
              << result.stats.nodes_expanded << ','
              << result.stats.nodes_pruned_by_bound << ','
              << result.stats.nodes_pruned_infeasible << ','
              << csvQuote(formatTourLimited(result.tour)) << ','
              << csvQuote(message) << '\n';
}

// 批处理清单每行一个实例路径；单个实例失败不会中断整个批次。
int runBatch(const std::string& list_path, const CliOptions& options)
{
    std::ifstream list_file(list_path);
    if (!list_file) {
        std::cerr << "Failed to open batch list: " << list_path << '\n';
        return 2;
    }

    std::vector<std::string> paths;
    std::string line;
    while (std::getline(list_file, line)) {
        std::string path = trim(line);
        // batch 文件允许用 # 写注释，方便记录数据集来源或分组。
        if (path.empty() || path[0] == '#') {
            continue;
        }
        paths.push_back(path);
    }

    if (paths.empty()) {
        std::cerr << "Batch list is empty: " << list_path << '\n';
        return 2;
    }

    bool all_ok = true;
    printBatchHeader();

    for (const std::string& path : paths) {
        try {
            // 每个实例独立打开和求解，避免一个坏文件影响后续实例。
            std::ifstream input(path);
            if (!input) {
                all_ok = false;
                printBatchRow(path, "error", nullptr, "failed to open input file");
                continue;
            }

            RunResult run = solveInput(input, options);
            if (run.result.feasible) {
                const std::string status = run.exact ? "ok" : "heuristic";
                const std::string message = run.exact ? std::string{} : "not proven optimal";
                printBatchRow(path, status, &run, message);
            } else {
                all_ok = false;
                printBatchRow(path, "infeasible", &run, "no feasible Hamiltonian tour");
            }
        } catch (const std::exception& ex) {
            all_ok = false;
            printBatchRow(path, "error", nullptr, ex.what());
        }
    }

    return all_ok ? 0 : 1;
}

// 命令行帮助信息。
void printUsage(const char* program)
{
    std::cerr << "Usage:\n"
              << "  " << program << " [options] [matrix-or-tsplib-file]\n"
              << "  " << program << " [options] --batch <list-file>\n"
              << "\nOptions:\n"
              << "  --method <auto|exact|heuristic>\n"
              << "  --branch-strategy <smart|exhaustive>\n"
              << "  --exact-max-n <n>\n"
              << "  --starts <n>\n"
              << "  --nearest-scan-limit <n>\n"
              << "  --full-two-opt-limit <n>\n"
              << "  --two-opt-window <n>\n"
              << "  --two-opt-passes <n>\n"
              << "  --seed <n>\n"
              << "  --debug\n"
              << "  --debug-interval <n>\n";
}

std::size_t parseSizeOption(const std::string& value, const std::string& name)
{
    std::size_t parsed = 0;
    const unsigned long long result = std::stoull(value, &parsed);
    if (parsed != value.size()) {
        throw std::runtime_error("invalid numeric value for " + name + ": " + value);
    }
    return static_cast<std::size_t>(result);
}

CliOptions parseArgs(int argc, char** argv)
{
    CliOptions options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        auto require_value = [&](const std::string& option_name) -> std::string {
            if (i + 1 >= argc) {
                throw std::runtime_error("missing value for " + option_name);
            }
            return argv[++i];
        };

        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            std::exit(0);
        } else if (arg == "--batch") {
            options.batch_path = require_value(arg);
        } else if (arg == "--branch-strategy") {
            const std::string value = require_value(arg);
            if (value == "smart") {
                options.branch_strategy = tsp::BranchStrategy::Smart;
            } else if (value == "exhaustive") {
                options.branch_strategy = tsp::BranchStrategy::Exhaustive;
            } else {
                throw std::runtime_error("--branch-strategy must be smart or exhaustive");
            }
        } else if (arg == "--method") {
            options.method = require_value(arg);
            if (options.method != "auto" && options.method != "exact" && options.method != "heuristic") {
                throw std::runtime_error("--method must be auto, exact, or heuristic");
            }
        } else if (arg == "--exact-max-n") {
            options.exact_max_n = parseSizeOption(require_value(arg), arg);
        } else if (arg == "--starts") {
            options.heuristic.starts = parseSizeOption(require_value(arg), arg);
        } else if (arg == "--nearest-scan-limit") {
            options.heuristic.nearest_scan_limit = parseSizeOption(require_value(arg), arg);
        } else if (arg == "--full-two-opt-limit") {
            options.heuristic.full_two_opt_limit = parseSizeOption(require_value(arg), arg);
        } else if (arg == "--two-opt-window") {
            options.heuristic.two_opt_window = parseSizeOption(require_value(arg), arg);
        } else if (arg == "--two-opt-passes") {
            options.heuristic.two_opt_passes = parseSizeOption(require_value(arg), arg);
        } else if (arg == "--seed") {
            options.heuristic.seed = static_cast<unsigned>(parseSizeOption(require_value(arg), arg));
        } else if (arg == "--debug") {
            options.debug = true;
        } else if (arg == "--debug-interval") {
            options.debug_interval = parseSizeOption(require_value(arg), arg);
        } else if (!arg.empty() && arg[0] == '-') {
            throw std::runtime_error("unknown option: " + arg);
        } else if (options.input_path.empty()) {
            options.input_path = arg;
        } else {
            throw std::runtime_error("multiple input files provided");
        }
    }
    return options;
}

} // namespace

int main(int argc, char** argv)
{
    try {
        const CliOptions options = parseArgs(argc, argv);
        if (!options.batch_path.empty()) {
            if (!options.input_path.empty()) {
                throw std::runtime_error("batch mode does not accept a separate input file");
            }
            return runBatch(options.batch_path, options);
        }
        if (!options.input_path.empty()) {
            return runSingleFile(options.input_path, options);
        }
        return runSingleStdin(options);
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 2;
    }

    return 0;
}
