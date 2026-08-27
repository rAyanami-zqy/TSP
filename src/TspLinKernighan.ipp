// 精确求解器内部用于收紧 incumbent 的 2-opt/Lin-Kernighan 实现。
void BranchBoundSolver::twoOpt(std::vector<int>& tour, double& cost) const
{
    bool improved = true;
    // 不断寻找能缩短回路的 2-opt 交换，直到没有改进为止。
    while (improved) {
        improved = false;
        for (int i = 1; i < n_ - 1 && !improved; ++i) {
            for (int k = i + 1; k < n_ && !improved; ++k) {
                // 固定 0 号顶点所在位置，从 i..k 翻转中间段。
                int a = tour[i - 1];
                int b = tour[i];
                int c = tour[k];
                int d = tour[(k + 1) % n_];
                // 新边不存在时，不能执行这次 2-opt 交换。
                if (!isFinite(dist_[a][c]) || !isFinite(dist_[b][d])) {
                    continue;
                }
                // 2-opt：用 (a,c)、(b,d) 替换 (a,b)、(c,d)，若总长下降则翻转中间段。
                double delta = dist_[a][c] + dist_[b][d] - dist_[a][b] - dist_[c][d];
                if (delta < -kHeuristicEps) {
                    std::reverse(tour.begin() + i, tour.begin() + k + 1);
                    cost += delta;
                    improved = true;
                }
            }
        }
    }

    // 最后重新计算一次成本，避免多次浮点增量更新累积误差。
    cost = tourCost(tour);
}

// ── Lin-Kernighan 启发式 ──────────────────────────────────────────

std::vector<int> BranchBoundSolver::buildPositionMap(const std::vector<int>& tour) const
{
    std::vector<int> pos(static_cast<std::size_t>(n_), -1);
    for (int i = 0; i < static_cast<int>(tour.size()); ++i) {
        pos[static_cast<std::size_t>(tour[i])] = i;
    }
    return pos;
}

void BranchBoundSolver::buildCandidateSets() const
{
    if (candidate_set_built_) {
        return;
    }
    candidate_set_.resize(static_cast<std::size_t>(n_));
    for (int i = 0; i < n_; ++i) {
        std::vector<std::pair<double, int>> nb;
        nb.reserve(static_cast<std::size_t>(n_) - 1);
        for (int j = 0; j < n_; ++j) {
            if (i == j || !isFinite(dist_[i][j])) {
                continue;
            }
            nb.emplace_back(dist_[i][j], j);
        }
        const int keep = std::min(kLkCandidateSize, static_cast<int>(nb.size()));
        const auto neighbor_less = [](const auto& left, const auto& right) {
            if (left.first != right.first) return left.first < right.first;
            return left.second < right.second;
        };
        if (keep < static_cast<int>(nb.size())) {
            std::nth_element(
                nb.begin(), nb.begin() + keep, nb.end(), neighbor_less);
        }
        nb.resize(static_cast<std::size_t>(keep));
        std::sort(nb.begin(), nb.end(), neighbor_less);
        candidate_set_[static_cast<std::size_t>(i)].reserve(static_cast<std::size_t>(keep));
        for (const auto& p : nb) {
            candidate_set_[static_cast<std::size_t>(i)].push_back(p.second);
        }
    }
    candidate_set_built_ = true;
}

bool BranchBoundSolver::lkSearch(int t1, int t2,
                                  std::vector<int>& next,
                                  std::vector<int>& prev,
                                  std::vector<bool>& active) const
{
    // LK 交替链中的一步：添加 (from,to)，删除 (to,rem)。
    struct LKStep {
        int from = -1;
        int to = -1;
        int rem = -1;
    };

    double best_gain = kHeuristicEps;
    std::vector<LKStep> best_seq;
    std::vector<LKStep> cur_seq;
    std::vector<unsigned char> used(static_cast<std::size_t>(n_), 0);

    used[static_cast<std::size_t>(t1)] = 1;
    used[static_cast<std::size_t>(t2)] = 1;

    // Phase 1: DFS 搜索最佳 k-opt 序列（固定 t4 = next[t3]，与 LK 原论文一致）。
    std::function<void(int, double, int)> dfs = [&](int from, double cum_gain, int depth) {
        if (depth >= kLkMaxDepth) {
            return;
        }
        for (int x : candidate_set_[static_cast<std::size_t>(from)]) {
            // 尝试 close-up：添加 (from, t1) 闭合回路。
            if (x == t1) {
                if (!isFinite(dist_[from][t1])) {
                    continue;
                }
                // 闭合边 (from, t1) 不能是当前 tour 中的边。
                if (next[static_cast<std::size_t>(from)] == t1
                    || prev[static_cast<std::size_t>(from)] == t1) {
                    continue;
                }
                double total = cum_gain - dist_[from][t1];
                if (total > best_gain) {
                    best_gain = total;
                    best_seq = cur_seq;
                }
                continue;
            }
            if (used[static_cast<std::size_t>(x)]) {
                continue;
            }
            if (!isFinite(dist_[from][x])) {
                continue;
            }
            // 增益准则：被加入边必须比累积 gain 小（维持正 gain）。
            if (dist_[from][x] >= cum_gain - kHeuristicEps) {
                continue;
            }

            // 按照 LK 原论文：t4 固定为 next[t3]（一致的前进方向）。
            int y = next[static_cast<std::size_t>(x)];
            if (y == from || used[static_cast<std::size_t>(y)]) {
                continue;
            }

            double new_gain = cum_gain - dist_[from][x] + dist_[x][y];
            if (new_gain <= kHeuristicEps) {
                continue;
            }

            used[static_cast<std::size_t>(y)] = 1;
            cur_seq.push_back({from, x, y});
            dfs(y, new_gain, depth + 1);
            cur_seq.pop_back();
            used[static_cast<std::size_t>(y)] = 0;
        }
    };

    dfs(t2, dist_[t1][t2], 1);

    if (best_seq.empty()) {
        return false;
    }

    // Phase 2: 应用 k-opt 交换。使用邻接表重建 tour。
    std::vector<std::vector<int>> adj(static_cast<std::size_t>(n_));
    for (int v = 0; v < n_; ++v) {
        adj[static_cast<std::size_t>(v)].push_back(next[static_cast<std::size_t>(v)]);
        adj[static_cast<std::size_t>(v)].push_back(prev[static_cast<std::size_t>(v)]);
    }

    auto rm_edge = [&](int u, int v) {
        auto& nu = adj[static_cast<std::size_t>(u)];
        for (auto it = nu.begin(); it != nu.end(); ++it) {
            if (*it == v) { nu.erase(it); break; }
        }
        auto& nv = adj[static_cast<std::size_t>(v)];
        for (auto it = nv.begin(); it != nv.end(); ++it) {
            if (*it == u) { nv.erase(it); break; }
        }
    };

    // 删除第一条边 (t1, t2)。
    rm_edge(t1, t2);

    // 按序列依次添加/删除边。
    int last = t2;
    for (const auto& s : best_seq) {
        adj[static_cast<std::size_t>(s.from)].push_back(s.to);
        adj[static_cast<std::size_t>(s.to)].push_back(s.from);
        rm_edge(s.to, s.rem);
        last = s.rem;
    }

    // 闭合边 (last, t1)。
    adj[static_cast<std::size_t>(last)].push_back(t1);
    adj[static_cast<std::size_t>(t1)].push_back(last);

    // 验证所有顶点度数 = 2 且两个邻居不同（无重复边）。
    for (int v = 0; v < n_; ++v) {
        const auto& nb = adj[static_cast<std::size_t>(v)];
        if (nb.size() != 2) {
            return false;
        }
        if (nb[0] == nb[1]) {
            return false;  // 两个邻居相同，回路无效。
        }
    }

    // 从顶点 0 开始沿邻接表走，验证是否构成单一 Hamilton 回路。
    // 必须恰好 n 步回到 0，且覆盖所有顶点。
    std::vector<int> new_tour(static_cast<std::size_t>(n_));
    std::vector<unsigned char> seen(static_cast<std::size_t>(n_), 0);
    int cur = 0;
    int pv = -1;
    for (int i = 0; i < n_; ++i) {
        new_tour[static_cast<std::size_t>(i)] = cur;
        seen[static_cast<std::size_t>(cur)] = 1;
        const auto& nb = adj[static_cast<std::size_t>(cur)];
        // 选不是来路的邻居作为下一步。
        int nxt = (nb[0] != pv) ? nb[0] : nb[1];
        pv = cur;
        cur = nxt;
    }
    // 必须回到起点且覆盖全部顶点。
    if (cur != 0) {
        return false;
    }
    for (int v = 0; v < n_; ++v) {
        if (!seen[static_cast<std::size_t>(v)]) {
            return false;
        }
    }

    // 重建 next / prev 双向链表。
    for (int i = 0; i < n_; ++i) {
        int u = new_tour[static_cast<std::size_t>(i)];
        int v = new_tour[static_cast<std::size_t>((i + 1) % n_)];
        next[static_cast<std::size_t>(u)] = v;
        prev[static_cast<std::size_t>(v)] = u;
    }

    // 重新激活被改动边的顶点，以便下一轮继续尝试改进。
    active[static_cast<std::size_t>(t1)] = true;
    active[static_cast<std::size_t>(t2)] = true;
    for (const auto& s : best_seq) {
        active[static_cast<std::size_t>(s.from)] = true;
        active[static_cast<std::size_t>(s.to)] = true;
        active[static_cast<std::size_t>(s.rem)] = true;
    }

    return true;
}

bool BranchBoundSolver::linKernighanImprove(std::vector<int>& tour, double& cost) const
{
    // 构建双向链表表示，O(1) 查找前后邻居。
    std::vector<int> next(static_cast<std::size_t>(n_));
    std::vector<int> prev(static_cast<std::size_t>(n_));
    for (int i = 0; i < n_; ++i) {
        int u = tour[static_cast<std::size_t>(i)];
        int v = tour[static_cast<std::size_t>((i + 1) % n_)];
        next[static_cast<std::size_t>(u)] = v;
        prev[static_cast<std::size_t>(v)] = u;
    }

    std::vector<bool> active(static_cast<std::size_t>(n_), true);
    bool improved = false;

    for (int pass = 0; pass < n_; ++pass) {
        bool pass_improved = false;
        for (int idx = 0; idx < n_; ++idx) {
            int t1 = tour[static_cast<std::size_t>(idx)];
            if (!active[static_cast<std::size_t>(t1)]) {
                continue;
            }

            bool found = false;
            // 两个方向都尝试：next[t1] 和 prev[t1] 作为 t2。
            int ngb[2] = {next[static_cast<std::size_t>(t1)],
                          prev[static_cast<std::size_t>(t1)]};
            for (int t2 : ngb) {
                if (lkSearch(t1, t2, next, prev, active)) {
                    improved = true;
                    pass_improved = true;
                    found = true;
                    // 从链表重建 tour。
                    int cur = 0;
                    for (int i = 0; i < n_; ++i) {
                        tour[static_cast<std::size_t>(i)] = cur;
                        cur = next[static_cast<std::size_t>(cur)];
                    }
                    cost = tourCost(tour);
                    break;
                }
            }

            if (!found) {
                active[static_cast<std::size_t>(t1)] = false;
            }
            // 找到改进则重置 don't-look 并重新扫描。
            if (found) {
                // active 已在 lkSearch 中部分重置，此处重置全部以安全。
                std::fill(active.begin(), active.end(), true);
                break;  // 跳出 idx 循环，从头开始本次 pass。
            }
        }
        if (!pass_improved) {
            break;  // 本轮无改进，LK 已收敛。
        }
    }

    return improved;
}

void BranchBoundSolver::doubleBridgeKick(std::vector<int>& tour) const
{
    if (n_ < 8) {
        return;
    }

    // 用基于 n 的确定性断点，避免引入随机数，确保可复现。
    const int a = 1 + (n_ / 7) % std::max(1, n_ / 4 - 1);
    const int b = a + 1 + (n_ / 5) % std::max(1, n_ / 4);
    const int c = b + 1 + (n_ / 3) % std::max(1, n_ / 4);
    if (c >= n_ - 1) {
        return;
    }

    // 四段：A=[0..a], B=[a+1..b], C=[b+1..c], D=[c+1..n_-1]。
    // Double-bridge 重排为 A + C + B + D。
    std::vector<int> kicked;
    kicked.reserve(static_cast<std::size_t>(n_));

    for (int i = 0; i <= a; ++i) {
        kicked.push_back(tour[static_cast<std::size_t>(i)]);
    }
    for (int i = b + 1; i <= c; ++i) {
        kicked.push_back(tour[static_cast<std::size_t>(i)]);
    }
    for (int i = a + 1; i <= b; ++i) {
        kicked.push_back(tour[static_cast<std::size_t>(i)]);
    }
    for (int i = c + 1; i < n_; ++i) {
        kicked.push_back(tour[static_cast<std::size_t>(i)]);
    }

    tour = std::move(kicked);
}

void BranchBoundSolver::linKernighan(std::vector<int>& tour, double& cost) const
{
    buildCandidateSets();

    std::vector<int> best_tour = tour;
    double best_cost = cost;

    for (int kick = 0; kick < kLkMaxKicks; ++kick) {
        // 连续 LK 改进直到局部最优。
        while (linKernighanImprove(tour, cost)) {
            if (cost + kHeuristicEps < best_cost) {
                best_tour = tour;
                best_cost = cost;
            }
        }

        // 更新当前最优。
        if (cost + kHeuristicEps < best_cost) {
            best_tour = tour;
            best_cost = cost;
        }

        if (kick == kLkMaxKicks - 1) {
            break;
        }

        // Double-bridge kick 跳出局部最优。
        doubleBridgeKick(tour);
        cost = tourCost(tour);

        if (cost + kHeuristicEps < best_cost) {
            best_tour = tour;
            best_cost = cost;
        }
    }

    tour = std::move(best_tour);
    cost = best_cost;
}
