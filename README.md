# Logic Rule Nonogram

## Logic Rule 執行順序

1. `RLmost_init()`
2. `RLmost()`
3. `leftmost()`
4. `rightmost()`

## CONFLICT RULE

### RLmost

- Total > 25 - start (Total = All clues with one space)
- Start + 1 < Total (Total = 0 ~ Current Clue)
- 線索長度不合
- 不是對應到同個線索

### Leftmost & Rightmost

- 已經到最後一格了，但是還沒有填完所有的 clue

### 檔案結構

├─include
│      board.h
│      cdef.h
│      fullyprobe.h
│      Hash.h
│      help.h
│      linesolve.h
│      mirror.h
│      options.h
│      probsolver.h
│      scanner.h
│      set.h
│      
├─logs
├─obj
│      board.o
│      fullyprobe.o
│      Hash.o
│      help.o
│      linesolve.o
│      main.o
│      mirror.o
│      options.o
│      probsolver.o
│      scanner.o
│      set.o
│      
├─src
│      board.cpp
│      fullyprobe.cpp
│      Hash.cpp
│      help.cpp
│      linesolve.cpp
│      main.cpp
│      mirror.cpp
│      options.cpp
│      probsolver.cpp
│      scanner.cpp
│      set.cpp

- main
- probsolver
- 

## Idea
https://www.reddit.com/r/puzzles/comments/eawno1/what_strategies_can_i_use_to_start_solving/?tl=zh-hant
1. double hash
2. 或許可以從線索總和最長的line開始解
3. 邊界推理法
3. 把memcpy改成交換指標或使用 std::array 搭配 std::copy
4. __SET, __GET 以 constexpr inline 函式取代
5. 使用 __builtin_popcountll, __builtin_ctzll，比手動 while-loop 快
![alt text](image-1.png)
![alt text](image.png)


09/18
- 把建醫(邏輯優化)、家駿(組合優化)、信服(邏輯未優化)學長用profiler測試差在哪
- 把建醫學長的邏輯propagate加上家駿學長的fp + backtracking
- 把建一、家駿學長的程式跑first propagate，比較時間