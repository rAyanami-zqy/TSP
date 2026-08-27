// 精确分支定界求解前的初始上界构造，以及困难实例上的多起点改善入口。
bool BranchBoundSolver::findInitialTour(
    std::vector<int>& tour, double& cost,
    std::vector<TourCandidate>& alternatives)
{
    alternatives.clear();
    // map 的 key 是消除旋转和反向等价后的规范 tour；value 保留真实方向的
    // tour 及其 2-opt 成本，供后续 diversified LK 使用。
    std::map<std::vector<int>, TourCandidate> distinct_tours;
    // best_* 保持所有最近邻起点中严格最好的候选，作为首次 LK 的输入。
    double best_cost = std::numeric_limits<double>::infinity();
    std::vector<int> best_tour;

    auto canonicalize = [&](const std::vector<int>& candidate) {
        const auto zero = std::find(candidate.begin(), candidate.end(), 0);
        if (zero == candidate.end()) return std::vector<int>{};
        // zero_index 将 tour 旋转到顶点 0 开头；forward/reverse 再消除方向等价。
        const int zero_index = static_cast<int>(
            std::distance(candidate.begin(), zero));
        std::vector<int> forward(static_cast<std::size_t>(n_));
        std::vector<int> reverse(static_cast<std::size_t>(n_));
        for (int offset = 0; offset < n_; ++offset) {
            forward[static_cast<std::size_t>(offset)] = candidate[
                static_cast<std::size_t>((zero_index + offset) % n_)];
            reverse[static_cast<std::size_t>(offset)] = candidate[
                static_cast<std::size_t>((zero_index - offset + n_) % n_)];
        }
        return reverse < forward ? reverse : forward;
    };

    // 从每个起点跑一次最近邻，再用 2-opt 局部改进，取最好的可行回路作为初始上界。
    for (int start = 0; start < n_; ++start) {
        std::vector<int> candidate;
        candidate.reserve(static_cast<std::size_t>(n_));
        std::vector<unsigned char> used(static_cast<std::size_t>(n_), 0);

        // current 是最近邻路径尾点；candidate_cost 是当前开放路径的累计
        // 原始距离，闭合回 start 后才成为 tour 成本。
        int current = start;
        candidate.push_back(current);
        used[current] = 1;
        double candidate_cost = 0.0;
        bool ok = true;

        for (int step = 1; step < n_; ++step) {
            // next/best_edge 保存当前点到所有未访问点中的最轻可用边。
            int next = -1;
            double best_edge = std::numeric_limits<double>::infinity();
            // 最近邻：从当前顶点出发，找尚未访问且距离最短的下一个顶点。
            for (int v = 0; v < n_; ++v) {
                if (used[v] || !isFinite(dist_[current][v])) {
                    continue;
                }
                if (dist_[current][v] < best_edge) {
                    best_edge = dist_[current][v];
                    next = v;
                }
            }
            if (next < 0) {
                // 当前起点的最近邻路径断开，换下一个起点尝试。
                ok = false;
                break;
            }
            candidate_cost += best_edge;
            current = next;
            candidate.push_back(current);
            used[current] = 1;
        }

        if (!ok || !isFinite(dist_[current][start])) {
            // 不能回到起点就不是 Hamilton 回路。
            continue;
        }
        candidate_cost += dist_[current][start];
        // 最近邻只给出初始回路，再用 2-opt 做局部改进。
        twoOpt(candidate, candidate_cost);

        // 保持原实现的严格小于与首次命中语义，确保便宜的首个 LK 起点
        // 不因候选池的规范化或排序而改变。
        if (candidate_cost < best_cost) {
            best_cost = candidate_cost;
            best_tour = candidate;
        }

        std::vector<int> canonical = canonicalize(candidate);
        if (!canonical.empty()) {
            distinct_tours.try_emplace(
                std::move(canonical),
                TourCandidate{candidate_cost, std::move(candidate)});
        }
    }

    if (best_tour.empty()) {
        return false;
    }

    const std::size_t distinct_tour_count = distinct_tours.size();
    distinct_tours.erase(canonicalize(best_tour));

    alternatives.reserve(distinct_tours.size());
    for (auto& [canonical, candidate] : distinct_tours) {
        (void)canonical;
        alternatives.push_back(std::move(candidate));
    }
    std::sort(
        alternatives.begin(), alternatives.end(),
        [](const TourCandidate& left, const TourCandidate& right) {
            if (left.cost != right.cost) return left.cost < right.cost;
            return left.tour < right.tour;
        });

    // 自适应阶段只需保留最有希望的一小组不同局部最优，避免大实例把
    // 所有 n 个 tour 带入多启动 LK。
    constexpr std::size_t kInitialCandidatePoolSize = 24;
    if (alternatives.size() > kInitialCandidatePoolSize) {
        alternatives.resize(kInitialCandidatePoolSize);
    }

    if (debug_.output != nullptr) {
        std::ostringstream line;
        line << "initial tour pool: distinct=" << distinct_tour_count
             << " retained=" << alternatives.size()
             << " best_2opt=" << formatDebugDouble(best_cost);
        if (!alternatives.empty()) {
            line << " next_2opt="
                 << formatDebugDouble(alternatives.front().cost);
        }
        writeDebugLine(debug_, line.str());
    }

    // 用 Lin-Kernighan 对最佳 NN+2opt 结果做精细优化，获取更紧上界。
    linKernighan(best_tour, best_cost);

    tour = std::move(best_tour);
    cost = best_cost;
    return true;
}

bool BranchBoundSolver::improveInitialTourDiversified(
    std::vector<TourCandidate>& alternatives,
    double root_lower_bound,
    std::vector<int>& tour, double& cost)
{
    constexpr std::size_t kDiversifiedLkStarts = 12;
    // start_count 是本次实际运行 LK 的候选上限；starts_run 记录提前停止前
    // 已完成的数量，用于性能日志。
    const std::size_t start_count = std::min(
        alternatives.size(), kDiversifiedLkStarts);
    bool improved = false;
    std::size_t starts_run = 0;

    for (std::size_t index = 0; index < start_count; ++index) {
        std::vector<int> candidate = alternatives[index].tour;
        double candidate_cost = alternatives[index].cost;
        linKernighan(candidate, candidate_cost);
        candidate_cost = tourCost(candidate);
        ++starts_run;

        if (candidate_cost < cost) {
            tour = std::move(candidate);
            cost = candidate_cost;
            improved = true;
        }
        if (isFinite(root_lower_bound)
            && shouldPrune(root_lower_bound, cost)) {
            break;
        }
    }

    writeDebugLine(
        debug_,
        "diversified LK completed: starts=" + std::to_string(starts_run)
            + " best=" + formatDebugDouble(cost));
    return improved;
}
