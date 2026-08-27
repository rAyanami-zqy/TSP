// 显式距离矩阵的单个权值解析，仅供问题解析模块使用。
// 解析一个距离矩阵元素，支持数字、INF、-、X 等表示缺边的写法。
double parseWeight(const std::string& token)
{
    // 先转小写，兼容 INF / Infinity 等大小写写法。
    std::string lowered;
    lowered.reserve(token.size());
    for (char ch : token) {
        lowered.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
    }

    if (lowered == "inf" || lowered == "infinity" || lowered == "-" || lowered == "x") {
        return std::numeric_limits<double>::infinity();
    }

    // stod 允许部分解析，因此需要检查 parsed 是否刚好到字符串末尾。
    std::size_t parsed = 0;
    double value = std::stod(token, &parsed);
    if (parsed != token.size()) {
        throw std::runtime_error("invalid numeric token: " + token);
    }
    return value;
}
