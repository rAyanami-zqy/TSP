# PHKMST 根 α-nearness 分支排序实验

## 目标

根 Held–Karp 势优化并构造最小 1-tree 后，对每条根非树边计算一次加入代价：

```text
alpha(u,v) = adjusted_cost(u,v)
             - max_adjusted_cost(root_tree_path(u,v))
```

根关联非树边使用“替换根 1-tree 中较重根边”的差值。根树边定义为
`alpha=0`。该表只作为静态分支先验，不改变按调整权重排序的 Kruskal 候选表。

## 实现范围

新增运行时选项：

```text
--branch-edge-order weight
--branch-edge-order root-alpha-asc
--branch-edge-order root-alpha-desc
```

BP 仍先选择度数最大的违规顶点，再从该顶点关联、尚未决策的当前 1-tree
边中选择：

- `weight`：当前调整权重升序，即 PHKMST 基线；
- `root-alpha-asc`：根 alpha 升序，等值时回退调整权重；
- `root-alpha-desc`：根 alpha 降序，等值时回退调整权重。

根 alpha 预处理为每个内部源点遍历一次根 MST，复杂度 `O(n^2)`、空间
`O(n^2)`，只在启用实验策略时执行。

## PHKMST 九实例结果

势更新使用当前调优配置：`subtree-adaptive`、gap 2.05%、epoch 层距1、最多
16轮、预算5000。

| 分支顺序 | wall | created | expanded | 势尝试 | 势迭代 |
|---|---:|---:|---:|---:|---:|
| weight | **0.84 s** | **161,702** | **100,660** | **2,550** | **19,984** |
| root-alpha-asc | 1.22 s | 289,769 | 200,695 | 3,800 | 32,148 |
| root-alpha-desc | 1.39 s | 350,191 | 201,423 | 4,581 | 34,361 |

相对权重基线，alpha 升序 wall 增加约45%，alpha 降序增加约65%。两种顺序
都令搜索树扩大，并改变浅层访问路径，使 st70 不再命中原先能把搜索树大幅
压缩的势 epoch。

## 关闭节点势更新的隔离实验

| 分支顺序 | wall | created | expanded |
|---|---:|---:|---:|
| weight | **3.73 s** | **2,600,206** | **1,911,618** |
| root-alpha-asc | 4.88 s | 3,495,028 | 2,461,498 |
| root-alpha-desc | 4.60 s | 3,812,661 | 2,374,144 |

没有节点势更新时根 alpha 仍然退化，说明负收益不只是与 PHKMST 触发器的
交互，而是静态加入代价本身不适合作为当前 BP 的主排序。

## 结论

1. 根 alpha 衡量“非树边强制加入根 1-tree 的代价”，而 BP 需要的是“当前
   树边被禁止后的局部下界增益”，目标不一致。
2. 根 1-tree 的树边 alpha 全为零，最重要的浅层分支仍大量回退权重顺序；
   一旦 replacement 边进入当前树，静态 alpha 又可能与当前约束不一致。
3. persistent epoch 会更换势和候选排序，根 alpha 在深层进一步老化。
4. 根 alpha 升序与降序都不应替换 PHKMST 默认权重分支策略。

后续若继续利用 alpha，更合理的范围是：每个成功 epoch 重算局部 alpha，或
只把根 alpha 用作局部 replacement delta 完全相同时的次级 tie-break；不再
把它作为 BP 主排序。

## 验证

- α 数值单元测试覆盖内部非树边、根非树边和树路径最大边公式；
- 两种 α 排序在独立小实例上与穷举最优值一致；
- CTest 4/4 通过；
- 九个经典实例均返回已知最优值。
