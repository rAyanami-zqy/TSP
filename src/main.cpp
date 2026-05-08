#include "TspSolver.hpp"

#include <cmath>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {

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

// 统一的单实例求解入口：调用矩阵解析和 BranchBoundSolver。
tsp::SolveResult solveInput(std::istream& input)
{
    // 主程序只负责输入、调用求解器和输出；算法细节集中在 BranchBoundSolver。
    auto distance = tsp::readDistanceMatrix(input);
    tsp::BranchBoundSolver solver(std::move(distance));
    return solver.solve();
}

// 单实例模式使用人类可读输出，方便手动观察搜索统计。
void printHumanResult(const tsp::SolveResult& result)
{
    std::cout << std::setprecision(10);
    std::cout << "Root lower bound: " << result.stats.root_lower_bound << '\n';
    std::cout << "Initial upper bound: " << result.stats.initial_upper_bound << '\n';
    std::cout << "Nodes created: " << result.stats.nodes_created << '\n';
    std::cout << "Nodes expanded: " << result.stats.nodes_expanded << '\n';
    std::cout << "Pruned by bound: " << result.stats.nodes_pruned_by_bound << '\n';
    std::cout << "Pruned infeasible: " << result.stats.nodes_pruned_infeasible << '\n';

    if (!result.feasible) {
        std::cout << "No feasible Hamiltonian tour found.\n";
        return;
    }

    std::cout << "Optimal cost: " << result.cost << '\n';
    std::cout << "Tour: " << tsp::formatTour(result.tour) << '\n';
}

// 从文件读取一个实例并求解。
int runSingleFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Failed to open input file: " << path << '\n';
        return 2;
    }

    const tsp::SolveResult result = solveInput(file);
    printHumanResult(result);
    return result.feasible ? 0 : 1;
}

// 未传入文件时，从标准输入读取一个实例。
int runSingleStdin()
{
    const tsp::SolveResult result = solveInput(std::cin);
    printHumanResult(result);
    return result.feasible ? 0 : 1;
}

// 批处理模式输出 CSV 表头，便于重定向到结果文件后做统计分析。
void printBatchHeader()
{
    std::cout
        << "instance,status,cost,root_lower_bound,initial_upper_bound,"
        << "nodes_created,nodes_expanded,pruned_by_bound,pruned_infeasible,tour,message\n";
}

// 输出一条批处理记录。result 为空表示文件读取或解析阶段已经失败。
void printBatchRow(const std::string& path,
                   const std::string& status,
                   const tsp::SolveResult* result,
                   const std::string& message)
{
    std::cout << csvQuote(path) << ','
              << status << ',';

    if (result == nullptr) {
        // 读取失败、解析失败等情况没有求解统计，只保留错误信息。
        std::cout << ",,,,,,,,"
                  << csvQuote(message) << '\n';
        return;
    }

    // 有求解结果时，把成本、搜索统计和回路统一写成一行 CSV。
    std::cout << formatDouble(result->cost) << ','
              << formatDouble(result->stats.root_lower_bound) << ','
              << formatDouble(result->stats.initial_upper_bound) << ','
              << result->stats.nodes_created << ','
              << result->stats.nodes_expanded << ','
              << result->stats.nodes_pruned_by_bound << ','
              << result->stats.nodes_pruned_infeasible << ','
              << csvQuote(tsp::formatTour(result->tour)) << ','
              << csvQuote(message) << '\n';
}

// 批处理清单每行一个实例路径；单个实例失败不会中断整个批次。
int runBatch(const std::string& list_path)
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

            tsp::SolveResult result = solveInput(input);
            if (result.feasible) {
                printBatchRow(path, "ok", &result, {});
            } else {
                all_ok = false;
                printBatchRow(path, "infeasible", &result, "no feasible Hamiltonian tour");
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
              << "  " << program << " [matrix-file]\n"
              << "  " << program << " --batch <list-file>\n";
}

} // namespace

int main(int argc, char** argv)
{
    try {
        // 无参数：从标准输入读取矩阵，适合管道或重定向。
        if (argc == 1) {
            return runSingleStdin();
        }

        const std::string first_arg = argv[1];
        // 帮助参数只打印用法，不进入求解流程。
        if (first_arg == "--help" || first_arg == "-h") {
            printUsage(argv[0]);
            return 0;
        }
        // 批处理参数固定需要一个清单文件。
        if (first_arg == "--batch") {
            if (argc != 3) {
                printUsage(argv[0]);
                return 2;
            }
            return runBatch(argv[2]);
        }
        // 一个普通参数：把它当成单个矩阵实例文件。
        if (argc == 2) {
            return runSingleFile(first_arg);
        }

        printUsage(argv[0]);
        return 2;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 2;
    }

    return 0;
}
