// TSP/TSPLIB 输入识别与解析。该实现片段由 TspSolver.cpp 包含，
// 不作为独立翻译单元编译，以保持重构前的代码生成方式。
namespace {

std::vector<std::string> tokenizeSectionLine(const std::string& line)
{
    std::istringstream in(line);
    std::vector<std::string> tokens;
    std::string token;
    while (in >> token) {
        if (upperCopy(token) == "EOF") {
            break;
        }
        tokens.push_back(token);
    }
    return tokens;
}

// TSPLIB 的 header line 可能是 "KEY: VALUE" 格式，也可能是 "KEY VALUE" 格式。这个函数解析出 KEY 和 VALUE，并把 KEY 转成大写、VALUE 去掉首尾空白。
std::pair<std::string, std::string> parseTsplibHeaderLine(const std::string& line)
{
    const std::size_t colon = line.find(':');
    if (colon != std::string::npos) {
        return {upperCopy(trimCopy(line.substr(0, colon))), trimCopy(line.substr(colon + 1))};
    }
    // 没有冒号时，按空白分隔 KEY 和 VALUE。
    std::istringstream in(line);
    std::string key;
    in >> key;
    std::string value;
    std::getline(in, value);
    return {upperCopy(key), trimCopy(value)};
}


std::vector<std::vector<double>> buildExplicitMatrix(
    int n,
    const std::string& format,
    const std::vector<double>& values)
{
    std::vector<std::vector<double>> matrix(static_cast<std::size_t>(n),
                                            std::vector<double>(static_cast<std::size_t>(n), 0.0));
    const std::string upper_format = upperCopy(format.empty() ? "FULL_MATRIX" : format);
    std::size_t cursor = 0;

    auto next_value = [&]() {
        if (cursor >= values.size()) {
            throw std::runtime_error("EDGE_WEIGHT_SECTION has fewer values than expected");
        }
        return values[cursor++];
    };

    if (upper_format == "FULL_MATRIX") {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                matrix[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = next_value();
            }
        }
    } else if (upper_format == "UPPER_ROW") {
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                const double value = next_value();
                matrix[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = value;
                matrix[static_cast<std::size_t>(j)][static_cast<std::size_t>(i)] = value;
            }
        }
    } else if (upper_format == "UPPER_DIAG_ROW") {
        for (int i = 0; i < n; ++i) {
            for (int j = i; j < n; ++j) {
                const double value = next_value();
                matrix[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = value;
                matrix[static_cast<std::size_t>(j)][static_cast<std::size_t>(i)] = value;
            }
        }
    } else if (upper_format == "LOWER_ROW") {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < i; ++j) {
                const double value = next_value();
                matrix[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = value;
                matrix[static_cast<std::size_t>(j)][static_cast<std::size_t>(i)] = value;
            }
        }
    } else if (upper_format == "LOWER_DIAG_ROW") {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j <= i; ++j) {
                const double value = next_value();
                matrix[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = value;
                matrix[static_cast<std::size_t>(j)][static_cast<std::size_t>(i)] = value;
            }
        }
    } else if (upper_format == "UPPER_COL") {
        for (int j = 0; j < n; ++j) {
            for (int i = 0; i < j; ++i) {
                const double value = next_value();
                matrix[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = value;
                matrix[static_cast<std::size_t>(j)][static_cast<std::size_t>(i)] = value;
            }
        }
    } else if (upper_format == "UPPER_DIAG_COL") {
        for (int j = 0; j < n; ++j) {
            for (int i = 0; i <= j; ++i) {
                const double value = next_value();
                matrix[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = value;
                matrix[static_cast<std::size_t>(j)][static_cast<std::size_t>(i)] = value;
            }
        }
    } else if (upper_format == "LOWER_COL") {
        for (int j = 0; j < n; ++j) {
            for (int i = j + 1; i < n; ++i) {
                const double value = next_value();
                matrix[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = value;
                matrix[static_cast<std::size_t>(j)][static_cast<std::size_t>(i)] = value;
            }
        }
    } else if (upper_format == "LOWER_DIAG_COL") {
        for (int j = 0; j < n; ++j) {
            for (int i = j; i < n; ++i) {
                const double value = next_value();
                matrix[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = value;
                matrix[static_cast<std::size_t>(j)][static_cast<std::size_t>(i)] = value;
            }
        }
    } else {
        throw std::runtime_error("unsupported EDGE_WEIGHT_FORMAT: " + format);
    }

    return matrix;
}

// 判断输入内容是否看起来像一个纯粹的距离矩阵（没有 TSPLIB 格式的 header 和 section 标记）。
bool looksLikePlainMatrix(const std::string& content)
{
    const std::string trimmed = trimCopy(content);
    if (trimmed.empty()) {
        return false;
    }
    const char first = trimmed.front();
    if (!std::isdigit(static_cast<unsigned char>(first)) && first != '+' && first != '-') {
        return false;
    }
    const std::size_t first_line_end = trimmed.find('\n');
    const std::string first_line = first_line_end == std::string::npos
        ? trimmed
        : trimmed.substr(0, first_line_end);

    return first_line.find(':') == std::string::npos;
}

} // namespace

TspProblem readTspProblem(std::istream& input)
{
    const std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    if (looksLikePlainMatrix(content)) {
        std::istringstream matrix_input(content);
        TspProblem problem;
        problem.name = "matrix";
        problem.type = "TSP";
        problem.edge_weight_type = "EXPLICIT";
        problem.edge_weight_format = "FULL_MATRIX";
        problem.matrix = readDistanceMatrix(matrix_input);
        return problem;
    }

    TspProblem problem;
    std::map<std::string, std::string> header;
    std::vector<double> edge_values;
    enum class Section {
        Header,
        NodeCoord,
        EdgeWeight,
        Ignored
    };
    Section section = Section::Header;

    std::istringstream lines(content);
    std::string line;
    while (std::getline(lines, line)) {

        line = trimCopy(line);
        if (line.empty()) {
            continue;
        }

        const std::string upper = upperCopy(line);
        if (upper == "EOF") {
            break;
        }
        if (upper == "NODE_COORD_SECTION") {
            section = Section::NodeCoord;
            continue;
        }
        if (upper == "EDGE_WEIGHT_SECTION") {
            section = Section::EdgeWeight;
            continue;
        }
        if (isSectionMarker(line)) {
            section = Section::Ignored;
            continue;
        }

        if (section == Section::Header) {
            auto [key, value] = parseTsplibHeaderLine(line);
            if (!key.empty()) {
                header[key] = value;
            }
        } else if (section == Section::NodeCoord) {
            const std::vector<std::string> tokens = tokenizeSectionLine(line);
            if (tokens.size() < 3) {
                continue;
            }
            const int id = std::stoi(tokens[0]);
            const int n = header.count("DIMENSION") ? std::stoi(header["DIMENSION"]) : id;
            if (static_cast<int>(problem.coordinates.size()) < n) {
                problem.coordinates.resize(static_cast<std::size_t>(n));
            }
            if (id < 1 || id > n) {
                throw std::runtime_error("NODE_COORD_SECTION contains an out-of-range node id");
            }
            Point point;
            point.x = std::stod(tokens[1]);
            point.y = std::stod(tokens[2]);
            if (tokens.size() >= 4) {
                point.z = std::stod(tokens[3]);
            }
            problem.coordinates[static_cast<std::size_t>(id - 1)] = point;
        } else if (section == Section::EdgeWeight) {
            const std::vector<std::string> tokens = tokenizeSectionLine(line);
            for (const std::string& token : tokens) {
                edge_values.push_back(parseWeight(token));
            }
        }
    }

    problem.name = header.count("NAME") ? header["NAME"] : "tsplib";
    problem.type = header.count("TYPE") ? upperCopy(header["TYPE"]) : "TSP";
    problem.edge_weight_type = header.count("EDGE_WEIGHT_TYPE")
        ? upperCopy(header["EDGE_WEIGHT_TYPE"])
        : (problem.coordinates.empty() ? "EXPLICIT" : "EUC_2D");
    problem.edge_weight_format = header.count("EDGE_WEIGHT_FORMAT")
        ? upperCopy(header["EDGE_WEIGHT_FORMAT"])
        : (problem.edge_weight_type == "EXPLICIT" ? "FULL_MATRIX" : "");

    const int dimension = header.count("DIMENSION") ? std::stoi(header["DIMENSION"]) : problem.dimension();
    if (dimension < 3) {
        throw std::runtime_error("TSP instance must contain at least 3 vertices");
    }

    if (problem.edge_weight_type == "EXPLICIT") {
        problem.matrix = buildExplicitMatrix(dimension, problem.edge_weight_format, edge_values);
        problem.coordinates.clear();
    } else if (static_cast<int>(problem.coordinates.size()) != dimension) {
        throw std::runtime_error("NODE_COORD_SECTION does not match DIMENSION");
    }

    return problem;
}

std::vector<std::vector<double>> readDistanceMatrix(std::istream& input)
{
    int n = 0;
    // 输入第一项必须是顶点数。
    if (!(input >> n)) {
        throw std::runtime_error("input must start with the number of vertices");
    }
    if (n < 3) {
        throw std::runtime_error("TSP instance must contain at least 3 vertices");
    }
    //nxn的的距离矩阵，初始化为0.0
    std::vector<std::vector<double>> distance(static_cast<std::size_t>(n),
                                              std::vector<double>(static_cast<std::size_t>(n), 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            std::string token;
            // 矩阵必须完整给出 n*n 个 token。
            if (!(input >> token)) {
                throw std::runtime_error("distance matrix has fewer entries than expected");
            }
            // parseWeight 负责把 x / inf 等缺边符号转换为 infinity。
            distance[i][j] = parseWeight(token);
        }
    }
    return distance;
}
