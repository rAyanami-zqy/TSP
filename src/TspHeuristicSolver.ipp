// 不进入精确 BP 搜索的通用 tour 构造与 2-opt 启发式。
// 与精确求解器内部的初始 tour/LK 启发式分开，便于独立阅读两条路径。
// 计算给定 TSP 回路的总成本，若回路无效（长度不为 n 或存在缺边）则返回 infinity。
namespace {

double problemTourCost(const TspProblem& problem, const std::vector<int>& tour)
{
    const int n = problem.dimension();
    if (static_cast<int>(tour.size()) != n) {
        return std::numeric_limits<double>::infinity();
    }

    double cost = 0.0;
    for (int i = 0; i < n; ++i) {
        const double edge = problem.distance(tour[static_cast<std::size_t>(i)],
                                             tour[static_cast<std::size_t>((i + 1) % n)]);
        if (!isFinite(edge)) {
            return std::numeric_limits<double>::infinity();
        }
        cost += edge;
    }
    return cost;
}

std::vector<int> sequentialTour(int n)
{
    std::vector<int> tour(static_cast<std::size_t>(n));
    std::iota(tour.begin(), tour.end(), 0);
    return tour;
}

// 从指定起点出发，按照最近邻策略构造一个 TSP 回路。起点不同可能得到不同的回路，返回所有起点中成本最低的那个回路。
std::vector<int> nearestNeighborTour(const TspProblem& problem, int start)
{
    const int n = problem.dimension();
    std::vector<int> tour;
    tour.reserve(static_cast<std::size_t>(n));
    std::vector<unsigned char> used(static_cast<std::size_t>(n), 0);

    int current = start;
    tour.push_back(current);
    used[static_cast<std::size_t>(current)] = 1;

    for (int step = 1; step < n; ++step) {
        int next = -1;
        double best = std::numeric_limits<double>::infinity();
        for (int v = 0; v < n; ++v) {
            if (used[static_cast<std::size_t>(v)]) {
                continue;
            }
            const double value = problem.distance(current, v);
            if (value < best) {
                best = value;
                next = v;
            }
        }
        if (next < 0 || !isFinite(best)) {
            return {};
        }
        current = next;
        used[static_cast<std::size_t>(current)] = 1;
        tour.push_back(current);
    }

    if (!isFinite(problem.distance(current, start))) {
        return {};
    }
    return tour;
}

std::uint64_t partBy1(std::uint32_t value)
{
    std::uint64_t result = value;
    result = (result | (result << 16U)) & 0x0000FFFF0000FFFFULL;
    result = (result | (result << 8U)) & 0x00FF00FF00FF00FFULL;
    result = (result | (result << 4U)) & 0x0F0F0F0F0F0F0F0FULL;
    result = (result | (result << 2U)) & 0x3333333333333333ULL;
    result = (result | (result << 1U)) & 0x5555555555555555ULL;
    return result;
}

std::vector<int> mortonTour(const TspProblem& problem)
{
    const int n = problem.dimension();
    if (!problem.hasCoordinates()) {
        return sequentialTour(n);
    }

    double min_x = problem.coordinates.front().x;
    double max_x = problem.coordinates.front().x;
    double min_y = problem.coordinates.front().y;
    double max_y = problem.coordinates.front().y;
    for (const Point& point : problem.coordinates) {
        min_x = std::min(min_x, point.x);
        max_x = std::max(max_x, point.x);
        min_y = std::min(min_y, point.y);
        max_y = std::max(max_y, point.y);
    }
    const double range_x = std::max(kHeuristicEps, max_x - min_x);
    const double range_y = std::max(kHeuristicEps, max_y - min_y);

    std::vector<std::pair<std::uint64_t, int>> keyed;
    keyed.reserve(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        const Point& point = problem.coordinates[static_cast<std::size_t>(i)];
        const auto sx = static_cast<std::uint32_t>(
            std::max(0.0, std::min(65535.0, 65535.0 * (point.x - min_x) / range_x)));
        const auto sy = static_cast<std::uint32_t>(
            std::max(0.0, std::min(65535.0, 65535.0 * (point.y - min_y) / range_y)));
        const std::uint64_t key = partBy1(sx) | (partBy1(sy) << 1U);
        keyed.push_back({key, i});
    }
    std::sort(keyed.begin(), keyed.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.first != rhs.first) {
            return lhs.first < rhs.first;
        }
        return lhs.second < rhs.second;
    });

    std::vector<int> tour;
    tour.reserve(static_cast<std::size_t>(n));
    for (const auto& item : keyed) {
        tour.push_back(item.second);
    }
    return tour;
}

std::vector<int> initialHeuristicTour(const TspProblem& problem, const HeuristicOptions& options)
{
    const int n = problem.dimension();
    if (static_cast<std::size_t>(n) > options.nearest_scan_limit) {
        writeDebugLine(options.debug,
                       "initial tour: morton order because dimension exceeds nearest-scan limit");
        return mortonTour(problem);
    }

    std::vector<int> starts;
    starts.push_back(0);
    if (n > 4) {
        starts.push_back(n / 2);
        starts.push_back(n / 4);
        starts.push_back((3 * n) / 4);
    }

    std::mt19937 rng(options.seed);
    std::uniform_int_distribution<int> distribution(0, n - 1);
    while (starts.size() < options.starts) {
        starts.push_back(distribution(rng));
    }
    std::sort(starts.begin(), starts.end());
    starts.erase(std::unique(starts.begin(), starts.end()), starts.end());
    writeDebugLine(options.debug,
                   "initial tour: nearest-neighbor starts=" + std::to_string(starts.size()));

    double best_cost = std::numeric_limits<double>::infinity();
    std::vector<int> best_tour;
    const std::size_t debug_interval = normalizedDebugInterval(options.debug);
    std::size_t starts_done = 0;
    for (int start : starts) {
        std::vector<int> candidate = nearestNeighborTour(problem, start);
        ++starts_done;
        if (candidate.empty()) {
            if (options.debug.output != nullptr && starts_done % debug_interval == 0) {
                writeDebugLine(options.debug,
                               "initial tour progress: tried=" + std::to_string(starts_done)
                                   + " best=" + formatDebugDouble(best_cost));
            }
            continue;
        }
        const double cost = problemTourCost(problem, candidate);
        if (cost < best_cost) {
            best_cost = cost;
            best_tour = std::move(candidate);
            writeDebugLine(options.debug,
                           "initial tour improved: start=" + std::to_string(start)
                               + " cost=" + formatDebugDouble(best_cost));
        } else if (options.debug.output != nullptr && starts_done % debug_interval == 0) {
            writeDebugLine(options.debug,
                           "initial tour progress: tried=" + std::to_string(starts_done)
                               + " best=" + formatDebugDouble(best_cost));
        }
    }
    if (!best_tour.empty()) {
        return best_tour;
    }
    writeDebugLine(options.debug, "initial tour: nearest-neighbor failed; using sequential order");
    return sequentialTour(n);
}

void fullTwoOpt(const TspProblem& problem,
                std::vector<int>& tour,
                double& cost,
                std::size_t passes,
                const DebugOptions& debug)
{
    const int n = problem.dimension();
    for (std::size_t pass = 0; pass < passes; ++pass) {
        const double before = cost;
        bool improved = false;
        for (int i = 1; i < n - 1 && !improved; ++i) {
            for (int k = i + 1; k < n && !improved; ++k) {
                const int a = tour[static_cast<std::size_t>(i - 1)];
                const int b = tour[static_cast<std::size_t>(i)];
                const int c = tour[static_cast<std::size_t>(k)];
                const int d = tour[static_cast<std::size_t>((k + 1) % n)];
                const double ac = problem.distance(a, c);
                const double bd = problem.distance(b, d);
                if (!isFinite(ac) || !isFinite(bd)) {
                    continue;
                }
                const double delta = ac + bd - problem.distance(a, b) - problem.distance(c, d);
                if (delta < -kHeuristicEps) {
                    std::reverse(tour.begin() + i, tour.begin() + k + 1);
                    cost += delta;
                    improved = true;
                }
            }
        }
        std::ostringstream line;
        line << "2-opt full pass=" << (pass + 1)
             << " improved=" << (improved ? "yes" : "no")
             << " cost=" << formatDebugDouble(cost)
             << " delta=" << formatDebugDouble(cost - before);
        writeDebugLine(debug, line.str());
        if (!improved) {
            break;
        }
    }
    cost = problemTourCost(problem, tour);
}

void windowTwoOpt(const TspProblem& problem,
                  std::vector<int>& tour,
                  double& cost,
                  std::size_t window,
                  std::size_t passes,
                  const DebugOptions& debug)
{
    const int n = problem.dimension();
    if (window == 0) {
        writeDebugLine(debug, "2-opt window skipped: window=0");
        return;
    }
    for (std::size_t pass = 0; pass < passes; ++pass) {
        const double before = cost;
        bool improved = false;
        for (int i = 1; i < n - 1; ++i) {
            const int upper = std::min(n - 1, i + static_cast<int>(window));
            for (int k = i + 1; k <= upper; ++k) {
                const int a = tour[static_cast<std::size_t>(i - 1)];
                const int b = tour[static_cast<std::size_t>(i)];
                const int c = tour[static_cast<std::size_t>(k)];
                const int d = tour[static_cast<std::size_t>((k + 1) % n)];
                const double ac = problem.distance(a, c);
                const double bd = problem.distance(b, d);
                if (!isFinite(ac) || !isFinite(bd)) {
                    continue;
                }
                const double delta = ac + bd - problem.distance(a, b) - problem.distance(c, d);
                if (delta < -kHeuristicEps) {
                    std::reverse(tour.begin() + i, tour.begin() + k + 1);
                    cost += delta;
                    improved = true;
                    break;
                }
            }
        }
        std::ostringstream line;
        line << "2-opt window pass=" << (pass + 1)
             << " improved=" << (improved ? "yes" : "no")
             << " cost=" << formatDebugDouble(cost)
             << " delta=" << formatDebugDouble(cost - before);
        writeDebugLine(debug, line.str());
        if (!improved) {
            break;
        }
    }
    cost = problemTourCost(problem, tour);
}

} // namespace

SolveResult solveHeuristic(const TspProblem& problem, const HeuristicOptions& options)
{
    SolveResult result;
    const int n = problem.dimension();
    writeDebugLine(options.debug, "heuristic solve started: vertices=" + std::to_string(n));
    if (n < 3) {
        result.cost = std::numeric_limits<double>::infinity();
        writeDebugLine(options.debug, "heuristic solve stopped: fewer than 3 vertices");
        return result;
    }

    std::vector<int> tour = initialHeuristicTour(problem, options);
    double cost = problemTourCost(problem, tour);
    if (!isFinite(cost)) {
        result.cost = std::numeric_limits<double>::infinity();
        writeDebugLine(options.debug, "heuristic solve stopped: initial tour is infeasible");
        return result;
    }
    writeDebugLine(options.debug, "heuristic initial cost=" + formatDebugDouble(cost));

    if (static_cast<std::size_t>(n) <= options.full_two_opt_limit) {
        writeDebugLine(options.debug,
                       "2-opt mode: full passes=" + std::to_string(options.two_opt_passes));
        fullTwoOpt(problem, tour, cost, options.two_opt_passes, options.debug);
    } else {
        writeDebugLine(options.debug,
                       "2-opt mode: window size=" + std::to_string(options.two_opt_window)
                           + " passes=" + std::to_string(options.two_opt_passes));
        windowTwoOpt(problem, tour, cost, options.two_opt_window, options.two_opt_passes, options.debug);
    }

    result.feasible = isFinite(cost);
    result.cost = cost;
    result.tour = std::move(tour);
    writeDebugLine(options.debug,
                   "heuristic solve finished: feasible=" + std::string(result.feasible ? "yes" : "no")
                       + " cost=" + formatDebugDouble(result.cost));
    return result;
}

std::string formatTour(const std::vector<int>& tour)
{
    std::ostringstream out;
    // 输出形如 0 -> 1 -> 2 -> 0，末尾补回起点表示闭合回路。
    for (std::size_t i = 0; i < tour.size(); ++i) {
        if (i != 0) {
            out << " -> ";
        }
        out << tour[i];
    }
    if (!tour.empty()) {
        out << " -> " << tour.front();
    }
    return out.str();
}
