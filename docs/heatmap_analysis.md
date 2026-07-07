# TSP Solver Heatmap Analysis — 函数级性能剖析

`tools/heatmap_analysis.py` 使用 valgrind callgrind 对 tsp_bb 求解过程进行函数级性能剖析，生成热力图。

## 快速开始

```bash
# 运行求解器并采集 profiling 数据
python3 tools/heatmap_analysis.py examples/five-city.txt --profile

# 不 profiling，仅生成搜索进度图
python3 tools/heatmap_analysis.py examples/five-city.txt
```

## 命令行选项

| 选项 | 说明 |
|---|---|
| `instance` | TSP 实例文件路径（纯矩阵或 TSPLIB `.tsp`） |
| `--profile` | 使用 valgrind callgrind 采集函数级性能数据 |
| `--solver PATH` | tsp_bb 二进制路径（默认 `build/tsp_bb`） |
| `--strategy smart\|simple` | 分支策略（默认 `smart`） |
| `--timeout N` | 求解器超时秒数（默认 1800；callgrind 下约 3x 慢） |
| `--load-log FILE` | 回放 debug log |
| `--load-profile DIR` | 回放已保存的 profiling 目录 |
| `--save-log` | 保存 debug log |
| `--out-dir DIR` | 自定义输出目录 |

## 输出文件

| 文件 | 内容 |
|---|---|
| `00_dashboard.png` | 6 合 1 总览：函数排行 + 时间线热力图 + 饼图 + 强度图 + 统计 + Top-10 |
| `01_function_overhead.png` | 函数 CPU 开销（Ir 指令数）水平柱状图 + 热力强度列 |
| `02_function_timeline.png` | Top-20 函数 × 时间片段的活跃度热力图 |
| `03_call_graph.png` | 函数共现矩阵（sqrt 开销乘积） |
| `04_search_progress.png` | 搜索深度×下界 2D 直方图 + 上下界收敛曲线 |
| `report.html` | HTML 汇总报告，包含 Top-15 函数表格 |
| `callgrind.out` | callgrind 原始数据 |
| `callgrind_annotate.txt` | callgrind_annotate 人类可读输出 |

## 典型用法

```bash
# 完整 profiling
python3 tools/heatmap_analysis.py data/classic/tsplib/burma14.tsp --profile

# 对比两种策略
python3 tools/heatmap_analysis.py data/classic/tsplib/gr17.tsp --profile \
    --strategy smart --out-dir docs/heatmap/gr17_smart
python3 tools/heatmap_analysis.py data/classic/tsplib/gr17.tsp --profile \
    --strategy simple --out-dir docs/heatmap/gr17_simple

# 保存数据供离线分析
python3 tools/heatmap_analysis.py data/classic/tsplib/burma14.tsp --profile --save-log

# 回放已有数据
python3 tools/heatmap_analysis.py data/classic/tsplib/burma14.tsp \
    --load-profile docs/heatmap/burma14 --load-log docs/heatmap/burma14/debug.log
```

## 热力图解读

### 函数开销图（01）
- 横条越长 = 该函数执行的 CPU 指令（Ir）越多
- 颜色越深 = 相对开销越高
- 重点关注占比最高的 3~5 个函数

### 函数时间线图（02）
- 每行是一个函数，列是时间片段
- 亮色 = 该时间段内该函数被频繁采样
- 观察热点函数的活跃时段是否集中在求解初期/后期

### 仪表板（00）
- 左上：函数排行柱状图（按 Ir 计数）
- 右上：时间线热力图
- 中右：饼图（Top-8 函数占比）
- 左下：开销强度列热力图
- 右下：Top-10 函数柱状图

## 依赖

```bash
pip install matplotlib numpy
# valgrind 通常已预装，否则: sudo apt install valgrind
```
