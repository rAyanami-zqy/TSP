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
    // 单实例文件路径；为空且未指定 batch_path 时从标准输入读取。
    std::string input_path;
    // 批处理清单路径；清单每个有效行保存一个实例文件路径。
    std::string batch_path;
    // 坐标实例展开为 n×n 稠密矩阵时允许的最大顶点数，防止意外耗尽内存。
    std::size_t exact_max_n = 10000;
    // 根节点 Held-Karp 势的上升算法；不影响问题可行域，只影响下界强度。
    tsp::RootAscentStrategy root_ascent = tsp::RootAscentStrategy::Polyak;
    // 搜索节点一次势更新内部的步长调度；触发和 epoch 语义由下方策略控制。
    tsp::NodeAscentStrategy node_ascent = tsp::NodeAscentStrategy::Polyak;
    // 默认沿用调整权重排序；实验策略只切换 BP 内部的分支边优先级，
    // 不改变 1-tree 下界或 Kruskal 候选顺序。
    tsp::BranchEdgeOrder branch_edge_order
        = tsp::BranchEdgeOrder::AdjustedWeight;
    // 搜索节点是否更新势，以及更新势只作临时证书还是安装为子树 epoch。
    tsp::PotentialUpdateStrategy potential_update
        = tsp::PotentialUpdateStrategy::None;
    // Depth 模式的触发间隔；Adaptive 模式的最小深度；subtree 模式下还
    // 表示当前节点距离上一势 epoch 至少要相隔多少层。
    std::size_t potential_update_depth = 4;
    // 每次节点势更新最多执行的次梯度迭代轮数。
    std::size_t potential_update_iterations = 8;
    // Adaptive 模式允许触发更新的最大相对 gap：(UB-LB)/max(1,|UB|)。
    double potential_update_gap_ratio = 0.05;
    // 一轮精确搜索允许尝试的节点势更新次数；根重启后重新计数。
    std::size_t potential_update_budget = 1000;
    // 为 true 时只报告启发式上界和根 1-tree 下界，不进入 BP 精确搜索。
    bool root_bound_only = false;
    // debug 输出写入 stderr；interval 表示每扩展多少个节点输出一次进度。
    bool debug = false;
    std::size_t debug_interval = 10000;
};

struct RunResult {
    // 求解器返回的 tour、成本和搜索统计。
    tsp::SolveResult result;
    // 输出标签：exact 或 root-bound。
    std::string method;
    // 解析后的实例名与顶点数，避免输出阶段再次访问已释放的 TspProblem。
    std::string problem_name;
    int dimension = 0;
};

// 去掉 batch 清单行首尾空白，便于处理空行和注释行。
std::string trim(const std::string& text)
{
    // begin/end 均为原字符串下标；全空白字符串由 npos 分支单独处理。
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
    // 超长 tour 只保留首尾两段；prefix + suffix 恒等于显示上限。
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
    output.method = options.root_bound_only ? "root-bound" : "exact";
    output.problem_name = problem.name;
    output.dimension = dimension;

    if (options.debug) {
        std::cerr << "[tsp-debug] problem loaded: name=" << output.problem_name
                  << " dimension=" << output.dimension
                  << " method=" << output.method << '\n';
    }

    auto distance = problem.toDenseMatrix(options.exact_max_n);
    tsp::BranchBoundSolver solver(std::move(distance));
    solver.setRootAscentStrategy(options.root_ascent);
    solver.setNodeAscentStrategy(options.node_ascent);
    // 分支顺序与势更新策略是两个正交开关，便于分别评估搜索树形状和下界质量。
    solver.setBranchEdgeOrder(options.branch_edge_order);
    solver.setPotentialUpdateOptions(
        options.potential_update,
        options.potential_update_depth,
        options.potential_update_iterations,
        options.potential_update_gap_ratio,
        options.potential_update_budget);
    solver.setRootBoundOnly(options.root_bound_only);
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
    std::cout << "Potential updates attempted: "
              << result.stats.potential_updates_attempted << '\n';
    std::cout << "Potential updates improved: "
              << result.stats.potential_updates_improved << '\n';
    std::cout << "Potential updates pruned: "
              << result.stats.potential_updates_pruned << '\n';
    std::cout << "Potential updates rebuilt: "
              << result.stats.potential_updates_rebuilt << '\n';
    std::cout << "Potential updates stopped prunable: "
              << result.stats.potential_updates_stopped_prunable << '\n';
    std::cout << "Potential update iterations: "
              << result.stats.potential_update_iterations << '\n';
    std::cout << "Potential update seconds: "
              << result.stats.potential_update_seconds << '\n';
    std::cout << "Potential update rebuild seconds: "
              << result.stats.potential_update_rebuild_seconds << '\n';
    std::cout << "Potential update total gain: "
              << result.stats.potential_update_total_gain << '\n';
    std::cout << "Potential update max gain: "
              << result.stats.potential_update_max_gain << '\n';

    if (!result.feasible) {
        std::cout << "No feasible Hamiltonian tour found.\n";
        return;
    }

    if (run.method == "root-bound") {
        std::cout << "Heuristic upper bound: " << result.cost << '\n';
        std::cout << "Heuristic tour: " << formatTourLimited(result.tour) << '\n';
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
        << "potential_updates_attempted,potential_updates_improved,"
        << "potential_updates_pruned,potential_updates_rebuilt,"
        << "potential_updates_stopped_prunable,"
        << "potential_update_iterations,potential_update_seconds,"
        << "potential_update_rebuild_seconds,potential_update_total_gain,"
        << "potential_update_max_gain,tour,message\n";
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
        std::cout << ",,,,,,,,,,,,,,,,,,,,"
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
              << result.stats.potential_updates_attempted << ','
              << result.stats.potential_updates_improved << ','
              << result.stats.potential_updates_pruned << ','
              << result.stats.potential_updates_rebuilt << ','
              << result.stats.potential_updates_stopped_prunable << ','
              << result.stats.potential_update_iterations << ','
              << formatDouble(result.stats.potential_update_seconds) << ','
              << formatDouble(result.stats.potential_update_rebuild_seconds) << ','
              << formatDouble(result.stats.potential_update_total_gain) << ','
              << formatDouble(result.stats.potential_update_max_gain) << ','
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
              << "  --hk-ascent <none|polyak|helsgaun|hybrid>\n"
              << "  --hk-node-ascent <polyak|helsgaun>\n"
              << "  --branch-edge-order <weight|root-alpha-asc|root-alpha-desc|"
                 "forbid-delta-asc|forbid-delta-desc|forbid-degree-desc|"
                 "root-frequency-middle|strong-top2|weight-desc|"
                 "max-degree-all-weight|excess-cover-weight|"
                 "local-excess-cover-weight|max-degree-excess-weight|"
                 "propagation-weight|forced-degree-weight|"
                 "max-degree-min-undecided|max-degree-max-undecided>\n"
              << "  --hk-potential-update <none|depth|adaptive|subtree-depth|subtree-adaptive>\n"
              << "  --hk-update-depth <n>\n"
              << "  --hk-update-iterations <n>\n"
              << "  --hk-update-gap-ratio <x>\n"
              << "  --hk-update-budget <n>\n"
              << "  --root-bound-only\n"
              << "  --debug\n"
              << "  --debug-interval <n>\n";
}

tsp::RootAscentStrategy parseRootAscentStrategy(const std::string& value)
{
    if (value == "none") return tsp::RootAscentStrategy::None;
    if (value == "polyak") return tsp::RootAscentStrategy::Polyak;
    if (value == "helsgaun") return tsp::RootAscentStrategy::Helsgaun;
    if (value == "hybrid") return tsp::RootAscentStrategy::Hybrid;
    throw std::runtime_error(
        "invalid value for --hk-ascent: " + value
        + " (expected none, polyak, helsgaun, or hybrid)");
}

tsp::NodeAscentStrategy parseNodeAscentStrategy(const std::string& value)
{
    if (value == "polyak") return tsp::NodeAscentStrategy::Polyak;
    if (value == "helsgaun") return tsp::NodeAscentStrategy::Helsgaun;
    throw std::runtime_error(
        "invalid value for --hk-node-ascent: " + value
        + " (expected polyak or helsgaun)");
}

tsp::PotentialUpdateStrategy parsePotentialUpdateStrategy(
    const std::string& value)
{
    if (value == "none") return tsp::PotentialUpdateStrategy::None;
    if (value == "depth") return tsp::PotentialUpdateStrategy::Depth;
    if (value == "adaptive") return tsp::PotentialUpdateStrategy::Adaptive;
    if (value == "subtree-depth") {
        return tsp::PotentialUpdateStrategy::SubtreeDepth;
    }
    if (value == "subtree-adaptive") {
        return tsp::PotentialUpdateStrategy::SubtreeAdaptive;
    }
    throw std::runtime_error(
        "invalid value for --hk-potential-update: " + value
        + " (expected none, depth, adaptive, subtree-depth, or subtree-adaptive)");
}

tsp::BranchEdgeOrder parseBranchEdgeOrder(const std::string& value)
{
    // 这里只选择 BP 的边比较器；root alpha 表由求解器在根势优化完成后构建。
    if (value == "weight") return tsp::BranchEdgeOrder::AdjustedWeight;
    if (value == "root-alpha-asc") {
        return tsp::BranchEdgeOrder::RootAlphaAscending;
    }
    if (value == "root-alpha-desc") {
        return tsp::BranchEdgeOrder::RootAlphaDescending;
    }
    if (value == "weight-desc") {
        return tsp::BranchEdgeOrder::AdjustedWeightDescending;
    }
    if (value == "max-degree-all-weight") {
        return tsp::BranchEdgeOrder::MaximumDegreeAllAdjustedWeight;
    }
    if (value == "excess-cover-weight") {
        return tsp::BranchEdgeOrder::MaximumExcessCoverAdjustedWeight;
    }
    if (value == "local-excess-cover-weight") {
        return tsp::BranchEdgeOrder::LocalExcessCoverAdjustedWeight;
    }
    if (value == "max-degree-excess-weight") {
        return tsp::BranchEdgeOrder::MaximumDegreeExcessCoverAdjustedWeight;
    }
    if (value == "propagation-weight") {
        return tsp::BranchEdgeOrder::PropagationPotentialAdjustedWeight;
    }
    if (value == "forced-degree-weight") {
        return tsp::BranchEdgeOrder::ForcedDegreeAdjustedWeight;
    }
    if (value == "max-degree-min-undecided") {
        return tsp::BranchEdgeOrder::MaximumDegreeMinimumUndecided;
    }
    if (value == "max-degree-max-undecided") {
        return tsp::BranchEdgeOrder::MaximumDegreeMaximumUndecided;
    }
    if (value == "forbid-delta-desc") {
        return tsp::BranchEdgeOrder::CurrentForbidDeltaDescending;
    }
    if (value == "forbid-delta-asc") {
        return tsp::BranchEdgeOrder::CurrentForbidDeltaAscending;
    }
    if (value == "forbid-degree-desc") {
        return tsp::BranchEdgeOrder::CurrentForbidDeltaDegreeAware;
    }
    if (value == "root-frequency-middle") {
        return tsp::BranchEdgeOrder::RootOneTreeFrequencyMiddle;
    }
    if (value == "strong-top2") {
        return tsp::BranchEdgeOrder::TwoSidedStrongBranchingTop2;
    }
    throw std::runtime_error(
        "invalid value for --branch-edge-order: " + value
        + " (expected weight, root-alpha-asc, root-alpha-desc, "
          "forbid-delta-asc, forbid-delta-desc, forbid-degree-desc, "
          "root-frequency-middle, strong-top2, weight-desc, "
          "max-degree-all-weight, excess-cover-weight, "
          "local-excess-cover-weight, max-degree-excess-weight, or "
          "propagation-weight, forced-degree-weight, "
          "max-degree-min-undecided, or max-degree-max-undecided)");
}

double parseDoubleOption(const std::string& value, const std::string& name)
{
    std::size_t parsed = 0;
    double result = 0.0;
    try {
        result = std::stod(value, &parsed);
    } catch (const std::exception&) {
        throw std::runtime_error("invalid numeric value for " + name + ": " + value);
    }
    if (parsed != value.size() || !std::isfinite(result)) {
        throw std::runtime_error("invalid numeric value for " + name + ": " + value);
    }
    return result;
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
        } else if (arg == "--hk-ascent") {
            options.root_ascent = parseRootAscentStrategy(require_value(arg));
        } else if (arg == "--hk-node-ascent") {
            options.node_ascent = parseNodeAscentStrategy(require_value(arg));
        } else if (arg == "--branch-edge-order") {
            options.branch_edge_order =
                parseBranchEdgeOrder(require_value(arg));
        } else if (arg == "--hk-potential-update") {
            options.potential_update =
                parsePotentialUpdateStrategy(require_value(arg));
        } else if (arg == "--hk-update-depth") {
            options.potential_update_depth = parseSizeOption(require_value(arg), arg);
            if (options.potential_update_depth == 0) {
                throw std::runtime_error("--hk-update-depth must be greater than zero");
            }
        } else if (arg == "--hk-update-iterations") {
            options.potential_update_iterations = parseSizeOption(require_value(arg), arg);
            if (options.potential_update_iterations == 0) {
                throw std::runtime_error("--hk-update-iterations must be greater than zero");
            }
        } else if (arg == "--hk-update-gap-ratio") {
            options.potential_update_gap_ratio = parseDoubleOption(require_value(arg), arg);
            if (options.potential_update_gap_ratio < 0.0) {
                throw std::runtime_error("--hk-update-gap-ratio must be non-negative");
            }
        } else if (arg == "--hk-update-budget") {
            options.potential_update_budget = parseSizeOption(require_value(arg), arg);
            if (options.potential_update_budget == 0) {
                throw std::runtime_error("--hk-update-budget must be greater than zero");
            }
        } else if (arg == "--root-bound-only") {
            options.root_bound_only = true;
        } else if (arg == "--debug") {
            options.debug = true;
        } else if (arg == "--debug-interval") {
            options.debug_interval = parseSizeOption(require_value(arg), arg);
            if (options.debug_interval == 0) {
                throw std::runtime_error("--debug-interval must be greater than zero");
            }
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
