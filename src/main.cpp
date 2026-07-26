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
    std::string input_path;
    std::string batch_path;
    std::size_t exact_max_n = 10000;
    bool debug = false;
    std::size_t debug_interval = 10000;
    // FULLHKMST 分支默认在每个搜索节点固定更新 32 轮，并把最后一轮
    // 1-tree 直接交给 BP。0.1% 窗口在实测中会因最终树/BP 顺序变化导致
    // 灾难性退化，因此默认只使用轮数停止；阈值仍可通过 CLI 显式实验。
    std::size_t branch_potential_updates = 32;
    double branch_potential_min_shrink_percent = 0.0;
    std::size_t branch_potential_max_depth =
        std::numeric_limits<std::size_t>::max();
    std::size_t branch_potential_depth_interval = 1;
    bool branch_potential_after_search_incumbent = false;
    std::size_t branch_potential_shrink_window = 8;
    bool reuse_last_one_tree_for_branching = true;
};

struct RunResult {
    tsp::SolveResult result;
    std::string method;
    std::string problem_name;
    int dimension = 0;
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
    out << std::setprecision(std::numeric_limits<double>::max_digits10) << value;
    return out.str();
}

std::string formatTourLimited(const std::vector<int>& tour, std::size_t max_vertices = 10000)
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

// 统一的单实例求解入口：自动识别矩阵或 TSPLIB，进行精确求解。
RunResult solveInput(std::istream& input, const CliOptions& options)
{
    tsp::TspProblem problem = tsp::readTspProblem(input);
    const int dimension = problem.dimension();
    RunResult output;
    output.method = "exact";
    output.problem_name = problem.name;
    output.dimension = dimension;

    if (options.debug) {
        std::cerr << "[tsp-debug] problem loaded: name=" << output.problem_name
                  << " dimension=" << output.dimension
                  << " method=" << output.method << '\n';
    }

    auto distance = problem.toDenseMatrix(options.exact_max_n);
    tsp::BranchBoundSolver solver(std::move(distance));
    solver.setBranchPotentialOptions(tsp::BranchPotentialOptions{
        options.branch_potential_updates,
        options.branch_potential_min_shrink_percent,
        options.branch_potential_max_depth,
        options.branch_potential_depth_interval,
        options.branch_potential_after_search_incumbent,
        options.branch_potential_shrink_window,
        options.reuse_last_one_tree_for_branching});
    if (options.debug) {
        solver.setDebugOutput(std::cerr, options.debug_interval);
    }
    output.result = solver.solve();
    return output;
}

// 单实例模式使用人类可读输出，方便手动观察搜索统计。
void printHumanResult(const RunResult& run)
{
    const tsp::SolveResult& result = run.result;
    // Preserve enough digits for an independently verified double result to
    // round-trip through the command-line output.
    std::cout << std::setprecision(std::numeric_limits<double>::max_digits10);
    std::cout << "Problem: " << run.problem_name << '\n';
    std::cout << "Dimension: " << run.dimension << '\n';
    std::cout << "Method: " << run.method << '\n';

    std::cout << "Root lower bound: " << result.stats.root_lower_bound << '\n';
    std::cout << "Initial upper bound: " << result.stats.initial_upper_bound << '\n';
    std::cout << "Nodes created: " << result.stats.nodes_created << '\n';
    std::cout << "Nodes expanded: " << result.stats.nodes_expanded << '\n';
    std::cout << "Pruned by bound: " << result.stats.nodes_pruned_by_bound << '\n';
    std::cout << "Pruned infeasible: " << result.stats.nodes_pruned_infeasible << '\n';
    std::cout << "Branch potential calls: "
              << result.stats.branch_potential_calls << '\n';
    std::cout << "Branch potential updates: "
              << result.stats.branch_potential_updates << '\n';
    std::cout << "Branch potential pruned: "
              << result.stats.branch_potential_pruned << '\n';
    std::cout << "Branch potential trees reused for BP: "
              << result.stats.branch_potential_tree_reused << '\n';
    std::cout << "Branch potential bound gain: "
              << result.stats.branch_potential_bound_gain << '\n';
    const double average_gap_shrink_percent =
        result.stats.branch_potential_calls == 0 ? 0.0
        : result.stats.branch_potential_gap_shrink_percent_sum
            / static_cast<double>(result.stats.branch_potential_calls);
    std::cout << "Branch potential average gap shrink percent: "
              << average_gap_shrink_percent << '\n';

    if (!result.feasible) {
        std::cout << "No feasible Hamiltonian tour found.\n";
        return;
    }

    std::cout << "Optimal cost: " << result.cost << '\n';
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
        << "nodes_created,nodes_expanded,pruned_by_bound,pruned_infeasible,"
        << "branch_potential_calls,branch_potential_updates,branch_potential_pruned,"
        << "branch_potential_tree_reused,"
        << "branch_potential_bound_gain,"
        << "branch_potential_average_gap_shrink_percent,"
        << "tour,message\n";
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
        std::cout << ",,,,,,,,,,,,,,,,"
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
              << result.stats.branch_potential_calls << ','
              << result.stats.branch_potential_updates << ','
              << result.stats.branch_potential_pruned << ','
              << result.stats.branch_potential_tree_reused << ','
              << formatDouble(result.stats.branch_potential_bound_gain) << ','
              << formatDouble(result.stats.branch_potential_calls == 0 ? 0.0
                     : result.stats.branch_potential_gap_shrink_percent_sum
                         / static_cast<double>(
                             result.stats.branch_potential_calls)) << ','
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
                printBatchRow(path, "ok", &run, "");
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
              << "  --exact-max-n <n>\n"
              << "  --debug\n"
              << "  --debug-interval <n>\n"
              << "  --branch-potential-updates <n>\n"
              << "  --branch-potential-min-shrink-percent <percent>\n"
              << "  --branch-potential-max-depth <n>\n"
              << "  --branch-potential-depth-interval <n>\n"
              << "  --branch-potential-after-search-incumbent\n"
              << "  --branch-potential-shrink-window <n>\n"
              << "  --full-hkmst\n"
              << "  --fixed-root-bp\n";
}

std::size_t parseSizeOption(const std::string& value, const std::string& name)
{
    if (value.empty()
        || !std::all_of(value.begin(), value.end(), [](char ch) { return ch >= '0' && ch <= '9'; })) {
        throw std::runtime_error("invalid numeric value for " + name + ": " + value);
    }

    std::size_t parsed = 0;
    unsigned long long result = 0;
    try {
        result = std::stoull(value, &parsed);
    } catch (const std::exception&) {
        throw std::runtime_error("invalid numeric value for " + name + ": " + value);
    }
    if (parsed != value.size()
        || result > static_cast<unsigned long long>(std::numeric_limits<std::size_t>::max())) {
        throw std::runtime_error("invalid numeric value for " + name + ": " + value);
    }
    return static_cast<std::size_t>(result);
}

double parseDoubleOption(const std::string& value, const std::string& name)
{
    std::size_t parsed = 0;
    double result = 0.0;
    try {
        result = std::stod(value, &parsed);
    } catch (const std::exception&) {
        throw std::runtime_error(
            "invalid numeric value for " + name + ": " + value);
    }
    if (parsed != value.size() || !std::isfinite(result)) {
        throw std::runtime_error(
            "invalid numeric value for " + name + ": " + value);
    }
    return result;
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
        } else if (arg == "--exact-max-n") {
            options.exact_max_n = parseSizeOption(require_value(arg), arg);
            if (options.exact_max_n == 0) {
                throw std::runtime_error("--exact-max-n must be greater than zero");
            }
        } else if (arg == "--debug") {
            options.debug = true;
        } else if (arg == "--debug-interval") {
            options.debug_interval = parseSizeOption(require_value(arg), arg);
            if (options.debug_interval == 0) {
                throw std::runtime_error("--debug-interval must be greater than zero");
            }
        } else if (arg == "--branch-potential-updates") {
            options.branch_potential_updates =
                parseSizeOption(require_value(arg), arg);
        } else if (arg == "--branch-potential-min-shrink-percent") {
            options.branch_potential_min_shrink_percent =
                parseDoubleOption(require_value(arg), arg);
            if (options.branch_potential_min_shrink_percent < 0.0) {
                throw std::runtime_error(
                    "--branch-potential-min-shrink-percent must be non-negative");
            }
        } else if (arg == "--branch-potential-max-depth") {
            options.branch_potential_max_depth =
                parseSizeOption(require_value(arg), arg);
        } else if (arg == "--branch-potential-depth-interval") {
            options.branch_potential_depth_interval =
                parseSizeOption(require_value(arg), arg);
            if (options.branch_potential_depth_interval == 0) {
                throw std::runtime_error(
                    "--branch-potential-depth-interval must be greater than zero");
            }
        } else if (arg == "--branch-potential-after-search-incumbent") {
            options.branch_potential_after_search_incumbent = true;
        } else if (arg == "--branch-potential-shrink-window") {
            options.branch_potential_shrink_window =
                parseSizeOption(require_value(arg), arg);
            if (options.branch_potential_shrink_window == 0) {
                throw std::runtime_error(
                    "--branch-potential-shrink-window must be greater than zero");
            }
        } else if (arg == "--full-hkmst") {
            options.reuse_last_one_tree_for_branching = true;
            if (options.branch_potential_updates == 0) {
                options.branch_potential_updates = 32;
            }
        } else if (arg == "--fixed-root-bp") {
            options.reuse_last_one_tree_for_branching = false;
            options.branch_potential_updates = 0;
            options.branch_potential_min_shrink_percent = 0.0;
        } else if (!arg.empty() && arg[0] == '-') {
            throw std::runtime_error("unknown option: " + arg);
        } else if (options.input_path.empty()) {
            options.input_path = arg;
        } else {
            throw std::runtime_error("multiple input files provided");
        }
    }
    if (options.branch_potential_min_shrink_percent > 0.0
        && options.branch_potential_updates == 0) {
        throw std::runtime_error(
            "--branch-potential-min-shrink-percent requires "
            "--branch-potential-updates greater than zero");
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
