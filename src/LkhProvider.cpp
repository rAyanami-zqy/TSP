#include "LkhProvider.hpp"

#include <cerrno>
#include <climits>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <system_error>

#if defined(__unix__) || defined(__APPLE__)
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace tsp {
namespace {

std::runtime_error systemError(const std::string& operation)
{
    return std::runtime_error(operation + ": " + std::strerror(errno));
}

std::string readProviderLog(const std::filesystem::path& path)
{
    std::ifstream input(path);
    if (!input) return {};
    std::ostringstream content;
    content << input.rdbuf();
    std::string text = content.str();
    constexpr std::size_t kMaximumDiagnosticBytes = 4096;
    if (text.size() > kMaximumDiagnosticBytes) {
        text = text.substr(text.size() - kMaximumDiagnosticBytes);
    }
    while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) {
        text.pop_back();
    }
    return text;
}

#if defined(__unix__) || defined(__APPLE__)
ssize_t writeWithoutSigpipe(int fd, const void* data, std::size_t size)
{
    struct sigaction ignored_action {};
    struct sigaction previous_action {};
    ignored_action.sa_handler = SIG_IGN;
    sigemptyset(&ignored_action.sa_mask);
    if (sigaction(SIGPIPE, &ignored_action, &previous_action) != 0) {
        return -1;
    }
    const ssize_t result = write(fd, data, size);
    const int saved_errno = errno;
    (void)sigaction(SIGPIPE, &previous_action, nullptr);
    errno = saved_errno;
    return result;
}
#endif

void rejectProtocolNewline(const std::string& value, const char* label)
{
    if (value.find('\n') != std::string::npos
        || value.find('\r') != std::string::npos) {
        throw std::runtime_error(std::string(label) + " contains a newline");
    }
}

std::vector<int> readLkhTour(const std::filesystem::path& path, int dimension)
{
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("LKH provider did not create tour file: "
                                 + path.string());
    }
    std::string line;
    bool in_tour = false;
    std::vector<int> tour;
    std::vector<unsigned char> seen(static_cast<std::size_t>(dimension), 0);
    while (std::getline(input, line)) {
        if (!in_tour) {
            if (line == "TOUR_SECTION" || line == "TOUR_SECTION\r") {
                in_tour = true;
            }
            continue;
        }
        std::istringstream row(line);
        long long id = 0;
        if (!(row >> id)) continue;
        if (id == -1) break;
        if (id < 1 || id > dimension
            || seen[static_cast<std::size_t>(id - 1)]) {
            throw std::runtime_error("LKH provider produced an invalid tour");
        }
        seen[static_cast<std::size_t>(id - 1)] = 1;
        tour.push_back(static_cast<int>(id - 1));
    }
    if (!in_tour || tour.size() != static_cast<std::size_t>(dimension)) {
        throw std::runtime_error("LKH provider produced an incomplete tour");
    }
    return tour;
}

void readLkhPotentials(const std::filesystem::path& path,
                       int dimension,
                       double scale,
                       std::vector<double>& potentials,
                       int& root)
{
    std::ifstream input(path);
    long long count = -1;
    if (!input || !(input >> count) || count != dimension) {
        throw std::runtime_error("LKH provider produced an invalid PI file");
    }
    potentials.assign(static_cast<std::size_t>(dimension), 0.0);
    std::vector<unsigned char> seen(static_cast<std::size_t>(dimension), 0);
    for (int index = 0; index < dimension; ++index) {
        long long id = 0;
        double value = 0.0;
        if (!(input >> id >> value) || id < 1 || id > dimension
            || seen[static_cast<std::size_t>(id - 1)]) {
            throw std::runtime_error("LKH provider produced an invalid PI row");
        }
        if (index == 0) root = static_cast<int>(id - 1);
        seen[static_cast<std::size_t>(id - 1)] = 1;
        potentials[static_cast<std::size_t>(id - 1)] = value / scale;
    }
}

} // namespace

LkhProvider::LkhProvider(std::string executable_path)
    : executable_path_(std::move(executable_path))
{
    if (executable_path_.empty()) {
        throw std::runtime_error("LKH provider executable path is empty");
    }
    rejectProtocolNewline(executable_path_, "LKH provider path");
    try {
        start();
    } catch (...) {
        stop();
        if (!workspace_path_.empty()) {
            std::error_code ignored;
            std::filesystem::remove_all(workspace_path_, ignored);
        }
        throw;
    }
}

LkhProvider::~LkhProvider()
{
    stop();
    if (!workspace_path_.empty()) {
        std::error_code ignored;
        std::filesystem::remove_all(workspace_path_, ignored);
    }
}

void LkhProvider::start()
{
#if defined(__unix__) || defined(__APPLE__)
    char workspace[] = "/tmp/tsp-lkh-provider.XXXXXX";
    if (mkdtemp(workspace) == nullptr) throw systemError("mkdtemp");
    workspace_path_ = workspace;

    int request_pipe[2] = {-1, -1};
    int response_pipe[2] = {-1, -1};
    if (pipe(request_pipe) != 0) throw systemError("provider request pipe");
    if (pipe(response_pipe) != 0) {
        close(request_pipe[0]);
        close(request_pipe[1]);
        throw systemError("provider response pipe");
    }

    const pid_t child = fork();
    if (child < 0) {
        close(request_pipe[0]);
        close(request_pipe[1]);
        close(response_pipe[0]);
        close(response_pipe[1]);
        throw systemError("fork LKH provider");
    }
    if (child == 0) {
        if (dup2(request_pipe[0], STDIN_FILENO) < 0
            || dup2(response_pipe[1], STDOUT_FILENO) < 0) {
            _exit(126);
        }
        close(request_pipe[0]);
        close(request_pipe[1]);
        close(response_pipe[0]);
        close(response_pipe[1]);
        execl(executable_path_.c_str(), executable_path_.c_str(),
              static_cast<char*>(nullptr));
        _exit(127);
    }

    close(request_pipe[0]);
    close(response_pipe[1]);
    provider_pid_ = static_cast<long>(child);
    request_fd_ = request_pipe[1];
    response_ = fdopen(response_pipe[0], "r");
    if (response_ == nullptr) {
        close(response_pipe[0]);
        stop();
        throw systemError("fdopen LKH provider response");
    }
    const std::string greeting = readProtocolLine();
    if (greeting != "READY") {
        stop();
        throw std::runtime_error(
            "LKH provider did not become ready (response: " + greeting + ")");
    }
#else
    throw std::runtime_error(
        "the isolated LKH provider requires a POSIX platform");
#endif
}

void LkhProvider::stop() noexcept
{
#if defined(__unix__) || defined(__APPLE__)
    if (request_fd_ >= 0) {
        // Closing the only writer makes getline() in the provider observe EOF,
        // which is equivalent to QUIT and cannot raise SIGPIPE if it died.
        close(request_fd_);
        request_fd_ = -1;
    }
    if (response_ != nullptr) {
        fclose(response_);
        response_ = nullptr;
    }
    if (provider_pid_ > 0) {
        int status = 0;
        while (waitpid(static_cast<pid_t>(provider_pid_), &status, 0) < 0
               && errno == EINTR) {
        }
        provider_pid_ = -1;
    }
#endif
}

std::string LkhProvider::readProtocolLine()
{
    if (response_ == nullptr) {
        throw std::runtime_error("LKH provider response pipe is closed");
    }
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), response_) == nullptr) {
        throw std::runtime_error("LKH provider terminated unexpectedly");
    }
    std::string line(buffer);
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
        line.pop_back();
    }
    return line;
}

LkhProviderResult LkhProvider::run(const std::string& problem_path,
                                   int dimension,
                                   const LkhProviderOptions& options)
{
#if defined(__unix__) || defined(__APPLE__)
    if (dimension < 3) throw std::runtime_error("invalid LKH dimension");
    if (options.runs == 0) throw std::runtime_error("LKH RUNS must be positive");
    if (!(options.potential_scale > 0.0)) {
        throw std::runtime_error("LKH potential scale must be positive");
    }
    if (options.time_limit_seconds < 0.0) {
        throw std::runtime_error("LKH time limit must be non-negative");
    }

    const std::filesystem::path absolute_problem =
        std::filesystem::absolute(problem_path);
    rejectProtocolNewline(absolute_problem.string(), "problem path");
    const std::filesystem::path request_directory =
        std::filesystem::path(workspace_path_)
        / ("request-" + std::to_string(++request_count_));
    std::filesystem::create_directory(request_directory);
    const std::filesystem::path parameter = request_directory / "run.par";
    const std::filesystem::path tour = request_directory / "result.tour";
    const std::filesystem::path pi = request_directory / "result.pi";

    {
        std::ofstream output(parameter);
        if (!output) {
            throw std::runtime_error("failed to create LKH parameter file");
        }
        output << "PROBLEM_FILE = " << absolute_problem.string() << '\n'
               << "OUTPUT_TOUR_FILE = " << tour.string() << '\n';
        if (options.produce_potentials) {
            output << "PI_FILE = " << pi.string() << '\n';
        }
        output << "RUNS = " << options.runs << '\n'
               << "MAX_TRIALS = "
               << (options.max_trials == 0
                       ? static_cast<std::size_t>(dimension)
                       : options.max_trials)
               << '\n'
               << "SEED = " << options.seed << '\n'
               << "TRACE_LEVEL = 0\n";
        if (options.time_limit_seconds > 0.0) {
            output << std::setprecision(
                          std::numeric_limits<double>::max_digits10)
                   << "TOTAL_TIME_LIMIT = "
                   << options.time_limit_seconds << '\n';
        }
        if (!output) {
            throw std::runtime_error("failed to write LKH parameter file");
        }
    }

    const std::string request = parameter.string() + "\n";
    std::size_t written = 0;
    while (written < request.size()) {
        const ssize_t count = writeWithoutSigpipe(
            request_fd_, request.data() + written, request.size() - written);
        if (count < 0) {
            if (errno == EINTR) continue;
            throw systemError("write LKH provider request");
        }
        written += static_cast<std::size_t>(count);
    }

    const std::string response = readProtocolLine();
    std::istringstream fields(response);
    std::string marker;
    int exit_code = -1;
    double seconds = 0.0;
    if (!(fields >> marker >> exit_code >> seconds) || marker != "DONE") {
        throw std::runtime_error("invalid LKH provider response: " + response);
    }
    if (exit_code != 0) {
        const std::filesystem::path log_path = parameter.string() + ".log";
        const std::string diagnostic = readProviderLog(log_path);
        throw std::runtime_error(
            "isolated LKH child failed with exit code "
            + std::to_string(exit_code)
            + (diagnostic.empty() ? std::string{}
                                  : std::string("; LKH output: ") + diagnostic));
    }

    LkhProviderResult result;
    result.provider_seconds = seconds;
    result.tour = readLkhTour(tour, dimension);
    if (options.produce_potentials) {
        readLkhPotentials(pi, dimension, options.potential_scale,
                          result.potentials, result.potential_root);
    }
    return result;
#else
    (void)problem_path;
    (void)dimension;
    (void)options;
    throw std::runtime_error(
        "the isolated LKH provider requires a POSIX platform");
#endif
}

} // namespace tsp
