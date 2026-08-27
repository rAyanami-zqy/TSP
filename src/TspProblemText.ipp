// 输入文本规范化与基础数值转换，仅供问题解析模块使用。
std::string trimCopy(const std::string& text)
{
    const std::string whitespace = " \t\r\n";
    const std::size_t begin = text.find_first_not_of(whitespace);
    if (begin == std::string::npos) {
        return {};
    }
    const std::size_t end = text.find_last_not_of(whitespace);
    return text.substr(begin, end - begin + 1);
}

// 将字符串转换为全大写形式，返回新的字符串副本。输入中的 section marker 可能大小写不一致。
std::string upperCopy(std::string text)
{
    for (char& ch : text) {
        ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }
    return text;
}

// 判断一行文本是否是输入文件中的 section marker，标记了不同类型数据的开始。输入中可能有多余空格和大小写不一致。
bool isSectionMarker(const std::string& line)
{
    const std::string upper = upperCopy(trimCopy(line));
    return upper == "NODE_COORD_SECTION"
        || upper == "EDGE_WEIGHT_SECTION"
        || upper == "DISPLAY_DATA_SECTION"
        || upper == "TOUR_SECTION"
        || upper == "DEPOT_SECTION"
        || upper == "DEMAND_SECTION"
        || upper == "EOF";
}

// 将 double 类型的距离值四舍五入到最近的整数，符合 TSPLIB 中某些 EDGE_WEIGHT_TYPE 的定义。
int roundedDistance(double value)
{
    return static_cast<int>(std::floor(value + 0.5));
}
