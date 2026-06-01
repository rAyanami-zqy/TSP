# TSP Solver Results — Smart Branching Strategy (Debug)

**Total instances found:** 24  
**Solver:** `tsp_bb --method exact --branch-strategy smart --exact-max-n 20 --debug`  
**Successful:** 23  
**Failed:** 1  

**Max vertices:** < 20  

---

## 1. `data/classic/tsplib/burma14.tsp` (n=14)

```
Problem: burma14
```

```
Dimension: 14
```

```
Method: exact
```

```
Root lower bound: 2542
```

```
Initial upper bound: 3323
```

```
Nodes created: 273087
```

```
Nodes expanded: 186707
```

```
Pruned by bound: 86380
```

```
Pruned infeasible: 100328
```

```
Optimal cost: 3323
```

```
Tour: 1 -> 0 -> 9 -> 8 -> 10 -> 7 -> 12 -> 6 -> 11 -> 5 -> 4 -> 3 -> 2 -> 13 -> 1
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=burma14 dimension=14 method=exact
[tsp-debug] exact solve started: vertices=14
[tsp-debug] initial incumbent: cost=3323
[tsp-debug] root: lower_bound=2542 search=recursive-dfs strategy=smart
[tsp-debug] progress: expanded=1000 created=1391 depth=20 bound=3283 best=3323 pruned_bound=384 pruned_infeasible=608
[tsp-debug] progress: expanded=2000 created=2827 depth=23 bound=3300 best=3323 pruned_bound=820 pruned_infeasible=1172
[tsp-debug] progress: expanded=3000 created=4305 depth=27 bound=3251 best=3323 pruned_bound=1301 pruned_infeasible=1694
[tsp-debug] progress: expanded=4000 created=5727 depth=22 bound=3228 best=3323 pruned_bound=1720 pruned_infeasible=2272
[tsp-debug] progress: expanded=5000 created=7204 depth=27 bound=3294 best=3323 pruned_bound=2198 pruned_infeasible=2795
[tsp-debug] progress: expanded=6000 created=8668 depth=29 bound=3303 best=3323 pruned_bound=2662 pruned_infeasible=3331
[tsp-debug] progress: expanded=7000 created=10204 depth=18 bound=3175 best=3323 pruned_bound=3195 pruned_infeasible=3795
[tsp-debug] progress: expanded=8000 created=11592 depth=24 bound=3309 best=3323 pruned_bound=3586 pruned_infeasible=4407
[tsp-debug] progress: expanded=9000 created=13011 depth=29 bound=3287 best=3323 pruned_bound=4005 pruned_infeasible=4988
[tsp-debug] progress: expanded=10000 created=14411 depth=19 bound=3076 best=3323 pruned_bound=4403 pruned_infeasible=5588
[tsp-debug] progress: expanded=11000 created=15846 depth=20 bound=3217 best=3323 pruned_bound=4839 pruned_infeasible=6153
[tsp-debug] progress: expanded=12000 created=17262 depth=22 bound=3309 best=3323 pruned_bound=5256 pruned_infeasible=6737
[tsp-debug] progress: expanded=13000 created=18709 depth=20 bound=3245 best=3323 pruned_bound=5704 pruned_infeasible=7290
[tsp-debug] progress: expanded=14000 created=20162 depth=19 bound=3104 best=3323 pruned_bound=6157 pruned_infeasible=7837
[tsp-debug] progress: expanded=15000 created=21595 depth=22 bound=3091 best=3323 pruned_bound=6587 pruned_infeasible=8404
[tsp-debug] progress: expanded=16000 created=23025 depth=24 bound=3312 best=3323 pruned_bound=7018 pruned_infeasible=8974
[tsp-debug] progress: expanded=17000 created=24481 depth=16 bound=3206 best=3323 pruned_bound=7476 pruned_infeasible=9518
[tsp-debug] progress: expanded=18000 created=25996 depth=17 bound=3199 best=3323 pruned_bound=7990 pruned_infeasible=10003
[tsp-debug] progress: expanded=19000 created=27466 depth=20 bound=3305 best=3323 pruned_bound=8460 pruned_infeasible=10533
[tsp-debug] progress: expanded=20000 created=28905 depth=28 bound=3240 best=3323 pruned_bound=8899 pruned_infeasible=11094
[tsp-debug] progress: expanded=21000 created=30352 depth=21 bound=3291 best=3323 pruned_bound=9346 pruned_infeasible=11647
[tsp-debug] progress: expanded=22000 created=31842 depth=21 bound=3144 best=3323 pruned_bound=9838 pruned_infeasible=12157
[tsp-debug] progress: expanded=23000 created=33283 depth=15 bound=3037 best=3323 pruned_bound=10278 pruned_infeasible=12716
[tsp-debug] progress: expanded=24000 created=34680 depth=16 bound=3069 best=3323 pruned_bound=10676 pruned_infeasible=13319
[tsp-debug] progress: expanded=25000 created=36117 depth=29 bound=3282 best=3323 pruned_bound=11110 pruned_infeasible=13882
[tsp-debug] progress: expanded=26000 created=37551 depth=18 bound=3222 best=3323 pruned_bound=11544 pruned_infeasible=14448
[tsp-debug] progress: expanded=27000 created=38998 depth=24 bound=3185 best=3323 pruned_bound=11991 pruned_infeasible=15001
[tsp-debug] progress: expanded=28000 created=40456 depth=21 bound=3137 best=3323 pruned_bound=12449 pruned_infeasible=15543
[tsp-debug] progress: expanded=29000 created=41923 depth=17 bound=3152 best=3323 pruned_bound=12918 pruned_infeasible=16076
[tsp-debug] progress: expanded=30000 created=43394 depth=24 bound=3258 best=3323 pruned_bound=13389 pruned_infeasible=16605
[tsp-debug] progress: expanded=31000 created=44926 depth=19 bound=3295 best=3323 pruned_bound=13922 pruned_infeasible=17073
[tsp-debug] progress: expanded=32000 created=46432 depth=14 bound=3210 best=3323 pruned_bound=14426 pruned_infeasible=17567
[tsp-debug] progress: expanded=33000 created=47867 depth=22 bound=3279 best=3323 pruned_bound=14863 pruned_infeasible=18132
[tsp-debug] progress: expanded=34000 created=49409 depth=26 bound=3249 best=3323 pruned_bound=15405 pruned_infeasible=18590
[tsp-debug] progress: expanded=35000 created=50833 depth=19 bound=3309 best=3323 pruned_bound=15825 pruned_infeasible=19166
[tsp-debug] progress: expanded=36000 created=52272 depth=24 bound=3249 best=3323 pruned_bound=16264 pruned_infeasible=19727
[tsp-debug] progress: expanded=37000 created=53627 depth=21 bound=3309 best=3323 pruned_bound=16620 pruned_infeasible=20372
[tsp-debug] progress: expanded=38000 created=55011 depth=27 bound=3237 best=3323 pruned_bound=17003 pruned_infeasible=20988
[tsp-debug] progress: expanded=39000 created=56408 depth=20 bound=3296 best=3323 pruned_bound=17401 pruned_infeasible=21591
[tsp-debug] progress: expanded=40000 created=57846 depth=29 bound=3288 best=3323 pruned_bound=17841 pruned_infeasible=22153
[tsp-debug] progress: expanded=41000 created=59319 depth=27 bound=3270 best=3323 pruned_bound=18312 pruned_infeasible=22680
[tsp-debug] progress: expanded=42000 created=60729 depth=20 bound=2919 best=3323 pruned_bound=18723 pruned_infeasible=23270
[tsp-debug] progress: expanded=43000 created=62107 depth=24 bound=3267 best=3323 pruned_bound=19101 pruned_infeasible=23892
[tsp-debug] progress: expanded=44000 created=63587 depth=21 bound=3277 best=3323 pruned_bound=19580 pruned_infeasible=24412
[tsp-debug] progress: expanded=45000 created=65026 depth=21 bound=3314 best=3323 pruned_bound=20021 pruned_infeasible=24973
[tsp-debug] progress: expanded=46000 created=66515 depth=24 bound=3282 best=3323 pruned_bound=20510 pruned_infeasible=25484
[tsp-debug] progress: expanded=47000 created=68011 depth=30 bound=3111 best=3323 pruned_bound=21007 pruned_infeasible=25988
[tsp-debug] progress: expanded=48000 created=69366 depth=25 bound=3299 best=3323 pruned_bound=21356 pruned_infeasible=26633
[tsp-debug] progress: expanded=49000 created=70787 depth=16 bound=3107 best=3323 pruned_bound=21782 pruned_infeasible=27212
[tsp-debug] progress: expanded=50000 created=72238 depth=25 bound=3252 best=3323 pruned_bound=22231 pruned_infeasible=27761
[tsp-debug] progress: expanded=51000 created=73683 depth=16 bound=3170 best=3323 pruned_bound=22678 pruned_infeasible=28316
[tsp-debug] progress: expanded=52000 created=75078 depth=16 bound=3018 best=3323 pruned_bound=23072 pruned_infeasible=28921
[tsp-debug] progress: expanded=53000 created=76522 depth=24 bound=3142 best=3323 pruned_bound=23516 pruned_infeasible=29477
[tsp-debug] progress: expanded=54000 created=77970 depth=17 bound=3249 best=3323 pruned_bound=23965 pruned_infeasible=30029
[tsp-debug] progress: expanded=55000 created=79462 depth=24 bound=3121 best=3323 pruned_bound=24456 pruned_infeasible=30537
[tsp-debug] progress: expanded=56000 created=80920 depth=30 bound=3131 best=3323 pruned_bound=24915 pruned_infeasible=31079
[tsp-debug] progress: expanded=57000 created=82354 depth=25 bound=3258 best=3323 pruned_bound=25344 pruned_infeasible=31645
[tsp-debug] progress: expanded=58000 created=83798 depth=25 bound=3234 best=3323 pruned_bound=25792 pruned_infeasible=32201
[tsp-debug] progress: expanded=59000 created=85248 depth=27 bound=3312 best=3323 pruned_bound=26241 pruned_infeasible=32751
[tsp-debug] progress: expanded=60000 created=86649 depth=25 bound=3309 best=3323 pruned_bound=26642 pruned_infeasible=33350
[tsp-debug] progress: expanded=61000 created=88147 depth=27 bound=3227 best=3323 pruned_bound=27141 pruned_infeasible=33852
[tsp-debug] progress: expanded=62000 created=89601 depth=21 bound=3242 best=3323 pruned_bound=27596 pruned_infeasible=34398
[tsp-debug] progress: expanded=63000 created=91086 depth=23 bound=3194 best=3323 pruned_bound=28079 pruned_infeasible=34913
[tsp-debug] progress: expanded=64000 created=92608 depth=29 bound=3227 best=3323 pruned_bound=28601 pruned_infeasible=35391
[tsp-debug] progress: expanded=65000 created=94104 depth=22 bound=3230 best=3323 pruned_bound=29098 pruned_infeasible=35895
[tsp-debug] progress: expanded=66000 created=95529 depth=16 bound=3112 best=3323 pruned_bound=29524 pruned_infeasible=36470
[tsp-debug] progress: expanded=67000 created=97017 depth=22 bound=3295 best=3323 pruned_bound=30009 pruned_infeasible=36982
[tsp-debug] progress: expanded=68000 created=98364 depth=20 bound=3230 best=3323 pruned_bound=30358 pruned_infeasible=37635
[tsp-debug] progress: expanded=69000 created=99756 depth=29 bound=3310 best=3323 pruned_bound=30748 pruned_infeasible=38243
[tsp-debug] progress: expanded=70000 created=101234 depth=24 bound=3272 best=3323 pruned_bound=31229 pruned_infeasible=38765
[tsp-debug] progress: expanded=71000 created=102738 depth=30 bound=3190 best=3323 pruned_bound=31733 pruned_infeasible=39261
[tsp-debug] progress: expanded=72000 created=104189 depth=9 bound=2763 best=3323 pruned_bound=32186 pruned_infeasible=39810
[tsp-debug] progress: expanded=73000 created=105678 depth=23 bound=3299 best=3323 pruned_bound=32672 pruned_infeasible=40321
[tsp-debug] progress: expanded=74000 created=107130 depth=22 bound=3281 best=3323 pruned_bound=33124 pruned_infeasible=40869
[tsp-debug] progress: expanded=75000 created=108605 depth=22 bound=3232 best=3323 pruned_bound=33599 pruned_infeasible=41394
[tsp-debug] progress: expanded=76000 created=110085 depth=26 bound=3113 best=3323 pruned_bound=34081 pruned_infeasible=41914
[tsp-debug] progress: expanded=77000 created=111547 depth=30 bound=3120 best=3323 pruned_bound=34543 pruned_infeasible=42452
[tsp-debug] progress: expanded=78000 created=113004 depth=19 bound=3188 best=3323 pruned_bound=34998 pruned_infeasible=42995
[tsp-debug] progress: expanded=79000 created=114404 depth=25 bound=3006 best=3323 pruned_bound=35397 pruned_infeasible=43595
[tsp-debug] progress: expanded=80000 created=115832 depth=29 bound=3316 best=3323 pruned_bound=35825 pruned_infeasible=44167
[tsp-debug] progress: expanded=81000 created=117250 depth=25 bound=3303 best=3323 pruned_bound=36242 pruned_infeasible=44749
[tsp-debug] progress: expanded=82000 created=118750 depth=29 bound=3275 best=3323 pruned_bound=36744 pruned_infeasible=45249
[tsp-debug] progress: expanded=83000 created=120143 depth=34 bound=3126 best=3323 pruned_bound=37137 pruned_infeasible=45856
[tsp-debug] progress: expanded=84000 created=121521 depth=27 bound=3033 best=3323 pruned_bound=37515 pruned_infeasible=46478
[tsp-debug] progress: expanded=85000 created=122995 depth=23 bound=3026 best=3323 pruned_bound=37990 pruned_infeasible=47004
[tsp-debug] progress: expanded=86000 created=124435 depth=20 bound=3295 best=3323 pruned_bound=38430 pruned_infeasible=47564
[tsp-debug] progress: expanded=87000 created=125885 depth=27 bound=3291 best=3323 pruned_bound=38879 pruned_infeasible=48114
[tsp-debug] progress: expanded=88000 created=127356 depth=19 bound=2977 best=3323 pruned_bound=39351 pruned_infeasible=48643
[tsp-debug] progress: expanded=89000 created=128834 depth=29 bound=3282 best=3323 pruned_bound=39828 pruned_infeasible=49165
[tsp-debug] progress: expanded=90000 created=130293 depth=23 bound=3121 best=3323 pruned_bound=40288 pruned_infeasible=49706
[tsp-debug] progress: expanded=91000 created=131709 depth=24 bound=3140 best=3323 pruned_bound=40704 pruned_infeasible=50290
[tsp-debug] progress: expanded=92000 created=133214 depth=20 bound=3244 best=3323 pruned_bound=41209 pruned_infeasible=50785
[tsp-debug] progress: expanded=93000 created=134751 depth=29 bound=3242 best=3323 pruned_bound=41745 pruned_infeasible=51248
[tsp-debug] progress: expanded=94000 created=136236 depth=24 bound=3153 best=3323 pruned_bound=42230 pruned_infeasible=51763
[tsp-debug] progress: expanded=95000 created=137757 depth=17 bound=3277 best=3323 pruned_bound=42755 pruned_infeasible=52242
[tsp-debug] progress: expanded=96000 created=139259 depth=21 bound=3136 best=3323 pruned_bound=43253 pruned_infeasible=52740
[tsp-debug] progress: expanded=97000 created=140765 depth=21 bound=3193 best=3323 pruned_bound=43760 pruned_infeasible=53234
[tsp-debug] progress: expanded=98000 created=142278 depth=29 bound=3279 best=3323 pruned_bound=44273 pruned_infeasible=53721
[tsp-debug] progress: expanded=99000 created=143779 depth=28 bound=3220 best=3323 pruned_bound=44774 pruned_infeasible=54220
[tsp-debug] progress: expanded=100000 created=145270 depth=19 bound=3257 best=3323 pruned_bound=45264 pruned_infeasible=54729
[tsp-debug] progress: expanded=101000 created=146757 depth=19 bound=2914 best=3323 pruned_bound=45750 pruned_infeasible=55242
[tsp-debug] progress: expanded=102000 created=148143 depth=21 bound=3141 best=3323 pruned_bound=46136 pruned_infeasible=55856
[tsp-debug] progress: expanded=103000 created=149553 depth=30 bound=3296 best=3323 pruned_bound=46549 pruned_infeasible=56446
[tsp-debug] progress: expanded=104000 created=151010 depth=18 bound=3181 best=3323 pruned_bound=47004 pruned_infeasible=56989
[tsp-debug] progress: expanded=105000 created=152484 depth=24 bound=3284 best=3323 pruned_bound=47477 pruned_infeasible=57515
[tsp-debug] progress: expanded=106000 created=154016 depth=20 bound=3203 best=3323 pruned_bound=48011 pruned_infeasible=57983
[tsp-debug] progress: expanded=107000 created=155527 depth=27 bound=3261 best=3323 pruned_bound=48521 pruned_infeasible=58472
[tsp-debug] progress: expanded=108000 created=157007 depth=16 bound=3124 best=3323 pruned_bound=49002 pruned_infeasible=58992
[tsp-debug] progress: expanded=109000 created=158498 depth=20 bound=3312 best=3323 pruned_bound=49493 pruned_infeasible=59501
[tsp-debug] progress: expanded=110000 created=159980 depth=23 bound=3315 best=3323 pruned_bound=49974 pruned_infeasible=60019
[tsp-debug] progress: expanded=111000 created=161471 depth=21 bound=3273 best=3323 pruned_bound=50463 pruned_infeasible=60528
[tsp-debug] progress: expanded=112000 created=162951 depth=19 bound=3183 best=3323 pruned_bound=50945 pruned_infeasible=61048
[tsp-debug] progress: expanded=113000 created=164404 depth=28 bound=3315 best=3323 pruned_bound=51399 pruned_infeasible=61595
[tsp-debug] progress: expanded=114000 created=165880 depth=37 bound=3293 best=3323 pruned_bound=51875 pruned_infeasible=62119
[tsp-debug] progress: expanded=115000 created=167365 depth=21 bound=3187 best=3323 pruned_bound=52360 pruned_infeasible=62634
[tsp-debug] progress: expanded=116000 created=168877 depth=24 bound=3232 best=3323 pruned_bound=52872 pruned_infeasible=63122
[tsp-debug] progress: expanded=117000 created=170368 depth=19 bound=3072 best=3323 pruned_bound=53365 pruned_infeasible=63631
[tsp-debug] progress: expanded=118000 created=171841 depth=30 bound=3285 best=3323 pruned_bound=53835 pruned_infeasible=64158
[tsp-debug] progress: expanded=119000 created=173331 depth=34 bound=3259 best=3323 pruned_bound=54326 pruned_infeasible=64668
[tsp-debug] progress: expanded=120000 created=174753 depth=29 bound=3217 best=3323 pruned_bound=54745 pruned_infeasible=65246
[tsp-debug] progress: expanded=121000 created=176250 depth=24 bound=3137 best=3323 pruned_bound=55245 pruned_infeasible=65749
[tsp-debug] progress: expanded=122000 created=177586 depth=12 bound=2855 best=3323 pruned_bound=55582 pruned_infeasible=66413
[tsp-debug] progress: expanded=123000 created=179013 depth=21 bound=3281 best=3323 pruned_bound=56006 pruned_infeasible=66986
[tsp-debug] progress: expanded=124000 created=180425 depth=23 bound=3180 best=3323 pruned_bound=56418 pruned_infeasible=67574
[tsp-debug] progress: expanded=125000 created=181916 depth=24 bound=3129 best=3323 pruned_bound=56909 pruned_infeasible=68083
[tsp-debug] progress: expanded=126000 created=183339 depth=26 bound=3295 best=3323 pruned_bound=57332 pruned_infeasible=68660
[tsp-debug] progress: expanded=127000 created=184694 depth=33 bound=3124 best=3323 pruned_bound=57689 pruned_infeasible=69305
[tsp-debug] progress: expanded=128000 created=186148 depth=24 bound=2954 best=3323 pruned_bound=58141 pruned_infeasible=69851
[tsp-debug] progress: expanded=129000 created=187587 depth=20 bound=3270 best=3323 pruned_bound=58581 pruned_infeasible=70412
[tsp-debug] progress: expanded=130000 created=189012 depth=22 bound=3218 best=3323 pruned_bound=59005 pruned_infeasible=70987
[tsp-debug] progress: expanded=131000 created=190464 depth=23 bound=3230 best=3323 pruned_bound=59459 pruned_infeasible=71535
[tsp-debug] progress: expanded=132000 created=191951 depth=16 bound=2899 best=3323 pruned_bound=59949 pruned_infeasible=72048
[tsp-debug] progress: expanded=133000 created=193439 depth=31 bound=3069 best=3323 pruned_bound=60436 pruned_infeasible=72560
[tsp-debug] progress: expanded=134000 created=194821 depth=23 bound=3197 best=3323 pruned_bound=60815 pruned_infeasible=73178
[tsp-debug] progress: expanded=135000 created=196269 depth=13 bound=2940 best=3323 pruned_bound=61265 pruned_infeasible=73730
[tsp-debug] progress: expanded=136000 created=197730 depth=31 bound=3176 best=3323 pruned_bound=61722 pruned_infeasible=74269
[tsp-debug] progress: expanded=137000 created=199187 depth=25 bound=3235 best=3323 pruned_bound=62179 pruned_infeasible=74812
[tsp-debug] progress: expanded=138000 created=200564 depth=22 bound=3127 best=3323 pruned_bound=62559 pruned_infeasible=75435
[tsp-debug] progress: expanded=139000 created=202029 depth=25 bound=3258 best=3323 pruned_bound=63024 pruned_infeasible=75970
[tsp-debug] progress: expanded=140000 created=203521 depth=30 bound=3280 best=3323 pruned_bound=63513 pruned_infeasible=76478
[tsp-debug] progress: expanded=141000 created=204962 depth=31 bound=3187 best=3323 pruned_bound=63956 pruned_infeasible=77037
[tsp-debug] progress: expanded=142000 created=206422 depth=21 bound=3236 best=3323 pruned_bound=64416 pruned_infeasible=77577
[tsp-debug] progress: expanded=143000 created=207907 depth=35 bound=3311 best=3323 pruned_bound=64900 pruned_infeasible=78092
[tsp-debug] progress: expanded=144000 created=209307 depth=39 bound=3263 best=3323 pruned_bound=65300 pruned_infeasible=78692
[tsp-debug] progress: expanded=145000 created=210799 depth=25 bound=3274 best=3323 pruned_bound=65792 pruned_infeasible=79200
[tsp-debug] progress: expanded=146000 created=212280 depth=21 bound=3152 best=3323 pruned_bound=66274 pruned_infeasible=79719
[tsp-debug] progress: expanded=147000 created=213794 depth=26 bound=3195 best=3323 pruned_bound=66789 pruned_infeasible=80205
[tsp-debug] progress: expanded=148000 created=215265 depth=22 bound=3229 best=3323 pruned_bound=67258 pruned_infeasible=80734
[tsp-debug] progress: expanded=149000 created=216779 depth=34 bound=3313 best=3323 pruned_bound=67774 pruned_infeasible=81220
[tsp-debug] progress: expanded=150000 created=218144 depth=22 bound=2908 best=3323 pruned_bound=68138 pruned_infeasible=81855
[tsp-debug] progress: expanded=151000 created=219551 depth=20 bound=3189 best=3323 pruned_bound=68546 pruned_infeasible=82448
[tsp-debug] progress: expanded=152000 created=221054 depth=26 bound=3197 best=3323 pruned_bound=69048 pruned_infeasible=82945
[tsp-debug] progress: expanded=153000 created=222521 depth=21 bound=3198 best=3323 pruned_bound=69515 pruned_infeasible=83478
[tsp-debug] progress: expanded=154000 created=224018 depth=24 bound=3235 best=3323 pruned_bound=70014 pruned_infeasible=83981
[tsp-debug] progress: expanded=155000 created=225484 depth=24 bound=3132 best=3323 pruned_bound=70479 pruned_infeasible=84515
[tsp-debug] progress: expanded=156000 created=226970 depth=25 bound=3207 best=3323 pruned_bound=70964 pruned_infeasible=85029
[tsp-debug] progress: expanded=157000 created=228452 depth=24 bound=3068 best=3323 pruned_bound=71447 pruned_infeasible=85547
[tsp-debug] progress: expanded=158000 created=229922 depth=21 bound=3080 best=3323 pruned_bound=71915 pruned_infeasible=86077
[tsp-debug] progress: expanded=159000 created=231308 depth=33 bound=3078 best=3323 pruned_bound=72301 pruned_infeasible=86691
[tsp-debug] progress: expanded=160000 created=232757 depth=20 bound=3205 best=3323 pruned_bound=72752 pruned_infeasible=87242
[tsp-debug] progress: expanded=161000 created=234269 depth=25 bound=3265 best=3323 pruned_bound=73265 pruned_infeasible=87730
[tsp-debug] progress: expanded=162000 created=235815 depth=28 bound=3290 best=3323 pruned_bound=73809 pruned_infeasible=88184
[tsp-debug] progress: expanded=163000 created=237330 depth=14 bound=2850 best=3323 pruned_bound=74326 pruned_infeasible=88669
[tsp-debug] progress: expanded=164000 created=238843 depth=31 bound=3262 best=3323 pruned_bound=74836 pruned_infeasible=89156
[tsp-debug] progress: expanded=165000 created=240369 depth=23 bound=3292 best=3323 pruned_bound=75364 pruned_infeasible=89630
[tsp-debug] progress: expanded=166000 created=241859 depth=25 bound=3291 best=3323 pruned_bound=75854 pruned_infeasible=90140
[tsp-debug] progress: expanded=167000 created=243369 depth=25 bound=3235 best=3323 pruned_bound=76363 pruned_infeasible=90630
[tsp-debug] progress: expanded=168000 created=244891 depth=28 bound=3193 best=3323 pruned_bound=76887 pruned_infeasible=91108
[tsp-debug] progress: expanded=169000 created=246426 depth=38 bound=3279 best=3323 pruned_bound=77422 pruned_infeasible=91573
[tsp-debug] progress: expanded=170000 created=247920 depth=27 bound=3175 best=3323 pruned_bound=77917 pruned_infeasible=92079
[tsp-debug] progress: expanded=171000 created=249431 depth=25 bound=3300 best=3323 pruned_bound=78425 pruned_infeasible=92568
[tsp-debug] progress: expanded=172000 created=250833 depth=16 bound=3249 best=3323 pruned_bound=78827 pruned_infeasible=93166
[tsp-debug] progress: expanded=173000 created=252271 depth=32 bound=3291 best=3323 pruned_bound=79267 pruned_infeasible=93728
[tsp-debug] progress: expanded=174000 created=253767 depth=20 bound=3182 best=3323 pruned_bound=79761 pruned_infeasible=94232
[tsp-debug] progress: expanded=175000 created=255284 depth=20 bound=3226 best=3323 pruned_bound=80278 pruned_infeasible=94715
[tsp-debug] progress: expanded=176000 created=256843 depth=31 bound=3300 best=3323 pruned_bound=80838 pruned_infeasible=95156
[tsp-debug] progress: expanded=177000 created=258406 depth=15 bound=3303 best=3323 pruned_bound=81401 pruned_infeasible=95593
[tsp-debug] progress: expanded=178000 created=259906 depth=12 bound=3235 best=3323 pruned_bound=81904 pruned_infeasible=96093
[tsp-debug] progress: expanded=179000 created=261424 depth=16 bound=3122 best=3323 pruned_bound=82420 pruned_infeasible=96575
[tsp-debug] progress: expanded=180000 created=262958 depth=16 bound=3115 best=3323 pruned_bound=82953 pruned_infeasible=97041
[tsp-debug] progress: expanded=181000 created=264428 depth=21 bound=3231 best=3323 pruned_bound=83425 pruned_infeasible=97571
[tsp-debug] progress: expanded=182000 created=265930 depth=20 bound=3224 best=3323 pruned_bound=83925 pruned_infeasible=98069
[tsp-debug] progress: expanded=183000 created=267452 depth=27 bound=3237 best=3323 pruned_bound=84447 pruned_infeasible=98547
[tsp-debug] progress: expanded=184000 created=268986 depth=28 bound=3232 best=3323 pruned_bound=84984 pruned_infeasible=99013
[tsp-debug] progress: expanded=185000 created=270482 depth=26 bound=3143 best=3323 pruned_bound=85480 pruned_infeasible=99517
[tsp-debug] progress: expanded=186000 created=271999 depth=35 bound=3251 best=3323 pruned_bound=85992 pruned_infeasible=100000
[tsp-debug] exact solve finished: feasible=yes cost=3323 expanded=186707 created=273087 pruned_bound=86380 pruned_infeasible=100328
```
</details>

## 2. `data/classic/tsplib/gr17.tsp` (n=17)

```
Problem: gr17
```

```
Dimension: 17
```

```
Method: exact
```

```
Root lower bound: 1501
```

```
Initial upper bound: 2085
```

```
Nodes created: 40989470
```

```
Nodes expanded: 30035452
```

```
Pruned by bound: 10954018
```

```
Pruned infeasible: 19081435
```

```
Optimal cost: 2085
```

```
Tour: 0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=gr17 dimension=17 method=exact
[tsp-debug] exact solve started: vertices=17
[tsp-debug] initial incumbent: cost=2085
[tsp-debug] root: lower_bound=1501 search=recursive-dfs strategy=smart
[tsp-debug] progress: expanded=1000 created=1374 depth=32 bound=2050 best=2085 pruned_bound=365 pruned_infeasible=625
[tsp-debug] progress: expanded=2000 created=2771 depth=29 bound=1910 best=2085 pruned_bound=764 pruned_infeasible=1228
[tsp-debug] progress: expanded=3000 created=4135 depth=38 bound=1973 best=2085 pruned_bound=1128 pruned_infeasible=1864
[tsp-debug] progress: expanded=4000 created=5507 depth=35 bound=2042 best=2085 pruned_bound=1500 pruned_infeasible=2492
[tsp-debug] progress: expanded=5000 created=6909 depth=34 bound=1932 best=2085 pruned_bound=1900 pruned_infeasible=3090
[tsp-debug] progress: expanded=6000 created=8277 depth=38 bound=2051 best=2085 pruned_bound=2268 pruned_infeasible=3722
[tsp-debug] progress: expanded=7000 created=9677 depth=51 bound=2077 best=2085 pruned_bound=2666 pruned_infeasible=4322
[tsp-debug] progress: expanded=8000 created=11075 depth=47 bound=2043 best=2085 pruned_bound=3065 pruned_infeasible=4924
[tsp-debug] progress: expanded=9000 created=12451 depth=36 bound=2017 best=2085 pruned_bound=3442 pruned_infeasible=5548
[tsp-debug] progress: expanded=10000 created=13892 depth=41 bound=2038 best=2085 pruned_bound=3886 pruned_infeasible=6107
[tsp-debug] progress: expanded=11000 created=15304 depth=48 bound=2064 best=2085 pruned_bound=4297 pruned_infeasible=6695
[tsp-debug] progress: expanded=12000 created=16596 depth=39 bound=2042 best=2085 pruned_bound=4587 pruned_infeasible=7403
[tsp-debug] progress: expanded=13000 created=17843 depth=34 bound=2038 best=2085 pruned_bound=4834 pruned_infeasible=8156
[tsp-debug] progress: expanded=14000 created=19096 depth=38 bound=2026 best=2085 pruned_bound=5086 pruned_infeasible=8903
[tsp-debug] progress: expanded=15000 created=20333 depth=35 bound=1966 best=2085 pruned_bound=5323 pruned_infeasible=9666
[tsp-debug] progress: expanded=16000 created=21647 depth=45 bound=1988 best=2085 pruned_bound=5638 pruned_infeasible=10352
[tsp-debug] progress: expanded=29952000 created=40870674 depth=45 bound=2045 best=2085 pruned_bound=10918671 pruned_infeasible=19033325
[tsp-debug] progress: expanded=29953000 created=40872078 depth=33 bound=1990 best=2085 pruned_bound=10919074 pruned_infeasible=19033921
[tsp-debug] progress: expanded=29954000 created=40873494 depth=36 bound=2084 best=2085 pruned_bound=10919488 pruned_infeasible=19034505
[tsp-debug] progress: expanded=29955000 created=40874940 depth=39 bound=2032 best=2085 pruned_bound=10919934 pruned_infeasible=19035059
[tsp-debug] progress: expanded=29956000 created=40876357 depth=32 bound=2072 best=2085 pruned_bound=10920352 pruned_infeasible=19035642
[tsp-debug] progress: expanded=29957000 created=40877741 depth=32 bound=1914 best=2085 pruned_bound=10920736 pruned_infeasible=19036258
[tsp-debug] progress: expanded=29958000 created=40879178 depth=39 bound=2076 best=2085 pruned_bound=10921173 pruned_infeasible=19036821
[tsp-debug] progress: expanded=29959000 created=40880534 depth=38 bound=2065 best=2085 pruned_bound=10921527 pruned_infeasible=19037465
[tsp-debug] progress: expanded=29960000 created=40881928 depth=42 bound=2073 best=2085 pruned_bound=10921920 pruned_infeasible=19038071
[tsp-debug] progress: expanded=29961000 created=40883372 depth=36 bound=2012 best=2085 pruned_bound=10922365 pruned_infeasible=19038627
[tsp-debug] progress: expanded=29962000 created=40884816 depth=38 bound=2028 best=2085 pruned_bound=10922810 pruned_infeasible=19039183
[tsp-debug] progress: expanded=29963000 created=40886231 depth=44 bound=1996 best=2085 pruned_bound=10923227 pruned_infeasible=19039768
[tsp-debug] progress: expanded=29964000 created=40887677 depth=37 bound=2034 best=2085 pruned_bound=10923673 pruned_infeasible=19040322
[tsp-debug] progress: expanded=29965000 created=40889087 depth=43 bound=2049 best=2085 pruned_bound=10924080 pruned_infeasible=19040912
[tsp-debug] progress: expanded=29966000 created=40890489 depth=36 bound=2010 best=2085 pruned_bound=10924484 pruned_infeasible=19041510
[tsp-debug] progress: expanded=29967000 created=40891932 depth=41 bound=2072 best=2085 pruned_bound=10924926 pruned_infeasible=19042067
[tsp-debug] progress: expanded=29968000 created=40893326 depth=47 bound=2080 best=2085 pruned_bound=10925320 pruned_infeasible=19042673
[tsp-debug] progress: expanded=29969000 created=40894750 depth=42 bound=1973 best=2085 pruned_bound=10925747 pruned_infeasible=19043249
[tsp-debug] progress: expanded=29970000 created=40896166 depth=43 bound=1994 best=2085 pruned_bound=10926159 pruned_infeasible=19043833
[tsp-debug] progress: expanded=29971000 created=40897631 depth=39 bound=2074 best=2085 pruned_bound=10926624 pruned_infeasible=19044368
[tsp-debug] progress: expanded=29972000 created=40899071 depth=36 bound=1999 best=2085 pruned_bound=10927067 pruned_infeasible=19044928
[tsp-debug] progress: expanded=29973000 created=40900509 depth=47 bound=2056 best=2085 pruned_bound=10927503 pruned_infeasible=19045490
[tsp-debug] progress: expanded=29974000 created=40901939 depth=42 bound=2061 best=2085 pruned_bound=10927933 pruned_infeasible=19046060
[tsp-debug] progress: expanded=29975000 created=40903397 depth=47 bound=2052 best=2085 pruned_bound=10928390 pruned_infeasible=19046602
[tsp-debug] progress: expanded=29976000 created=40904869 depth=42 bound=2036 best=2085 pruned_bound=10928864 pruned_infeasible=19047130
[tsp-debug] progress: expanded=29977000 created=40906355 depth=43 bound=2054 best=2085 pruned_bound=10929352 pruned_infeasible=19047644
[tsp-debug] progress: expanded=29978000 created=40907734 depth=36 bound=2047 best=2085 pruned_bound=10929729 pruned_infeasible=19048265
[tsp-debug] progress: expanded=29979000 created=40909158 depth=36 bound=2070 best=2085 pruned_bound=10930150 pruned_infeasible=19048841
[tsp-debug] progress: expanded=29980000 created=40910575 depth=32 bound=2012 best=2085 pruned_bound=10930571 pruned_infeasible=19049424
[tsp-debug] progress: expanded=29981000 created=40912032 depth=35 bound=2043 best=2085 pruned_bound=10931025 pruned_infeasible=19049967
[tsp-debug] progress: expanded=29982000 created=40913429 depth=35 bound=2055 best=2085 pruned_bound=10931422 pruned_infeasible=19050570
[tsp-debug] progress: expanded=29983000 created=40914867 depth=37 bound=2062 best=2085 pruned_bound=10931861 pruned_infeasible=19051132
[tsp-debug] progress: expanded=29984000 created=40916318 depth=33 bound=1964 best=2085 pruned_bound=10932311 pruned_infeasible=19051681
[tsp-debug] progress: expanded=29985000 created=40917688 depth=34 bound=1995 best=2085 pruned_bound=10932680 pruned_infeasible=19052311
[tsp-debug] progress: expanded=29986000 created=40919097 depth=30 bound=1935 best=2085 pruned_bound=10933091 pruned_infeasible=19052902
[tsp-debug] progress: expanded=29987000 created=40920482 depth=33 bound=1957 best=2085 pruned_bound=10933478 pruned_infeasible=19053517
[tsp-debug] progress: expanded=29988000 created=40921862 depth=33 bound=2045 best=2085 pruned_bound=10933855 pruned_infeasible=19054137
[tsp-debug] progress: expanded=29989000 created=40923232 depth=37 bound=2083 best=2085 pruned_bound=10934226 pruned_infeasible=19054767
[tsp-debug] progress: expanded=29990000 created=40924642 depth=38 bound=1996 best=2085 pruned_bound=10934637 pruned_infeasible=19055357
[tsp-debug] progress: expanded=29991000 created=40926060 depth=29 bound=2012 best=2085 pruned_bound=10935055 pruned_infeasible=19055939
[tsp-debug] progress: expanded=29992000 created=40927481 depth=41 bound=2070 best=2085 pruned_bound=10935474 pruned_infeasible=19056518
[tsp-debug] progress: expanded=29993000 created=40928923 depth=35 bound=2076 best=2085 pruned_bound=10935916 pruned_infeasible=19057076
[tsp-debug] progress: expanded=29994000 created=40930292 depth=42 bound=2074 best=2085 pruned_bound=10936286 pruned_infeasible=19057707
[tsp-debug] progress: expanded=29995000 created=40931699 depth=37 bound=2084 best=2085 pruned_bound=10936692 pruned_infeasible=19058300
[tsp-debug] progress: expanded=29996000 created=40933140 depth=39 bound=2084 best=2085 pruned_bound=10937135 pruned_infeasible=19058859
[tsp-debug] progress: expanded=29997000 created=40934575 depth=30 bound=1985 best=2085 pruned_bound=10937569 pruned_infeasible=19059424
[tsp-debug] progress: expanded=29998000 created=40935982 depth=36 bound=2021 best=2085 pruned_bound=10937977 pruned_infeasible=19060017
[tsp-debug] progress: expanded=29999000 created=40937453 depth=38 bound=2035 best=2085 pruned_bound=10938450 pruned_infeasible=19060546
[tsp-debug] progress: expanded=30000000 created=40938950 depth=30 bound=1983 best=2085 pruned_bound=10938947 pruned_infeasible=19061049
[tsp-debug] progress: expanded=30001000 created=40940430 depth=31 bound=1979 best=2085 pruned_bound=10939424 pruned_infeasible=19061569
[tsp-debug] progress: expanded=30002000 created=40941829 depth=36 bound=2061 best=2085 pruned_bound=10939823 pruned_infeasible=19062170
[tsp-debug] progress: expanded=30003000 created=40943253 depth=36 bound=2038 best=2085 pruned_bound=10940248 pruned_infeasible=19062746
[tsp-debug] progress: expanded=30004000 created=40944652 depth=43 bound=2078 best=2085 pruned_bound=10940646 pruned_infeasible=19063347
[tsp-debug] progress: expanded=30005000 created=40946076 depth=38 bound=2064 best=2085 pruned_bound=10941069 pruned_infeasible=19063923
[tsp-debug] progress: expanded=30006000 created=40947539 depth=23 bound=1849 best=2085 pruned_bound=10941537 pruned_infeasible=19064460
[tsp-debug] progress: expanded=30007000 created=40948891 depth=36 bound=1996 best=2085 pruned_bound=10941885 pruned_infeasible=19065108
[tsp-debug] progress: expanded=30008000 created=40950335 depth=42 bound=2063 best=2085 pruned_bound=10942331 pruned_infeasible=19065664
[tsp-debug] progress: expanded=30009000 created=40951790 depth=35 bound=2019 best=2085 pruned_bound=10942788 pruned_infeasible=19066209
[tsp-debug] progress: expanded=30010000 created=40953195 depth=40 bound=2067 best=2085 pruned_bound=10943188 pruned_infeasible=19066804
[tsp-debug] progress: expanded=30011000 created=40954608 depth=39 bound=2064 best=2085 pruned_bound=10943604 pruned_infeasible=19067391
[tsp-debug] progress: expanded=30012000 created=40956028 depth=35 bound=1917 best=2085 pruned_bound=10944023 pruned_infeasible=19067971
[tsp-debug] progress: expanded=30013000 created=40957488 depth=46 bound=2056 best=2085 pruned_bound=10944482 pruned_infeasible=19068511
[tsp-debug] progress: expanded=30014000 created=40958929 depth=32 bound=2047 best=2085 pruned_bound=10944926 pruned_infeasible=19069070
[tsp-debug] progress: expanded=30015000 created=40960306 depth=38 bound=2042 best=2085 pruned_bound=10945300 pruned_infeasible=19069693
[tsp-debug] progress: expanded=30016000 created=40961704 depth=48 bound=2071 best=2085 pruned_bound=10945700 pruned_infeasible=19070295
[tsp-debug] progress: expanded=30017000 created=40963128 depth=45 bound=2076 best=2085 pruned_bound=10946120 pruned_infeasible=19070871
[tsp-debug] progress: expanded=30018000 created=40964556 depth=49 bound=2079 best=2085 pruned_bound=10946548 pruned_infeasible=19071443
[tsp-debug] progress: expanded=30019000 created=40965994 depth=43 bound=2077 best=2085 pruned_bound=10946987 pruned_infeasible=19072005
[tsp-debug] progress: expanded=30020000 created=40967449 depth=35 bound=2026 best=2085 pruned_bound=10947446 pruned_infeasible=19072550
[tsp-debug] progress: expanded=30021000 created=40968949 depth=43 bound=2073 best=2085 pruned_bound=10947946 pruned_infeasible=19073050
[tsp-debug] progress: expanded=30022000 created=40970360 depth=44 bound=2060 best=2085 pruned_bound=10948354 pruned_infeasible=19073639
[tsp-debug] progress: expanded=30023000 created=40971783 depth=43 bound=2045 best=2085 pruned_bound=10948778 pruned_infeasible=19074216
[tsp-debug] progress: expanded=30024000 created=40973175 depth=47 bound=2033 best=2085 pruned_bound=10949169 pruned_infeasible=19074824
[tsp-debug] progress: expanded=30025000 created=40974576 depth=43 bound=2070 best=2085 pruned_bound=10949570 pruned_infeasible=19075423
[tsp-debug] progress: expanded=30026000 created=40976034 depth=34 bound=1997 best=2085 pruned_bound=10950031 pruned_infeasible=19075965
[tsp-debug] progress: expanded=30027000 created=40977449 depth=40 bound=2048 best=2085 pruned_bound=10950442 pruned_infeasible=19076550
[tsp-debug] progress: expanded=30028000 created=40978879 depth=36 bound=2073 best=2085 pruned_bound=10950875 pruned_infeasible=19077120
[tsp-debug] progress: expanded=30029000 created=40980245 depth=37 bound=2016 best=2085 pruned_bound=10951242 pruned_infeasible=19077754
[tsp-debug] progress: expanded=30030000 created=40981664 depth=47 bound=2005 best=2085 pruned_bound=10951660 pruned_infeasible=19078335
[tsp-debug] progress: expanded=30031000 created=40983146 depth=30 bound=1989 best=2085 pruned_bound=10952144 pruned_infeasible=19078853
[tsp-debug] progress: expanded=30032000 created=40984556 depth=33 bound=2034 best=2085 pruned_bound=10952554 pruned_infeasible=19079443
[tsp-debug] progress: expanded=30033000 created=40985972 depth=48 bound=2050 best=2085 pruned_bound=10952967 pruned_infeasible=19080027
[tsp-debug] progress: expanded=30034000 created=40987373 depth=43 bound=2076 best=2085 pruned_bound=10953369 pruned_infeasible=19080626
[tsp-debug] progress: expanded=30035000 created=40988811 depth=46 bound=2066 best=2085 pruned_bound=10953806 pruned_infeasible=19081188
[tsp-debug] exact solve finished: feasible=yes cost=2085 expanded=30035452 created=40989470 pruned_bound=10954018 pruned_infeasible=19081435
```
</details>

## 3. `data/classic/tsplib/ulysses16.tsp` (n=16)

**Status:** TIMEOUT (>30m)

## 4. `examples/five-city.txt` (n=5)

```
Problem: matrix
```

```
Dimension: 5
```

```
Method: exact
```

```
Root lower bound: 21
```

```
Initial upper bound: 26
```

```
Nodes created: 30
```

```
Nodes expanded: 18
```

```
Pruned by bound: 12
```

```
Pruned infeasible: 7
```

```
Optimal cost: 26
```

```
Tour: 0 -> 1 -> 3 -> 2 -> 4 -> 0
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=26
[tsp-debug] root: lower_bound=21 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=26 expanded=18 created=30 pruned_bound=12 pruned_infeasible=7
```
</details>

## 5. `examples/tsplib/five-node-euc.tsp` (n=5)

```
Problem: five-node-euc
```

```
Dimension: 5
```

```
Method: exact
```

```
Root lower bound: 6
```

```
Initial upper bound: 8
```

```
Nodes created: 16
```

```
Nodes expanded: 10
```

```
Pruned by bound: 6
```

```
Pruned infeasible: 5
```

```
Optimal cost: 8
```

```
Tour: 0 -> 4 -> 1 -> 2 -> 3 -> 0
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=five-node-euc dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=8
[tsp-debug] root: lower_bound=6 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=8 expanded=10 created=16 pruned_bound=6 pruned_infeasible=5
```
</details>

## 6. `examples/tsplib/five-node-explicit.tsp` (n=5)

```
Problem: five-node-explicit
```

```
Dimension: 5
```

```
Method: exact
```

```
Root lower bound: 21
```

```
Initial upper bound: 26
```

```
Nodes created: 30
```

```
Nodes expanded: 18
```

```
Pruned by bound: 12
```

```
Pruned infeasible: 7
```

```
Optimal cost: 26
```

```
Tour: 0 -> 1 -> 3 -> 2 -> 4 -> 0
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=five-node-explicit dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=26
[tsp-debug] root: lower_bound=21 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=26 expanded=18 created=30 pruned_bound=12 pruned_infeasible=7
```
</details>

## 7. `examples/converted/five-node-euc.txt` (n=5)

```
Problem: matrix
```

```
Dimension: 5
```

```
Method: exact
```

```
Root lower bound: 6
```

```
Initial upper bound: 8
```

```
Nodes created: 16
```

```
Nodes expanded: 10
```

```
Pruned by bound: 6
```

```
Pruned infeasible: 5
```

```
Optimal cost: 8
```

```
Tour: 0 -> 4 -> 1 -> 2 -> 3 -> 0
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=8
[tsp-debug] root: lower_bound=6 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=8 expanded=10 created=16 pruned_bound=6 pruned_infeasible=5
```
</details>

## 8. `examples/converted/five-node-explicit.txt` (n=5)

```
Problem: matrix
```

```
Dimension: 5
```

```
Method: exact
```

```
Root lower bound: 21
```

```
Initial upper bound: 26
```

```
Nodes created: 30
```

```
Nodes expanded: 18
```

```
Pruned by bound: 12
```

```
Pruned infeasible: 7
```

```
Optimal cost: 26
```

```
Tour: 0 -> 1 -> 3 -> 2 -> 4 -> 0
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=26
[tsp-debug] root: lower_bound=21 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=26 expanded=18 created=30 pruned_bound=12 pruned_infeasible=7
```
</details>

## 9. `examples/random/complete/rnd-01-complete-n4.txt` (n=4)

```
Problem: matrix
```

```
Dimension: 4
```

```
Method: exact
```

```
Root lower bound: 36
```

```
Initial upper bound: 45
```

```
Nodes created: 6
```

```
Nodes expanded: 3
```

```
Pruned by bound: 3
```

```
Pruned infeasible: 1
```

```
Optimal cost: 45
```

```
Tour: 0 -> 3 -> 2 -> 1 -> 0
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=4 method=exact
[tsp-debug] exact solve started: vertices=4
[tsp-debug] initial incumbent: cost=45
[tsp-debug] root: lower_bound=36 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=45 expanded=3 created=6 pruned_bound=3 pruned_infeasible=1
```
</details>

## 10. `examples/random/complete/rnd-02-complete-n5.txt` (n=5)

```
Problem: matrix
```

```
Dimension: 5
```

```
Method: exact
```

```
Root lower bound: 127
```

```
Initial upper bound: 136
```

```
Nodes created: 11
```

```
Nodes expanded: 6
```

```
Pruned by bound: 5
```

```
Pruned infeasible: 2
```

```
Optimal cost: 136
```

```
Tour: 0 -> 2 -> 3 -> 4 -> 1 -> 0
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=136
[tsp-debug] root: lower_bound=127 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=136 expanded=6 created=11 pruned_bound=5 pruned_infeasible=2
```
</details>

## 11. `examples/random/complete/rnd-03-complete-n4.txt` (n=4)

```
Problem: matrix
```

```
Dimension: 4
```

```
Method: exact
```

```
Root lower bound: 102
```

```
Initial upper bound: 102
```

```
Nodes created: 1
```

```
Nodes expanded: 0
```

```
Pruned by bound: 1
```

```
Pruned infeasible: 0
```

```
Optimal cost: 102
```

```
Tour: 0 -> 1 -> 3 -> 2 -> 0
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=4 method=exact
[tsp-debug] exact solve started: vertices=4
[tsp-debug] initial incumbent: cost=102
[tsp-debug] root: lower_bound=102 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=102 expanded=0 created=1 pruned_bound=1 pruned_infeasible=0
```
</details>

## 12. `examples/random/complete/rnd-04-complete-n5.txt` (n=5)

```
Problem: matrix
```

```
Dimension: 5
```

```
Method: exact
```

```
Root lower bound: 79
```

```
Initial upper bound: 88
```

```
Nodes created: 11
```

```
Nodes expanded: 6
```

```
Pruned by bound: 5
```

```
Pruned infeasible: 2
```

```
Optimal cost: 88
```

```
Tour: 0 -> 3 -> 2 -> 4 -> 1 -> 0
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=88
[tsp-debug] root: lower_bound=79 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=88 expanded=6 created=11 pruned_bound=5 pruned_infeasible=2
```
</details>

## 13. `examples/random/complete/rnd-05-complete-n7.txt` (n=7)

```
Problem: matrix
```

```
Dimension: 7
```

```
Method: exact
```

```
Root lower bound: 49
```

```
Initial upper bound: 74
```

```
Nodes created: 42
```

```
Nodes expanded: 25
```

```
Pruned by bound: 17
```

```
Pruned infeasible: 9
```

```
Optimal cost: 74
```

```
Tour: 0 -> 2 -> 4 -> 1 -> 5 -> 6 -> 3 -> 0
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=74
[tsp-debug] root: lower_bound=49 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=74 expanded=25 created=42 pruned_bound=17 pruned_infeasible=9
```
</details>

## 14. `examples/random/complete/rnd-06-complete-n5.txt` (n=5)

```
Problem: matrix
```

```
Dimension: 5
```

```
Method: exact
```

```
Root lower bound: 43
```

```
Initial upper bound: 51
```

```
Nodes created: 11
```

```
Nodes expanded: 6
```

```
Pruned by bound: 5
```

```
Pruned infeasible: 2
```

```
Optimal cost: 51
```

```
Tour: 0 -> 3 -> 2 -> 1 -> 4 -> 0
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=51
[tsp-debug] root: lower_bound=43 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=51 expanded=6 created=11 pruned_bound=5 pruned_infeasible=2
```
</details>

## 15. `examples/random/complete/rnd-07-complete-n8.txt` (n=8)

```
Problem: matrix
```

```
Dimension: 8
```

```
Method: exact
```

```
Root lower bound: 46
```

```
Initial upper bound: 64
```

```
Nodes created: 41
```

```
Nodes expanded: 25
```

```
Pruned by bound: 16
```

```
Pruned infeasible: 10
```

```
Optimal cost: 64
```

```
Tour: 2 -> 3 -> 4 -> 7 -> 0 -> 5 -> 6 -> 1 -> 2
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=8 method=exact
[tsp-debug] exact solve started: vertices=8
[tsp-debug] initial incumbent: cost=64
[tsp-debug] root: lower_bound=46 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=64 expanded=25 created=41 pruned_bound=16 pruned_infeasible=10
```
</details>

## 16. `examples/random/complete/rnd-08-complete-n7.txt` (n=7)

```
Problem: matrix
```

```
Dimension: 7
```

```
Method: exact
```

```
Root lower bound: 50
```

```
Initial upper bound: 73
```

```
Nodes created: 34
```

```
Nodes expanded: 20
```

```
Pruned by bound: 14
```

```
Pruned infeasible: 7
```

```
Optimal cost: 73
```

```
Tour: 0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=73
[tsp-debug] root: lower_bound=50 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=73 expanded=20 created=34 pruned_bound=14 pruned_infeasible=7
```
</details>

## 17. `examples/random/complete/rnd-09-complete-n8.txt` (n=8)

```
Problem: matrix
```

```
Dimension: 8
```

```
Method: exact
```

```
Root lower bound: 40
```

```
Initial upper bound: 93
```

```
Nodes created: 173
```

```
Nodes expanded: 111
```

```
Pruned by bound: 62
```

```
Pruned infeasible: 50
```

```
Optimal cost: 93
```

```
Tour: 1 -> 3 -> 2 -> 7 -> 0 -> 5 -> 4 -> 6 -> 1
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=8 method=exact
[tsp-debug] exact solve started: vertices=8
[tsp-debug] initial incumbent: cost=93
[tsp-debug] root: lower_bound=40 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=93 expanded=111 created=173 pruned_bound=62 pruned_infeasible=50
```
</details>

## 18. `examples/random/complete/rnd-10-complete-n7.txt` (n=7)

```
Problem: matrix
```

```
Dimension: 7
```

```
Method: exact
```

```
Root lower bound: 45
```

```
Initial upper bound: 49
```

```
Nodes created: 22
```

```
Nodes expanded: 13
```

```
Pruned by bound: 9
```

```
Pruned infeasible: 5
```

```
Optimal cost: 49
```

```
Tour: 0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 0
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=49
[tsp-debug] root: lower_bound=45 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=49 expanded=13 created=22 pruned_bound=9 pruned_infeasible=5
```
</details>

## 19. `examples/random/sparse/rnd-01-sparse-n5.txt` (n=5)

```
Problem: matrix
```

```
Dimension: 5
```

```
Method: exact
```

```
Root lower bound: 102
```

```
Initial upper bound: 110
```

```
Nodes created: 6
```

```
Nodes expanded: 3
```

```
Pruned by bound: 3
```

```
Pruned infeasible: 1
```

```
Optimal cost: 110
```

```
Tour: 3 -> 4 -> 2 -> 1 -> 0 -> 3
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=110
[tsp-debug] root: lower_bound=102 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=110 expanded=3 created=6 pruned_bound=3 pruned_infeasible=1
```
</details>

## 20. `examples/random/sparse/rnd-02-sparse-n7.txt` (n=7)

```
Problem: matrix
```

```
Dimension: 7
```

```
Method: exact
```

```
Root lower bound: 161
```

```
Initial upper bound: 196
```

```
Nodes created: 32
```

```
Nodes expanded: 22
```

```
Pruned by bound: 10
```

```
Pruned infeasible: 13
```

```
Optimal cost: 196
```

```
Tour: 1 -> 5 -> 3 -> 0 -> 4 -> 2 -> 6 -> 1
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=196
[tsp-debug] root: lower_bound=161 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=196 expanded=22 created=32 pruned_bound=10 pruned_infeasible=13
```
</details>

## 21. `examples/random/sparse/rnd-03-sparse-n7.txt` (n=7)

```
Problem: matrix
```

```
Dimension: 7
```

```
Method: exact
```

```
Root lower bound: 78
```

```
Initial upper bound: 86
```

```
Nodes created: 11
```

```
Nodes expanded: 7
```

```
Pruned by bound: 4
```

```
Pruned infeasible: 4
```

```
Optimal cost: 86
```

```
Tour: 1 -> 6 -> 3 -> 4 -> 5 -> 2 -> 0 -> 1
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=86
[tsp-debug] root: lower_bound=78 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=86 expanded=7 created=11 pruned_bound=4 pruned_infeasible=4
```
</details>

## 22. `examples/random/sparse/rnd-04-sparse-n6.txt` (n=6)

```
Problem: matrix
```

```
Dimension: 6
```

```
Method: exact
```

```
Root lower bound: 115
```

```
Initial upper bound: 115
```

```
Nodes created: 1
```

```
Nodes expanded: 0
```

```
Pruned by bound: 1
```

```
Pruned infeasible: 0
```

```
Optimal cost: 115
```

```
Tour: 0 -> 4 -> 3 -> 5 -> 1 -> 2 -> 0
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=6 method=exact
[tsp-debug] exact solve started: vertices=6
[tsp-debug] initial incumbent: cost=115
[tsp-debug] root: lower_bound=115 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=115 expanded=0 created=1 pruned_bound=1 pruned_infeasible=0
```
</details>

## 23. `examples/random/sparse/rnd-05-sparse-n6.txt` (n=6)

```
Problem: matrix
```

```
Dimension: 6
```

```
Method: exact
```

```
Root lower bound: 53
```

```
Initial upper bound: 100
```

```
Nodes created: 49
```

```
Nodes expanded: 36
```

```
Pruned by bound: 13
```

```
Pruned infeasible: 24
```

```
Optimal cost: 100
```

```
Tour: 0 -> 5 -> 2 -> 1 -> 4 -> 3 -> 0
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=6 method=exact
[tsp-debug] exact solve started: vertices=6
[tsp-debug] initial incumbent: cost=100
[tsp-debug] root: lower_bound=53 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=100 expanded=36 created=49 pruned_bound=13 pruned_infeasible=24
```
</details>

## 24. `examples/random/sparse/rnd-06-sparse-n7.txt` (n=7)

```
Problem: matrix
```

```
Dimension: 7
```

```
Method: exact
```

```
Root lower bound: 169
```

```
Initial upper bound: 199
```

```
Nodes created: 48
```

```
Nodes expanded: 38
```

```
Pruned by bound: 10
```

```
Pruned infeasible: 29
```

```
Optimal cost: 199
```

```
Tour: 4 -> 1 -> 5 -> 3 -> 2 -> 6 -> 0 -> 4
```

<details>
<summary>Debug output (stderr)</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=199
[tsp-debug] root: lower_bound=169 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=199 expanded=38 created=48 pruned_bound=10 pruned_infeasible=29
```
</details>

---

## Summary

| Result | Count |
|--------|-------|
| Optimal/Has Tour | 23 |
| Failed/Timeout | 1 |
| **Total** | 24 |

### Failures

- `data/classic/tsplib/ulysses16.tsp: TIMEOUT`
