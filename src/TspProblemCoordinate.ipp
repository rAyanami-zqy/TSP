// TSPLIB 坐标距离模型，包括二维、三维、ATT 与 GEO 等类型。
// 将 TSPLIB GEO 格式的“度.分”坐标转换为弧度。
double geoCoordinateToRadians(double value)
{
    const int degrees = static_cast<int>(value);
    const double minutes = value - static_cast<double>(degrees);
    return kPi * (static_cast<double>(degrees) + 5.0 * minutes / 3.0) / 180.0;
}

// 根据 EDGE_WEIGHT_TYPE 定义的距离计算方法，计算两个点之间的距离。支持多种距离类型，符合 TSPLIB 中的定义。
double coordinateDistance(const Point& a, const Point& b, const std::string& edge_weight_type)
{
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    const double dz = a.z - b.z;
    const std::string type = upperCopy(edge_weight_type);

    if (type == "EUC_2D") {
        return roundedDistance(std::sqrt(dx * dx + dy * dy));
    }
    if (type == "CEIL_2D") {
        return std::ceil(std::sqrt(dx * dx + dy * dy));
    }
    if (type == "FLOOR_2D") {
        return std::floor(std::sqrt(dx * dx + dy * dy));
    }
    if (type == "MAN_2D") {
        return roundedDistance(std::fabs(dx) + std::fabs(dy));
    }
    if (type == "MAX_2D") {
        return roundedDistance(std::max(std::fabs(dx), std::fabs(dy)));
    }
    if (type == "EUC_3D") {
        return roundedDistance(std::sqrt(dx * dx + dy * dy + dz * dz));
    }
    if (type == "CEIL_3D") {
        return std::ceil(std::sqrt(dx * dx + dy * dy + dz * dz));
    }
    if (type == "MAN_3D") {
        return roundedDistance(std::fabs(dx) + std::fabs(dy) + std::fabs(dz));
    }
    if (type == "MAX_3D") {
        return roundedDistance(std::max({std::fabs(dx), std::fabs(dy), std::fabs(dz)}));
    }
    if (type == "ATT") {
        const double rij = std::sqrt((dx * dx + dy * dy) / 10.0);
        const int tij = roundedDistance(rij);
        return tij < rij ? tij + 1 : tij;
    }
    if (type == "GEO") {
        const double lat_a = geoCoordinateToRadians(a.x);
        const double lon_a = geoCoordinateToRadians(a.y);
        const double lat_b = geoCoordinateToRadians(b.x);
        const double lon_b = geoCoordinateToRadians(b.y);
        const double q1 = std::cos(lon_a - lon_b);
        const double q2 = std::cos(lat_a - lat_b);
        const double q3 = std::cos(lat_a + lat_b);
        const double argument = 0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3);
        const double clamped = std::max(-1.0, std::min(1.0, argument));
        return static_cast<int>(6378.388 * std::acos(clamped) + 1.0);
    }

    throw std::runtime_error("unsupported coordinate EDGE_WEIGHT_TYPE: " + edge_weight_type);
}
