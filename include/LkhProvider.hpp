#pragma once

#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>

namespace tsp {

// Parameters intentionally kept small and deterministic for the initial-bound
// provider. More specialized LKH experiments can still use a standalone .par.
struct LkhProviderOptions {
    std::size_t runs = 1;
    std::size_t max_trials = 0; // 0 means the instance dimension.
    unsigned long long seed = 1;
    double time_limit_seconds = 0.0; // 0 leaves LKH unlimited.
    bool produce_potentials = true;
    double potential_scale = 100.0;
};

struct LkhProviderResult {
    std::vector<int> tour;
    std::vector<double> potentials;
    int potential_root = 0;
    double provider_seconds = 0.0;
};

// A persistent client for tsp_lkh_provider. The provider process stays alive
// across a batch, but every LKH invocation runs in a forked child so LKH's
// process-global state is never reused between instances.
class LkhProvider {
public:
    explicit LkhProvider(std::string executable_path);
    ~LkhProvider();

    LkhProvider(const LkhProvider&) = delete;
    LkhProvider& operator=(const LkhProvider&) = delete;

    LkhProviderResult run(const std::string& problem_path,
                          int dimension,
                          const LkhProviderOptions& options);

private:
    void start();
    void stop() noexcept;
    std::string readProtocolLine();

    std::string executable_path_;
    std::string workspace_path_;
    long provider_pid_ = -1;
    int request_fd_ = -1;
    FILE* response_ = nullptr;
    std::size_t request_count_ = 0;
};

} // namespace tsp
