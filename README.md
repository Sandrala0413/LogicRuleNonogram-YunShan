# Logic Rule Nonogram

### Logic Rule 執行順序

1. `RLmost_init()`
2. `RLmost()`
3. `leftmost()`
4. `rightmost()`

### RLmost

- Total > 25 - start (Total = All clues with one space)
- Start + 1 < Total (Total = 0 ~ Current Clue)
- 線索長度不合
- 不是對應到同個線索

### Leftmost & Rightmost

- 已經到最後一格了，但是還沒有填完所有的 clue

### 檔案結構

專案主要檔案與資料夾結構如下：

.
├─ board_diff.py
├─ compare.py
├─ incomplete.txt
├─ input.txt
├─ intel-oneapi-base-toolkit-2025.2.1.44_offline.sh
├─ log.txt
├─ Makefile
├─ output.txt
├─ README.md
├─ TCGA2019.txt
├─ TCGA2020.txt
├─ TCGA2021.txt
├─ TCGA2024.txt
├─ tree.txt
├─ bin
│  ├─ nonogram
│  └─ output.txt
├─ dep
│  ├─ board.d
│  ├─ fullyprobe.d
│  ├─ Hash.d
│  ├─ help.d
│  ├─ linesolve.d
│  ├─ mirror.d
│  ├─ options.d
│  ├─ probsolver.d
│  ├─ scanner.d
│  └─ set.d
├─ include
│  ├─ board.h
│  ├─ cdef.h
│  ├─ fullyprobe.h
│  ├─ Hash.h
│  ├─ help.h
│  ├─ linesolve.h
│  ├─ mirror.h
│  ├─ options.h
│  ├─ probsolver.h
│  ├─ scanner.h
│  └─ set.h
├─ logs
├─ obj
├─ src
│  ├─ board.cpp
│  ├─ fullyprobe.cpp
│  ├─ Hash.cpp
│  ├─ help.cpp
│  ├─ linesolve.cpp
│  ├─ main.cpp
│  ├─ mirror.cpp
│  ├─ options.cpp
│  ├─ probsolver.cpp
│  ├─ scanner.cpp
│  └─ set.cpp
└─ txt
	├─ 2024-tcga.txt
	├─ 20240721.txt
	├─ 20240722.txt
	└─ check.txt

### 執行指令範例

```cpp
./nonogram.exe -S 1 -E 300 -I TCGA2021.txt -O output.txt -M 4 --yes-log --simple-stdout
```