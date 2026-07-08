# TSP Solver Heatmap Analysis — 函数级性能剖析

`tools/heatmap_analysis.py` 使用 valgrind callgrind 对 tsp_bb 求解过程进行函数级性能剖析，并生成更易读的函数耗时占比报告。

注意：callgrind 直接采集的是 `Ir`（instruction read / 指令数），不是原生运行时的逐函数秒数。脚本会先原生运行一次求解器得到总耗时，再按 callgrind 的函数占比换算出每个函数的估算秒数。因此报告中的函数秒数是“按指令占比缩放的原生耗时估算”，适合定位热点和比较优化前后变化。

## 快速开始

```bash
# 运行求解器并采集 profiling 数据
python3 tools/heatmap_analysis.py examples/five-city.txt --profile

# 回放已有 profiling 数据
python3 tools/heatmap_analysis.py examples/five-city.txt \
    --load-profile docs/heatmap/five-city
```

## 命令行选项

| 选项 | 说明 |
|---|---|
| `instance` | TSP 实例文件路径（纯矩阵或 TSPLIB `.tsp`） |
| `--profile` | 使用 valgrind callgrind 采集函数级性能数据 |
| `--solver PATH` | tsp_bb 二进制路径（默认 `build/tsp_bb`） |
| `--strategy STRATEGY` | 仅旧版求解器支持 `--branch-strategy` 时传入 |
| `--timeout N` | 求解器超时秒数（默认 1800；callgrind 下约 3x 慢） |
| `--load-profile DIR` | 回放已保存的 profiling 目录 |
| `--out-dir DIR` | 自定义输出目录 |
| `--top-n N` | 图表和文本报告显示前 N 个函数（默认 30） |
| `--scope solver\|solver-runtime\|all` | 显示求解器函数、求解器+运行时函数或全部函数 |
| `--no-native-timing` | 不执行原生计时，只输出 callgrind 百分比 |

## 输出文件

| 文件 | 内容 |
|---|---|
| `function_time_breakdown.csv` | 每个函数的 self/inclusive Ir、百分比和估算秒数 |
| `report.md` | Markdown 汇总报告，含测量说明、求解统计和 Top 函数表 |
| `01_self_time_share.png` | 按 self cost 排序的水平柱状图，直接标注百分比和估算秒数 |
| `02_inclusive_time_share.png` | inclusive 与 self cost 对比图 |
| `03_function_metric_heatmap.png` | 函数指标热力图：Self%、Inclusive%、Self ms、Inclusive ms |
| `function_overhead.png` | 兼容旧文件名，内容同 `01_self_time_share.png` |
| `callgrind.out` | callgrind 原始数据 |
| `callgrind_self.txt` | self/exclusive callgrind_annotate 输出 |
| `callgrind_inclusive.txt` | inclusive callgrind_annotate 输出 |
| `callgrind_tree.txt` | 调用树输出，供人工追查调用关系 |
| `profile_metadata.json` | 原生总耗时、求解统计和采集配置 |

## 典型用法

```bash
# 完整 profiling
python3 tools/heatmap_analysis.py data/classic/tsplib/burma14.tsp --profile

# 对比两种策略
python3 tools/heatmap_analysis.py data/classic/tsplib/gr17.tsp --profile \
    --out-dir docs/heatmap/gr17_current
python3 tools/heatmap_analysis.py data/classic/tsplib/gr17.tsp --profile \
    --solver solver/tsp_bb_26_07_07/tsp_bb \
    --out-dir docs/heatmap/gr17_260707

# 回放已有数据
python3 tools/heatmap_analysis.py data/classic/tsplib/burma14.tsp \
    --load-profile docs/heatmap/burma14
```

## 热力图解读

### `01_self_time_share.png`
- 横条越长 = 函数自身指令数占总程序指令数越高。
- 标签格式为 `百分比 / 估算秒数`。
- self cost 是排查“函数自身实现开销”的首选视图。

### `02_inclusive_time_share.png`
- inclusive 包含被调函数开销，self 只包含函数本体开销。
- 如果 inclusive 高但 self 低，说明热点在其子调用中。
- inclusive 不是可加指标，不能把多行 inclusive 秒数相加。

### `03_function_metric_heatmap.png`
- 每列独立归一化着色，单元格文字显示实际数值。
- 用于同时比较 Self%、Inclusive%、Self ms、Inclusive ms。

## 依赖

```bash
pip install matplotlib
# valgrind 通常已预装，否则: sudo apt install valgrind
```
