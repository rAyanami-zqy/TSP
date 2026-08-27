// TspProblem 的数据访问与稠密距离矩阵物化。
int TspProblem::dimension() const
{
    if (!matrix.empty()) {
        return static_cast<int>(matrix.size());
    }
    return static_cast<int>(coordinates.size());
}

bool TspProblem::hasCoordinates() const
{
    return !coordinates.empty();
}

bool TspProblem::hasDenseMatrix() const
{
    return !matrix.empty();
}

double TspProblem::distance(int u, int v) const
{
    if (u == v) {
        return 0.0;
    }
    if (!matrix.empty()) {
        return matrix[static_cast<std::size_t>(u)][static_cast<std::size_t>(v)];
    }
    if (!coordinates.empty()) {
        return coordinateDistance(
            coordinates[static_cast<std::size_t>(u)],
            coordinates[static_cast<std::size_t>(v)],
            edge_weight_type.empty() ? "EUC_2D" : edge_weight_type);
    }
    throw std::runtime_error("problem has neither a matrix nor coordinates");
}

std::vector<std::vector<double>> TspProblem::toDenseMatrix(std::size_t max_dimension) const
{
    const int n = dimension();
    if (n <= 0) {
        throw std::runtime_error("TSP problem has no vertices");
    }
    if (max_dimension != 0 && static_cast<std::size_t>(n) > max_dimension) {
        throw std::runtime_error("instance dimension exceeds dense exact limit");
    }
    if (!matrix.empty()) {
        return matrix;
    }

    std::vector<std::vector<double>> dense(static_cast<std::size_t>(n),
                                           std::vector<double>(static_cast<std::size_t>(n), 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            const double value = distance(i, j);
            dense[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = value;
            dense[static_cast<std::size_t>(j)][static_cast<std::size_t>(i)] = value;
        }
    }
    return dense;
}
