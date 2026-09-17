#include "TspSolver.hpp"
#include "LkhProvider.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {

enum class LkhProviderPiMode {
    Off,
    Replace,
    WarmStart,
};

enum class LkhProviderFailureMode {
    Error,
    Fallback,
};

struct CliOptions {
    // 单实例文件路径；为空且未指定 batch_path 时从标准输入读取。
    std::string input_path;
    // 批处理清单路径；清单每个有效行保存一个实例文件路径。
    std::string batch_path;
    // 外部 tour 文件：n 后接 n 个 0-based 顶点编号，适用于单实例。
    std::string initial_tour_path;
    // LKH PI_FILE 或同格式节点势；ID 为 1-based，读入后除以 scale。
    std::string root_pi_path;
    double root_pi_scale = 100.0;
    tsp::RootPotentialSeedStrategy root_pi_mode
        = tsp::RootPotentialSeedStrategy::WarmStart;
    // 外部势 warm start 固定使用独立的小预算算法，不受普通根上升配置影响。
    tsp::RootAscentStrategy root_pi_refine_ascent
        = tsp::RootAscentStrategy::Polyak;
    std::size_t root_pi_refine_iterations = 64;
    double root_pi_refine_smoothing_current_weight = 0.7;
    double root_pi_refine_dynamic_cosine_scale = 0.2;
    double root_pi_refine_dynamic_min_current_weight = 0.5;
    double root_pi_refine_dynamic_max_current_weight = 0.9;
    // LKH PI_FILE 的第一条节点记录是其最终 1-tree 特殊根；默认把该节点
    // 重标号为内部顶点 0，避免同一组势因根不一致损失下界。
    bool root_pi_relabel_root = true;
    // 可选的本地 LKH provider。auto 使用同一 CMake 构建产物；provider
    // 在批处理中长驻，但每次 LKH 调用由隔离子进程完成。
    std::string lkh_provider_path;
    std::size_t lkh_provider_runs = 1;
    std::size_t lkh_provider_max_trials = 0;
    unsigned long long lkh_provider_seed = 1;
    double lkh_provider_time_limit_seconds = 0.0;
    LkhProviderPiMode lkh_provider_pi_mode =
        LkhProviderPiMode::WarmStart;
    LkhProviderFailureMode lkh_provider_failure_mode =
        LkhProviderFailureMode::Error;
    // 坐标实例展开为 n×n 稠密矩阵时允许的最大顶点数，防止意外耗尽内存。
    std::size_t exact_max_n = 10000;
    // 根节点 Held-Karp 势的上升算法；不影响问题可行域，只影响下界强度。
    tsp::RootAscentStrategy root_ascent = tsp::RootAscentStrategy::Polyak;
    // 每个根势阶段的最大评估轮数；默认保持求解器原有的 400。
    std::size_t root_ascent_iterations = 400;
    // Polyak 方向平滑中当前次梯度的固定/正交基准权重。
    double root_ascent_smoothing_current_weight = 0.7;
    // 动态方向平滑的余弦缩放以及最终当前次梯度权重上下限。
    double root_ascent_dynamic_cosine_scale = 0.2;
    double root_ascent_dynamic_min_current_weight = 0.5;
    double root_ascent_dynamic_max_current_weight = 0.9;
    // 可选逐轮 CSV。只允许与 root-bound-only 的单实例模式一起使用。
    std::string root_ascent_trace_path;
    // 搜索节点一次势更新内部的步长调度；触发和 epoch 语义由下方策略控制。
    tsp::NodeAscentStrategy node_ascent = tsp::NodeAscentStrategy::Polyak;
    // 节点改良 Polyak 的固定/正交基准权重、余弦缩放和动态上下限。
    double node_ascent_smoothing_current_weight = 0.7;
    double node_ascent_dynamic_cosine_scale = 0.2;
    double node_ascent_dynamic_min_current_weight = 0.5;
    double node_ascent_dynamic_max_current_weight = 0.9;
    // 默认沿用调整权重排序；实验策略只切换 BP 内部的分支边优先级，
    // 不改变 1-tree 下界或 Kruskal 候选顺序。
    tsp::BranchEdgeOrder branch_edge_order
        = tsp::BranchEdgeOrder::AdjustedWeight;
    // 搜索节点是否更新势；启用时新势始终安装为子树 epoch。
    tsp::PotentialUpdateStrategy potential_update
        = tsp::PotentialUpdateStrategy::None;
    // 当前节点距离上一势 epoch 至少要相隔多少层。
    std::size_t potential_update_depth = 4;
    // 节点势上升最大绝对 DFS 深度；0 表示不限制。
    std::size_t potential_update_max_depth = 0;
    // 剩余所需 tour 边数不超过该值时跳过势上升；0 表示关闭。
    std::size_t potential_update_skip_last_edges = 0;
    // 每次节点势更新最多执行的次梯度迭代轮数。
    std::size_t potential_update_iterations = 8;
    // GAPMST 默认要求 0.0001；显式设为 0 可复现 CPHKMST 触发行为。
    double potential_update_min_gap_change_ratio = 0.0001;
    // GAPMST 默认保护前 2 个绝对 DFS 层；0 表示从深度 1 起应用。
    std::size_t potential_update_gap_change_start_depth = 2;
    // 可选的大 gap 分档起点和迭代上限；iterations=0 表示关闭分档。
    double potential_update_large_gap_ratio = 0.0;
    std::size_t potential_update_large_gap_iterations = 0;
    // 一轮精确搜索允许尝试的节点势更新次数；根重启后重新计数，0 不限。
    std::size_t potential_update_budget = 1000;
    // 两阶段筛选默认关闭；正数表示完整上升前先观察多少次实际势更新。
    std::size_t potential_update_probe_updates = 0;
    // 初始相对 gap 大于该值时才执行 probe，较小 gap 直接完成全部迭代。
    double potential_update_probe_min_gap_ratio = 0.0;
    // probe 最强证书必须覆盖原 gap 的最小比例，否则丢弃该次更新。
    double potential_update_probe_min_coverage = 0.0;
    // 为 true 时只报告启发式上界和根 1-tree 下界，不进入 BP 精确搜索。
    bool root_bound_only = false;
    // 根 fixing 后默认把剩余候选重建成紧凑搜索 epoch；off 复现旧数据结构。
    bool root_candidate_compaction = true;
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
    // 从读取实例到 solve() 返回的单实例墙钟时间；批处理逐行输出该值。
    double instance_wall_seconds = 0.0;
    std::size_t lkh_provider_calls = 0;
    std::size_t lkh_provider_failures = 0;
    double lkh_provider_seconds = 0.0;
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
RunResult solveInput(std::istream& input,
                     const std::string& source_path,
                     const CliOptions& options,
                     tsp::LkhProvider* lkh_provider)
{
    const auto started_at = std::chrono::steady_clock::now();
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

    std::vector<int> supplied_tour;
    std::vector<double> root_potentials;
    int external_root = 0;
    bool lkh_tour_supplied = false;
    if (lkh_provider != nullptr) {
        ++output.lkh_provider_calls;
        if (options.debug) {
            // 先记录已开始的调用；若外层超时恰好发生在 provider 内部，
            // 实验驱动仍能恢复“确实调用过 LKH”，而不是把它误记为未调用。
            std::cerr << "[tsp-debug] LKH provider: calls="
                      << output.lkh_provider_calls
                      << " failures=" << output.lkh_provider_failures
                      << " seconds=" << output.lkh_provider_seconds << '\n';
        }
        const auto provider_started_at = std::chrono::steady_clock::now();
        try {
            tsp::LkhProviderOptions provider_options;
            provider_options.runs = options.lkh_provider_runs;
            provider_options.max_trials = options.lkh_provider_max_trials;
            provider_options.seed = options.lkh_provider_seed;
            provider_options.time_limit_seconds =
                options.lkh_provider_time_limit_seconds;
            provider_options.produce_potentials =
                options.lkh_provider_pi_mode != LkhProviderPiMode::Off;
            provider_options.potential_scale = options.root_pi_scale;
            tsp::LkhProviderResult provider_result =
                lkh_provider->run(source_path, dimension, provider_options);
            lkh_tour_supplied = !provider_result.tour.empty();
            supplied_tour = std::move(provider_result.tour);
            root_potentials = std::move(provider_result.potentials);
            external_root = provider_result.potential_root;
            output.lkh_provider_seconds = provider_result.provider_seconds;
        } catch (const std::exception& ex) {
            ++output.lkh_provider_failures;
            output.lkh_provider_seconds = std::chrono::duration<double>(
                std::chrono::steady_clock::now() - provider_started_at).count();
            if (options.debug) {
                std::cerr << "[tsp-debug] LKH provider: calls="
                          << output.lkh_provider_calls
                          << " failures=" << output.lkh_provider_failures
                          << " seconds=" << output.lkh_provider_seconds << '\n';
            }
            if (options.lkh_provider_failure_mode
                == LkhProviderFailureMode::Error) {
                throw;
            }
            if (options.debug) {
                std::cerr << "[tsp-debug] LKH provider failed; using internal "
                             "initialization: " << ex.what() << '\n';
            }
        }
        if (options.debug) {
            std::cerr << "[tsp-debug] LKH provider: calls="
                      << output.lkh_provider_calls
                      << " failures=" << output.lkh_provider_failures
                      << " seconds=" << output.lkh_provider_seconds << '\n';
        }
    }
    if (!options.initial_tour_path.empty()) {
        std::ifstream tour_input(options.initial_tour_path);
        long long count = -1;
        if (!(tour_input >> count) || count != dimension) {
            throw std::runtime_error("initial tour file must start with the instance dimension");
        }
        supplied_tour.resize(static_cast<std::size_t>(dimension));
        for (int& vertex : supplied_tour) {
            if (!(tour_input >> vertex)) throw std::runtime_error("incomplete initial tour file");
        }
        tour_input >> std::ws;
        if (!tour_input.eof()) throw std::runtime_error("extra data in initial tour file");
    }
    if (!options.root_pi_path.empty()) {
        std::ifstream pi_input(options.root_pi_path);
        long long count = -1;
        if (!(pi_input >> count) || count != dimension) {
            throw std::runtime_error(
                "root PI file must start with the instance dimension");
        }
        root_potentials.assign(static_cast<std::size_t>(dimension), 0.0);
        std::vector<unsigned char> seen(
            static_cast<std::size_t>(dimension), 0);
        for (int index = 0; index < dimension; ++index) {
            long long id = 0;
            double value = 0.0;
            if (!(pi_input >> id >> value) || id < 1 || id > dimension
                || seen[static_cast<std::size_t>(id - 1)]) {
                throw std::runtime_error(
                    "root PI file contains an invalid or duplicate node ID");
            }
            if (index == 0) external_root = static_cast<int>(id - 1);
            seen[static_cast<std::size_t>(id - 1)] = 1;
            root_potentials[static_cast<std::size_t>(id - 1)] =
                value / options.root_pi_scale;
        }
    }

    auto distance = problem.toDenseMatrix(options.exact_max_n);
    std::vector<int> new_to_old(static_cast<std::size_t>(dimension));
    std::vector<int> old_to_new(static_cast<std::size_t>(dimension));
    for (int vertex = 0; vertex < dimension; ++vertex) {
        new_to_old[static_cast<std::size_t>(vertex)] = vertex;
    }
    const bool relabel_root = !root_potentials.empty()
        && options.root_pi_relabel_root && external_root != 0;
    if (relabel_root) {
        std::swap(new_to_old[0],
                  new_to_old[static_cast<std::size_t>(external_root)]);
        for (int vertex = 0; vertex < dimension; ++vertex) {
            old_to_new[static_cast<std::size_t>(
                new_to_old[static_cast<std::size_t>(vertex)])] = vertex;
        }
        std::vector<std::vector<double>> relabeled(
            static_cast<std::size_t>(dimension),
            std::vector<double>(static_cast<std::size_t>(dimension)));
        for (int u = 0; u < dimension; ++u) {
            for (int v = 0; v < dimension; ++v) {
                relabeled[static_cast<std::size_t>(u)]
                         [static_cast<std::size_t>(v)] = distance[
                    static_cast<std::size_t>(new_to_old[static_cast<std::size_t>(u)])][
                    static_cast<std::size_t>(new_to_old[static_cast<std::size_t>(v)])];
            }
        }
        distance = std::move(relabeled);
        for (int& vertex : supplied_tour) {
            if (vertex >= 0 && vertex < dimension) {
                vertex = old_to_new[static_cast<std::size_t>(vertex)];
            }
        }
        std::vector<double> relabeled_potentials(
            static_cast<std::size_t>(dimension));
        for (int vertex = 0; vertex < dimension; ++vertex) {
            relabeled_potentials[static_cast<std::size_t>(vertex)] =
                root_potentials[static_cast<std::size_t>(
                    new_to_old[static_cast<std::size_t>(vertex)])];
        }
        root_potentials = std::move(relabeled_potentials);
    }

    tsp::BranchBoundSolver solver(std::move(distance));
    if (!supplied_tour.empty()) {
        solver.setInitialTour(supplied_tour);
    }
    const bool has_external_potentials = !root_potentials.empty();
    if (has_external_potentials) {
        const tsp::RootPotentialSeedStrategy seed_strategy =
            options.lkh_provider_pi_mode == LkhProviderPiMode::Replace
                ? tsp::RootPotentialSeedStrategy::Replace
                : options.root_pi_mode;
        solver.setRootPotentialSeed(
            std::move(root_potentials), seed_strategy);
    }
    const bool provider_supplied_potentials = has_external_potentials
        && lkh_provider != nullptr;
    const bool refine_external_potentials = has_external_potentials
        && (provider_supplied_potentials
                ? options.lkh_provider_pi_mode
                    == LkhProviderPiMode::WarmStart
                : options.root_pi_mode
                    == tsp::RootPotentialSeedStrategy::WarmStart);
    solver.setRootAscentStrategy(refine_external_potentials
            ? options.root_pi_refine_ascent
            : options.root_ascent);
    solver.setRootAscentIterationLimit(refine_external_potentials
            ? options.root_pi_refine_iterations
            : options.root_ascent_iterations);
    solver.setRootAscentDirectionSmoothing(
        refine_external_potentials
            ? options.root_pi_refine_smoothing_current_weight
            : options.root_ascent_smoothing_current_weight,
        refine_external_potentials
            ? options.root_pi_refine_dynamic_cosine_scale
            : options.root_ascent_dynamic_cosine_scale,
        refine_external_potentials
            ? options.root_pi_refine_dynamic_min_current_weight
            : options.root_ascent_dynamic_min_current_weight,
        refine_external_potentials
            ? options.root_pi_refine_dynamic_max_current_weight
            : options.root_ascent_dynamic_max_current_weight);
    solver.setNodeAscentStrategy(options.node_ascent);
    solver.setNodeAscentDirectionSmoothing(
        options.node_ascent_smoothing_current_weight,
        options.node_ascent_dynamic_cosine_scale,
        options.node_ascent_dynamic_min_current_weight,
        options.node_ascent_dynamic_max_current_weight);
    // 与 PHKMST 对齐：不跨兄弟节点复用临时势。
    solver.setNodeAscentSiblingWarmWeight(0.0);
    solver.setNodeAscentSiblingWarmStartStrategy(
        tsp::SiblingWarmStartStrategy::Off);
    // 与 PHKMST 对齐：普通路径固定一次历史 8-NN CLK。只有 LKH provider
    // 路径关闭内部 CLK，避免为同一初始化重复支付局部搜索成本。
    solver.setInitialClkStrategy(
        lkh_tour_supplied
            ? tsp::InitialClkStrategy::Off
            : tsp::InitialClkStrategy::Single,
        0.0, 0);
    solver.setLkCandidateSetOptions(
        tsp::LkCandidateSetStrategy::Nearest, 8);
    solver.setRootGuidedLk(false, false);
    // 分支顺序与势更新策略是两个正交开关，便于分别评估搜索树形状和下界质量。
    solver.setBranchEdgeOrder(options.branch_edge_order);
    solver.setPotentialUpdateOptions(
        options.potential_update,
        options.potential_update_depth,
        options.potential_update_iterations,
        options.potential_update_budget);
    solver.setPotentialUpdateMaxDepth(options.potential_update_max_depth);
    solver.setPotentialUpdateSkipLastEdges(
        options.potential_update_skip_last_edges);
    solver.setPotentialUpdateLargeGapTier(
        options.potential_update_large_gap_ratio,
        options.potential_update_large_gap_iterations);
    solver.setPotentialUpdateGapChangeThreshold(
        options.potential_update_min_gap_change_ratio);
    solver.setPotentialUpdateGapChangeStartDepth(
        options.potential_update_gap_change_start_depth);
    solver.setPotentialUpdateProbeOptions(
        options.potential_update_probe_updates,
        options.potential_update_probe_min_gap_ratio,
        options.potential_update_probe_min_coverage);
    solver.setRootCandidateCompaction(options.root_candidate_compaction);
    solver.setRootBoundOnly(options.root_bound_only);
    std::ofstream root_ascent_trace;
    if (!options.root_ascent_trace_path.empty()) {
        root_ascent_trace.open(options.root_ascent_trace_path);
        if (!root_ascent_trace) {
            throw std::runtime_error(
                "failed to open root ascent trace: "
                + options.root_ascent_trace_path);
        }
        root_ascent_trace
            << "strategy,iteration,phase,phase_iteration,"
               "lower_bound,best_lower_bound\n";
        solver.setRootAscentTraceOutput(root_ascent_trace);
    }
    if (options.debug) {
        solver.setDebugOutput(std::cerr, options.debug_interval);
    }
    output.result = solver.solve();
    if (relabel_root) {
        for (int& vertex : output.result.tour) {
            vertex = new_to_old[static_cast<std::size_t>(vertex)];
        }
    }
    if (root_ascent_trace.is_open()) {
        root_ascent_trace.flush();
        if (!root_ascent_trace) {
            throw std::runtime_error(
                "failed to write root ascent trace: "
                + options.root_ascent_trace_path);
        }
    }
    output.instance_wall_seconds = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - started_at).count();
    return output;
}

double finalUpperBound(const RunResult& run)
{
    return run.result.feasible
        ? run.result.cost : run.result.stats.initial_upper_bound;
}

double finalLowerBound(const RunResult& run)
{
    // exact 正常返回表示证明已经完成；root-bound 模式只拥有根 1-tree 证书。
    return run.method == "exact" && run.result.feasible
        ? run.result.cost : run.result.stats.root_lower_bound;
}

double finalRelativeGap(const RunResult& run)
{
    const double upper = finalUpperBound(run);
    const double lower = finalLowerBound(run);
    if (!std::isfinite(upper) || !std::isfinite(lower)) {
        return std::numeric_limits<double>::infinity();
    }
    return std::max(0.0, upper - lower)
        / std::max(1.0, std::fabs(upper));
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
    std::cout << "Final upper bound: " << finalUpperBound(run) << '\n';
    std::cout << "Final lower bound: " << finalLowerBound(run) << '\n';
    std::cout << "Final relative gap: " << finalRelativeGap(run) << '\n';
    std::cout << "Initial tour seconds: "
              << result.stats.initial_tour_seconds << '\n';
    std::cout << "Initial CLK starts: "
              << result.stats.initial_clk_starts << '\n';
    std::cout << "Adaptive CLK triggers: "
              << result.stats.adaptive_clk_triggers << '\n';
    std::cout << "Adaptive CLK improvements: "
              << result.stats.adaptive_clk_improvements << '\n';
    std::cout << "Root-guided LK calls: "
              << result.stats.root_guided_lk_calls << '\n';
    std::cout << "Root-guided LK improvements: "
              << result.stats.root_guided_lk_improvements << '\n';
    std::cout << "Root-guided LK reascents: "
              << result.stats.root_guided_lk_reascents << '\n';
    std::cout << "Root-guided LK seconds: "
              << result.stats.root_guided_lk_seconds << '\n';
    std::cout << "Root-guided LK total gain: "
              << result.stats.root_guided_lk_total_gain << '\n';
    std::cout << "Root fixing calls: " << result.stats.root_fixing_calls << '\n';
    std::cout << "Root fixing tested: " << result.stats.root_fixing_tested << '\n';
    std::cout << "Root fixing fixed zero: "
              << result.stats.root_fixing_fixed_zero << '\n';
    std::cout << "Root fixing tree tested: "
              << result.stats.root_fixing_tree_tested << '\n';
    std::cout << "Root fixing fixed one: "
              << result.stats.root_fixing_fixed_one << '\n';
    std::cout << "Root fixing active after: "
              << result.stats.root_fixing_active_after << '\n';
    std::cout << "Root fixing seconds: "
              << result.stats.root_fixing_seconds << '\n';
    std::cout << "Root candidate compaction calls: "
              << result.stats.root_candidate_compaction_calls << '\n';
    std::cout << "Root candidate edges before: "
              << result.stats.root_candidate_edges_before << '\n';
    std::cout << "Root candidate edges after: "
              << result.stats.root_candidate_edges_after << '\n';
    std::cout << "Root candidate compaction seconds: "
              << result.stats.root_candidate_compaction_seconds << '\n';
    std::cout << "Root potential iterations: "
              << result.stats.root_potential_iterations << '\n';
    std::cout << "Root ascent seconds: "
              << result.stats.root_ascent_seconds << '\n';
    std::cout << "Root external potential replacements: "
              << result.stats.root_external_potential_replacements << '\n';
    std::cout << "Root external potential warm starts: "
              << result.stats.root_external_potential_warm_starts << '\n';
    std::cout << "LKH provider calls: " << run.lkh_provider_calls << '\n';
    std::cout << "LKH provider failures: " << run.lkh_provider_failures << '\n';
    std::cout << "LKH provider seconds: " << run.lkh_provider_seconds << '\n';
    std::cout << "Instance wall seconds: " << run.instance_wall_seconds << '\n';
    std::cout << "Nodes created: " << result.stats.nodes_created << '\n';
    std::cout << "Nodes expanded: " << result.stats.nodes_expanded << '\n';
    std::cout << "Pruned by bound: " << result.stats.nodes_pruned_by_bound << '\n';
    std::cout << "Pruned infeasible: " << result.stats.nodes_pruned_infeasible << '\n';
    std::cout << "Search-node potential update candidates: "
              << result.stats.search_node_potential_update_candidates << '\n';
    std::cout << "Search-node potential updates triggered: "
              << result.stats.search_node_potential_updates_triggered << '\n';
    std::cout << "Potential updates skipped strategy none: "
              << result.stats.search_node_potential_updates_skipped_strategy_none << '\n';
    std::cout << "Potential updates skipped update depth zero: "
              << result.stats.search_node_potential_updates_skipped_update_depth_zero << '\n';
    std::cout << "Potential updates skipped budget exhausted: "
              << result.stats.search_node_potential_updates_skipped_budget_exhausted << '\n';
    std::cout << "Potential updates skipped numerically unsafe: "
              << result.stats.search_node_potential_updates_skipped_numerically_unsafe << '\n';
    std::cout << "Potential updates skipped invalid state: "
              << result.stats.search_node_potential_updates_skipped_invalid_state << '\n';
    std::cout << "Potential updates skipped zero violation: "
              << result.stats.search_node_potential_updates_skipped_zero_violation << '\n';
    std::cout << "Potential updates skipped zero iteration limit: "
              << result.stats.search_node_potential_updates_skipped_zero_iteration_limit << '\n';
    std::cout << "Potential updates skipped depth interval: "
              << result.stats.search_node_potential_updates_skipped_depth_interval << '\n';
    std::cout << "Potential updates skipped max depth: "
              << result.stats.search_node_potential_updates_skipped_max_depth << '\n';
    std::cout << "Potential updates skipped near leaf: "
              << result.stats.search_node_potential_updates_skipped_near_leaf << '\n';
    std::cout << "Potential updates skipped gap change below minimum: "
              << result.stats
                     .search_node_potential_updates_skipped_gap_change_below_minimum
              << '\n';
    std::cout << "Potential updates improved: "
              << result.stats.potential_updates_improved << '\n';
    std::cout << "Potential updates pruned: "
              << result.stats.potential_updates_pruned << '\n';
    std::cout << "Potential updates rebuilt: "
              << result.stats.potential_updates_rebuilt << '\n';
    std::cout << "Potential updates stopped prunable: "
              << result.stats.potential_updates_stopped_prunable << '\n';
    std::cout << "Potential updates large-gap tier: "
              << result.stats.potential_updates_large_gap_tier << '\n';
    std::cout << "Potential update gap-change shallow bypasses: "
              << result.stats.potential_update_gap_change_shallow_bypasses << '\n';
    std::cout << "Potential update probes started: "
              << result.stats.potential_update_probes_started << '\n';
    std::cout << "Potential update probes continued: "
              << result.stats.potential_update_probes_continued << '\n';
    std::cout << "Potential update probes rejected: "
              << result.stats.potential_update_probes_rejected << '\n';
    std::cout << "Search-node potential iterations: "
              << result.stats.search_node_potential_iterations << '\n';
    std::cout << "Potential update seconds: "
              << result.stats.potential_update_seconds << '\n';
    std::cout << "Potential update rebuild seconds: "
              << result.stats.potential_update_rebuild_seconds << '\n';
    std::cout << "Sibling warm probes: "
              << result.stats.sibling_warm_probes << '\n';
    std::cout << "Sibling warm accepted: "
              << result.stats.sibling_warm_accepted << '\n';
    std::cout << "Sibling warm rejected: "
              << result.stats.sibling_warm_rejected << '\n';
    std::cout << "Replacement seconds: "
              << result.stats.replacement_seconds << '\n';
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
int runSingleFile(const std::string& path,
                  const CliOptions& options,
                  tsp::LkhProvider* lkh_provider)
{
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Failed to open input file: " << path << '\n';
        return 2;
    }
    std::cout << "Solving instance from file: " << path << '\n';
    std::cout.flush();
    const RunResult run = solveInput(file, path, options, lkh_provider);
    printHumanResult(run);
    return run.result.feasible ? 0 : 1;
}

// 未传入文件时，从标准输入读取一个实例。
int runSingleStdin(const CliOptions& options)
{
    const RunResult run = solveInput(std::cin, {}, options, nullptr);
    printHumanResult(run);
    return run.result.feasible ? 0 : 1;
}

// 批处理模式输出 CSV 表头，便于重定向到结果文件后做统计分析。
void printBatchHeader()
{
    std::cout
        << "instance,status,method,dimension,cost,root_lower_bound,initial_upper_bound,"
        << "final_upper_bound,final_lower_bound,final_relative_gap,"
        << "initial_tour_seconds,initial_clk_starts,adaptive_clk_triggers,"
        << "adaptive_clk_improvements,"
        << "root_guided_lk_calls,root_guided_lk_improvements,"
        << "root_guided_lk_reascents,root_guided_lk_seconds,"
        << "root_guided_lk_total_gain,"
        << "root_fixing_calls,root_fixing_tested,root_fixing_fixed_zero,"
        << "root_fixing_tree_tested,root_fixing_fixed_one,root_fixing_active_after,"
        << "root_fixing_seconds,"
        << "root_candidate_compaction_calls,root_candidate_edges_before,"
        << "root_candidate_edges_after,root_candidate_compaction_seconds,"
        << "root_potential_iterations,root_ascent_seconds,"
        << "root_external_potential_replacements,"
        << "root_external_potential_warm_starts,"
        << "lkh_provider_calls,lkh_provider_failures,lkh_provider_seconds,"
        << "instance_wall_seconds,"
        << "nodes_created,nodes_expanded,pruned_by_bound,pruned_infeasible,"
        << "search_node_potential_update_candidates,"
        << "search_node_potential_updates_triggered,"
        << "search_node_potential_updates_skipped_strategy_none,"
        << "search_node_potential_updates_skipped_update_depth_zero,"
        << "search_node_potential_updates_skipped_budget_exhausted,"
        << "search_node_potential_updates_skipped_numerically_unsafe,"
        << "search_node_potential_updates_skipped_invalid_state,"
        << "search_node_potential_updates_skipped_zero_violation,"
        << "search_node_potential_updates_skipped_zero_iteration_limit,"
        << "search_node_potential_updates_skipped_max_depth,"
        << "search_node_potential_updates_skipped_near_leaf,"
        << "search_node_potential_updates_skipped_depth_interval,"
        << "search_node_potential_updates_skipped_gap_change_below_minimum,"
        << "potential_updates_improved,"
        << "potential_updates_pruned,potential_updates_rebuilt,"
        << "potential_updates_stopped_prunable,"
        << "potential_updates_large_gap_tier,"
        << "potential_update_gap_change_shallow_bypasses,"
        << "search_node_potential_iterations,potential_update_seconds,"
        << "potential_update_rebuild_seconds,potential_update_total_gain,"
        << "potential_update_max_gain,potential_update_probes_started,"
        << "potential_update_probes_continued,potential_update_probes_rejected,"
        << "sibling_warm_probes,sibling_warm_accepted,sibling_warm_rejected,"
        << "replacement_seconds,"
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
        // method 到 tour 共 72 个空字段；最后一个字段保留错误消息。
        // 新增批量列时必须同步此数量，确保错误行也与 CSV 表头严格对齐。
        for (int field = 0; field < 72; ++field) {
            std::cout << ',';
        }
        std::cout << csvQuote(message) << '\n';
        return;
    }

    const tsp::SolveResult& result = run->result;
    // 有求解结果时，把成本、搜索统计和回路统一写成一行 CSV。
    std::cout << run->method << ','
              << run->dimension << ','
              << formatDouble(result.cost) << ','
              << formatDouble(result.stats.root_lower_bound) << ','
              << formatDouble(result.stats.initial_upper_bound) << ','
              << formatDouble(finalUpperBound(*run)) << ','
              << formatDouble(finalLowerBound(*run)) << ','
              << formatDouble(finalRelativeGap(*run)) << ','
              << formatDouble(result.stats.initial_tour_seconds) << ','
              << result.stats.initial_clk_starts << ','
              << result.stats.adaptive_clk_triggers << ','
              << result.stats.adaptive_clk_improvements << ','
              << result.stats.root_guided_lk_calls << ','
              << result.stats.root_guided_lk_improvements << ','
              << result.stats.root_guided_lk_reascents << ','
              << formatDouble(result.stats.root_guided_lk_seconds) << ','
              << formatDouble(result.stats.root_guided_lk_total_gain) << ','
              << result.stats.root_fixing_calls << ','
              << result.stats.root_fixing_tested << ','
              << result.stats.root_fixing_fixed_zero << ','
              << result.stats.root_fixing_tree_tested << ','
              << result.stats.root_fixing_fixed_one << ','
              << result.stats.root_fixing_active_after << ','
              << formatDouble(result.stats.root_fixing_seconds) << ','
              << result.stats.root_candidate_compaction_calls << ','
              << result.stats.root_candidate_edges_before << ','
              << result.stats.root_candidate_edges_after << ','
              << formatDouble(
                     result.stats.root_candidate_compaction_seconds) << ','
              << result.stats.root_potential_iterations << ','
              << formatDouble(result.stats.root_ascent_seconds) << ','
              << result.stats.root_external_potential_replacements << ','
              << result.stats.root_external_potential_warm_starts << ','
              << run->lkh_provider_calls << ','
              << run->lkh_provider_failures << ','
              << formatDouble(run->lkh_provider_seconds) << ','
              << formatDouble(run->instance_wall_seconds) << ','
              << result.stats.nodes_created << ','
              << result.stats.nodes_expanded << ','
              << result.stats.nodes_pruned_by_bound << ','
              << result.stats.nodes_pruned_infeasible << ','
              << result.stats.search_node_potential_update_candidates << ','
              << result.stats.search_node_potential_updates_triggered << ','
              << result.stats.search_node_potential_updates_skipped_strategy_none << ','
              << result.stats.search_node_potential_updates_skipped_update_depth_zero << ','
              << result.stats.search_node_potential_updates_skipped_budget_exhausted << ','
              << result.stats.search_node_potential_updates_skipped_numerically_unsafe << ','
              << result.stats.search_node_potential_updates_skipped_invalid_state << ','
              << result.stats.search_node_potential_updates_skipped_zero_violation << ','
              << result.stats.search_node_potential_updates_skipped_zero_iteration_limit << ','
              << result.stats.search_node_potential_updates_skipped_max_depth << ','
              << result.stats.search_node_potential_updates_skipped_near_leaf << ','
              << result.stats.search_node_potential_updates_skipped_depth_interval << ','
              << result.stats
                     .search_node_potential_updates_skipped_gap_change_below_minimum
              << ','
              << result.stats.potential_updates_improved << ','
              << result.stats.potential_updates_pruned << ','
              << result.stats.potential_updates_rebuilt << ','
              << result.stats.potential_updates_stopped_prunable << ','
              << result.stats.potential_updates_large_gap_tier << ','
              << result.stats.potential_update_gap_change_shallow_bypasses << ','
              << result.stats.search_node_potential_iterations << ','
              << formatDouble(result.stats.potential_update_seconds) << ','
              << formatDouble(result.stats.potential_update_rebuild_seconds) << ','
              << formatDouble(result.stats.potential_update_total_gain) << ','
              << formatDouble(result.stats.potential_update_max_gain) << ','
              << result.stats.potential_update_probes_started << ','
              << result.stats.potential_update_probes_continued << ','
              << result.stats.potential_update_probes_rejected << ','
              << result.stats.sibling_warm_probes << ','
              << result.stats.sibling_warm_accepted << ','
              << result.stats.sibling_warm_rejected << ','
              << formatDouble(result.stats.replacement_seconds) << ','
              << csvQuote(formatTourLimited(result.tour)) << ','
              << csvQuote(message) << '\n';
}

// 批处理清单每行一个实例路径；单个实例失败不会中断整个批次。
int runBatch(const std::string& list_path,
             const CliOptions& options,
             tsp::LkhProvider* lkh_provider)
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

            RunResult run = solveInput(input, path, options, lkh_provider);
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
              << "  --initial-tour <file> (n then n zero-based vertices; single instance)\n"
              << "  --root-pi <LKH-PI-file> (single instance)\n"
              << "  --root-pi-scale <x > 0> (default 100)\n"
              << "  --root-pi-mode <replace|warm-start> (default warm-start)\n"
              << "  --root-pi-refine-ascent <none|polyak|helsgaun|hybrid|"
                 "hybrid-reverse|polyak-smoothed|polyak-smoothed-dynamic>"
                 " (default polyak)\n"
              << "  --root-pi-refine-iterations <n> (default 64)\n"
              << "  --root-pi-refine-smoothing-current-weight <x in [0,1]>"
                 " (default 0.7)\n"
              << "  --root-pi-refine-dynamic-cosine-scale <x >= 0>"
                 " (default 0.2)\n"
              << "  --root-pi-refine-dynamic-min-current-weight <x in [0,1]>"
                 " (default 0.5)\n"
              << "  --root-pi-refine-dynamic-max-current-weight <x in [0,1]>"
                 " (default 0.9)\n"
              << "  --root-pi-relabel-root <off|on> (default on)\n"
              << "  --lkh-provider <path> (use auto for CMake-built provider)\n"
              << "  --lkh-runs <n> (default 1)\n"
              << "  --lkh-max-trials <n> (default 0 = dimension)\n"
              << "  --lkh-seed <n> (default 1)\n"
              << "  --lkh-time-limit <seconds> (default 0 = unlimited)\n"
              << "  --lkh-pi-mode <off|replace|warm-start>"
                 " (default warm-start)\n"
              << "  --lkh-provider-failure <error|fallback>"
                 " (default error)\n"
              << "  --hk-ascent <none|polyak|helsgaun|hybrid|hybrid-reverse|"
                 "polyak-smoothed|polyak-smoothed-dynamic>\n"
              << "  --root-ascent-iterations <n>\n"
              << "  --root-ascent-smoothing-current-weight <x in [0,1]>\n"
              << "  --root-ascent-dynamic-cosine-scale <x >= 0>\n"
              << "  --root-ascent-dynamic-min-current-weight <x in [0,1]>\n"
              << "  --root-ascent-dynamic-max-current-weight <x in [0,1]>\n"
              << "  --root-ascent-trace <csv-path> (root-bound-only, single instance)\n"
              << "  --hk-node-ascent <polyak|helsgaun|polyak-smoothed|"
                 "polyak-smoothed-dynamic>\n"
              << "  --hk-node-smoothing-current-weight <x in [0,1]>\n"
              << "  --hk-node-dynamic-cosine-scale <x >= 0>\n"
              << "  --hk-node-dynamic-min-current-weight <x in [0,1]>\n"
              << "  --hk-node-dynamic-max-current-weight <x in [0,1]>\n"
              << "  --branch-edge-order <weight|root-alpha-asc|root-alpha-desc|"
                 "root-alpha-global-asc|root-alpha-global-desc|"
                 "forbid-delta-asc|forbid-delta-desc|forbid-degree-desc|"
                 "root-frequency-middle|strong-top2|weight-desc|"
                 "max-degree-all-weight|excess-cover-weight|"
                 "local-excess-cover-weight|max-degree-excess-weight|"
                 "propagation-weight|forced-degree-weight|"
                 "max-degree-min-undecided|max-degree-max-undecided>\n"
              << "  --hk-potential-update <none|subtree-depth|subtree-adaptive>\n"
              << "  --hk-update-depth <n>\n"
              << "  --hk-update-max-depth <n> (0 = unlimited)\n"
              << "  --hk-update-skip-last-edges <n> (0 = disabled)\n"
              << "  --hk-update-iterations <n>\n"
              << "  --hk-update-min-gap-change-ratio <x> (default 0.0001)\n"
              << "  --hk-update-gap-change-start-depth <n> (default 2)\n"
              << "  --hk-update-large-gap-ratio <x>\n"
              << "  --hk-update-large-gap-iterations <n>\n"
              << "  --hk-update-budget <n> (0 = unlimited)\n"
              << "  --hk-update-probe-updates <n>\n"
              << "  --hk-update-probe-min-gap-ratio <x>\n"
              << "  --hk-update-probe-min-coverage <x in [0,1]>\n"
              << "  --root-candidate-compaction <off|on> (default on)\n"
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
    if (value == "hybrid-reverse") {
        return tsp::RootAscentStrategy::HybridReverse;
    }
    if (value == "polyak-smoothed") {
        return tsp::RootAscentStrategy::PolyakSmoothed;
    }
    if (value == "polyak-smoothed-dynamic") {
        return tsp::RootAscentStrategy::PolyakSmoothedDynamic;
    }
    throw std::runtime_error(
        "invalid value for --hk-ascent: " + value
        + " (expected none, polyak, helsgaun, hybrid, hybrid-reverse, "
          "polyak-smoothed, or polyak-smoothed-dynamic)");
}

tsp::NodeAscentStrategy parseNodeAscentStrategy(const std::string& value)
{
    if (value == "polyak") return tsp::NodeAscentStrategy::Polyak;
    if (value == "helsgaun") return tsp::NodeAscentStrategy::Helsgaun;
    if (value == "polyak-smoothed") {
        return tsp::NodeAscentStrategy::PolyakSmoothed;
    }
    if (value == "polyak-smoothed-dynamic") {
        return tsp::NodeAscentStrategy::PolyakSmoothedDynamic;
    }
    throw std::runtime_error(
        "invalid value for --hk-node-ascent: " + value
        + " (expected polyak, helsgaun, polyak-smoothed, "
          "or polyak-smoothed-dynamic)");
}

bool parseOnOff(const std::string& value, const std::string& option)
{
    if (value == "on") return true;
    if (value == "off") return false;
    throw std::runtime_error(
        "invalid value for " + option + ": " + value
        + " (expected off or on)");
}

tsp::RootPotentialSeedStrategy parseRootPotentialSeedStrategy(
    const std::string& value)
{
    if (value == "replace") {
        return tsp::RootPotentialSeedStrategy::Replace;
    }
    if (value == "warm-start") {
        return tsp::RootPotentialSeedStrategy::WarmStart;
    }
    throw std::runtime_error(
        "invalid value for --root-pi-mode: " + value
        + " (expected replace or warm-start)");
}

LkhProviderPiMode parseLkhProviderPiMode(const std::string& value)
{
    if (value == "off") return LkhProviderPiMode::Off;
    if (value == "replace") return LkhProviderPiMode::Replace;
    if (value == "warm-start") return LkhProviderPiMode::WarmStart;
    throw std::runtime_error(
        "invalid value for --lkh-pi-mode: " + value
        + " (expected off, replace, or warm-start)");
}

LkhProviderFailureMode parseLkhProviderFailureMode(
    const std::string& value)
{
    if (value == "error") return LkhProviderFailureMode::Error;
    if (value == "fallback") return LkhProviderFailureMode::Fallback;
    throw std::runtime_error(
        "invalid value for --lkh-provider-failure: " + value
        + " (expected error or fallback)");
}

tsp::PotentialUpdateStrategy parsePotentialUpdateStrategy(
    const std::string& value)
{
    if (value == "none") return tsp::PotentialUpdateStrategy::None;
    if (value == "subtree-depth") {
        return tsp::PotentialUpdateStrategy::SubtreeDepth;
    }
    if (value == "subtree-adaptive") {
        return tsp::PotentialUpdateStrategy::SubtreeAdaptive;
    }
    throw std::runtime_error(
        "invalid value for --hk-potential-update: " + value
        + " (expected none, subtree-depth, or subtree-adaptive)");
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
    if (value == "root-alpha-global-asc") {
        return tsp::BranchEdgeOrder::RootAlphaGlobalAscending;
    }
    if (value == "root-alpha-global-desc") {
        return tsp::BranchEdgeOrder::RootAlphaGlobalDescending;
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
          "root-alpha-global-asc, root-alpha-global-desc, "
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
    bool large_gap_ratio_seen = false;
    bool large_gap_iterations_seen = false;
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
        } else if (arg == "--initial-tour") {
            options.initial_tour_path = require_value(arg);
        } else if (arg == "--root-pi") {
            options.root_pi_path = require_value(arg);
        } else if (arg == "--root-pi-scale") {
            options.root_pi_scale = parseDoubleOption(require_value(arg), arg);
            if (options.root_pi_scale <= 0.0) {
                throw std::runtime_error(
                    "--root-pi-scale must be greater than zero");
            }
        } else if (arg == "--root-pi-mode") {
            options.root_pi_mode =
                parseRootPotentialSeedStrategy(require_value(arg));
        } else if (arg == "--root-pi-refine-ascent") {
            options.root_pi_refine_ascent =
                parseRootAscentStrategy(require_value(arg));
        } else if (arg == "--root-pi-refine-iterations") {
            options.root_pi_refine_iterations =
                parseSizeOption(require_value(arg), arg);
            if (options.root_pi_refine_iterations == 0) {
                throw std::runtime_error(
                    "--root-pi-refine-iterations must be greater than zero");
            }
        } else if (arg == "--root-pi-refine-smoothing-current-weight") {
            options.root_pi_refine_smoothing_current_weight =
                parseDoubleOption(require_value(arg), arg);
        } else if (arg == "--root-pi-refine-dynamic-cosine-scale") {
            options.root_pi_refine_dynamic_cosine_scale =
                parseDoubleOption(require_value(arg), arg);
            if (options.root_pi_refine_dynamic_cosine_scale < 0.0) {
                throw std::runtime_error(
                    "--root-pi-refine-dynamic-cosine-scale must be non-negative");
            }
        } else if (arg == "--root-pi-refine-dynamic-min-current-weight") {
            options.root_pi_refine_dynamic_min_current_weight =
                parseDoubleOption(require_value(arg), arg);
        } else if (arg == "--root-pi-refine-dynamic-max-current-weight") {
            options.root_pi_refine_dynamic_max_current_weight =
                parseDoubleOption(require_value(arg), arg);
        } else if (arg == "--root-pi-relabel-root") {
            options.root_pi_relabel_root =
                parseOnOff(require_value(arg), arg);
        } else if (arg == "--lkh-provider") {
            options.lkh_provider_path = require_value(arg);
        } else if (arg == "--lkh-runs") {
            options.lkh_provider_runs = parseSizeOption(require_value(arg), arg);
            if (options.lkh_provider_runs == 0) {
                throw std::runtime_error("--lkh-runs must be greater than zero");
            }
        } else if (arg == "--lkh-max-trials") {
            options.lkh_provider_max_trials =
                parseSizeOption(require_value(arg), arg);
        } else if (arg == "--lkh-seed") {
            options.lkh_provider_seed =
                parseSizeOption(require_value(arg), arg);
        } else if (arg == "--lkh-time-limit") {
            options.lkh_provider_time_limit_seconds =
                parseDoubleOption(require_value(arg), arg);
            if (options.lkh_provider_time_limit_seconds < 0.0) {
                throw std::runtime_error(
                    "--lkh-time-limit must be non-negative");
            }
        } else if (arg == "--lkh-pi-mode") {
            options.lkh_provider_pi_mode =
                parseLkhProviderPiMode(require_value(arg));
        } else if (arg == "--lkh-provider-failure") {
            options.lkh_provider_failure_mode =
                parseLkhProviderFailureMode(require_value(arg));
        } else if (arg == "--exact-max-n") {
            options.exact_max_n = parseSizeOption(require_value(arg), arg);
            if (options.exact_max_n == 0) {
                throw std::runtime_error("--exact-max-n must be greater than zero");
            }
        } else if (arg == "--hk-ascent") {
            options.root_ascent = parseRootAscentStrategy(require_value(arg));
        } else if (arg == "--root-ascent-iterations") {
            options.root_ascent_iterations = parseSizeOption(require_value(arg), arg);
            if (options.root_ascent_iterations == 0) {
                throw std::runtime_error(
                    "--root-ascent-iterations must be greater than zero");
            }
        } else if (arg == "--root-ascent-smoothing-current-weight") {
            options.root_ascent_smoothing_current_weight =
                parseDoubleOption(require_value(arg), arg);
        } else if (arg == "--root-ascent-dynamic-cosine-scale") {
            options.root_ascent_dynamic_cosine_scale =
                parseDoubleOption(require_value(arg), arg);
            if (options.root_ascent_dynamic_cosine_scale < 0.0) {
                throw std::runtime_error(
                    "--root-ascent-dynamic-cosine-scale must be non-negative");
            }
        } else if (arg == "--root-ascent-dynamic-min-current-weight") {
            options.root_ascent_dynamic_min_current_weight =
                parseDoubleOption(require_value(arg), arg);
        } else if (arg == "--root-ascent-dynamic-max-current-weight") {
            options.root_ascent_dynamic_max_current_weight =
                parseDoubleOption(require_value(arg), arg);
        } else if (arg == "--root-ascent-trace") {
            options.root_ascent_trace_path = require_value(arg);
        } else if (arg == "--hk-node-ascent") {
            options.node_ascent = parseNodeAscentStrategy(require_value(arg));
        } else if (arg == "--hk-node-smoothing-current-weight") {
            options.node_ascent_smoothing_current_weight =
                parseDoubleOption(require_value(arg), arg);
        } else if (arg == "--hk-node-dynamic-cosine-scale") {
            options.node_ascent_dynamic_cosine_scale =
                parseDoubleOption(require_value(arg), arg);
            if (options.node_ascent_dynamic_cosine_scale < 0.0) {
                throw std::runtime_error(
                    "--hk-node-dynamic-cosine-scale must be non-negative");
            }
        } else if (arg == "--hk-node-dynamic-min-current-weight") {
            options.node_ascent_dynamic_min_current_weight =
                parseDoubleOption(require_value(arg), arg);
        } else if (arg == "--hk-node-dynamic-max-current-weight") {
            options.node_ascent_dynamic_max_current_weight =
                parseDoubleOption(require_value(arg), arg);
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
        } else if (arg == "--hk-update-max-depth") {
            options.potential_update_max_depth =
                parseSizeOption(require_value(arg), arg);
        } else if (arg == "--hk-update-skip-last-edges") {
            options.potential_update_skip_last_edges =
                parseSizeOption(require_value(arg), arg);
        } else if (arg == "--hk-update-iterations") {
            options.potential_update_iterations = parseSizeOption(require_value(arg), arg);
        } else if (arg == "--hk-update-min-gap-change-ratio") {
            options.potential_update_min_gap_change_ratio =
                parseDoubleOption(require_value(arg), arg);
            if (options.potential_update_min_gap_change_ratio < 0.0) {
                throw std::runtime_error(
                    "--hk-update-min-gap-change-ratio must be non-negative");
            }
        } else if (arg == "--hk-update-gap-change-start-depth") {
            options.potential_update_gap_change_start_depth =
                parseSizeOption(require_value(arg), arg);
        } else if (arg == "--hk-update-large-gap-ratio") {
            options.potential_update_large_gap_ratio =
                parseDoubleOption(require_value(arg), arg);
            if (options.potential_update_large_gap_ratio < 0.0) {
                throw std::runtime_error(
                    "--hk-update-large-gap-ratio must be non-negative");
            }
            large_gap_ratio_seen = true;
        } else if (arg == "--hk-update-large-gap-iterations") {
            options.potential_update_large_gap_iterations =
                parseSizeOption(require_value(arg), arg);
            if (options.potential_update_large_gap_iterations == 0) {
                throw std::runtime_error(
                    "--hk-update-large-gap-iterations must be greater than zero");
            }
            large_gap_iterations_seen = true;
        } else if (arg == "--hk-update-budget") {
            options.potential_update_budget = parseSizeOption(require_value(arg), arg);
        } else if (arg == "--hk-update-probe-updates") {
            options.potential_update_probe_updates =
                parseSizeOption(require_value(arg), arg);
        } else if (arg == "--hk-update-probe-min-gap-ratio") {
            options.potential_update_probe_min_gap_ratio =
                parseDoubleOption(require_value(arg), arg);
            if (options.potential_update_probe_min_gap_ratio < 0.0) {
                throw std::runtime_error(
                    "--hk-update-probe-min-gap-ratio must be non-negative");
            }
        } else if (arg == "--hk-update-probe-min-coverage") {
            options.potential_update_probe_min_coverage =
                parseDoubleOption(require_value(arg), arg);
            if (options.potential_update_probe_min_coverage < 0.0
                || options.potential_update_probe_min_coverage > 1.0) {
                throw std::runtime_error(
                    "--hk-update-probe-min-coverage must be in [0, 1]");
            }
        } else if (arg == "--root-candidate-compaction") {
            options.root_candidate_compaction =
                parseOnOff(require_value(arg), arg);
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
    if (large_gap_ratio_seen != large_gap_iterations_seen) {
        throw std::runtime_error(
            "--hk-update-large-gap-ratio and "
            "--hk-update-large-gap-iterations must be used together");
    }
    if (!options.initial_tour_path.empty() && !options.batch_path.empty()) {
        throw std::runtime_error("--initial-tour only supports a single instance");
    }
    if (!options.root_pi_path.empty() && !options.batch_path.empty()) {
        throw std::runtime_error("--root-pi only supports a single instance");
    }
    if (!options.lkh_provider_path.empty()
        && (!options.initial_tour_path.empty()
            || !options.root_pi_path.empty())) {
        throw std::runtime_error(
            "--lkh-provider cannot be combined with --initial-tour or --root-pi");
    }
    if (!options.lkh_provider_path.empty()
        && options.input_path.empty() && options.batch_path.empty()) {
        throw std::runtime_error(
            "--lkh-provider requires a file input or --batch");
    }
    if (options.root_pi_refine_dynamic_min_current_weight < 0.0
        || options.root_pi_refine_dynamic_min_current_weight
            > options.root_pi_refine_smoothing_current_weight
        || options.root_pi_refine_smoothing_current_weight
            > options.root_pi_refine_dynamic_max_current_weight
        || options.root_pi_refine_dynamic_max_current_weight > 1.0) {
        throw std::runtime_error(
            "root PI refinement direction weights must satisfy "
            "0 <= dynamic minimum <= fixed <= dynamic maximum <= 1");
    }
    if (options.root_ascent_dynamic_min_current_weight < 0.0
        || options.root_ascent_dynamic_min_current_weight
            > options.root_ascent_smoothing_current_weight
        || options.root_ascent_smoothing_current_weight
            > options.root_ascent_dynamic_max_current_weight
        || options.root_ascent_dynamic_max_current_weight > 1.0) {
        throw std::runtime_error(
            "root ascent direction weights must satisfy "
            "0 <= dynamic minimum <= fixed <= dynamic maximum <= 1");
    }
    if (options.node_ascent_dynamic_min_current_weight < 0.0
        || options.node_ascent_dynamic_min_current_weight
            > options.node_ascent_smoothing_current_weight
        || options.node_ascent_smoothing_current_weight
            > options.node_ascent_dynamic_max_current_weight
        || options.node_ascent_dynamic_max_current_weight > 1.0) {
        throw std::runtime_error(
            "node ascent direction weights must satisfy "
            "0 <= dynamic minimum <= fixed <= dynamic maximum <= 1");
    }
    if (!options.root_ascent_trace_path.empty() && !options.root_bound_only) {
        throw std::runtime_error(
            "--root-ascent-trace requires --root-bound-only");
    }
    if (!options.root_ascent_trace_path.empty() && !options.batch_path.empty()) {
        throw std::runtime_error(
            "--root-ascent-trace only supports a single instance");
    }
    return options;
}

std::string resolveLkhProviderPath(const std::string& requested)
{
    if (requested != "auto") return requested;
#ifdef TSP_DEFAULT_LKH_PROVIDER_PATH
    return TSP_DEFAULT_LKH_PROVIDER_PATH;
#else
    throw std::runtime_error(
        "--lkh-provider auto is unavailable: configure with "
        "-DTSP_LKH_SOURCE_DIR=/path/to/LKH-2.x or pass an executable path");
#endif
}

} // namespace

int main(int argc, char** argv)
{
    try {
        const CliOptions options = parseArgs(argc, argv);
        std::unique_ptr<tsp::LkhProvider> lkh_provider;
        if (!options.lkh_provider_path.empty()) {
            lkh_provider = std::make_unique<tsp::LkhProvider>(
                resolveLkhProviderPath(options.lkh_provider_path));
        }
        if (!options.batch_path.empty()) {
            if (!options.input_path.empty()) {
                throw std::runtime_error("batch mode does not accept a separate input file");
            }
            return runBatch(options.batch_path, options, lkh_provider.get());
        }
        if (!options.input_path.empty()) {
            return runSingleFile(
                options.input_path, options, lkh_provider.get());
        }
        return runSingleStdin(options);
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 2;
    }

    return 0;
}
