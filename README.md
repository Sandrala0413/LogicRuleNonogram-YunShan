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

## 12/24

### Test

- Segamentation fault: $484
- Cannot solve (超過十分鐘):
  - $526、$543、$547、$548、$549、$552、$547、
  - $602、$606、$609、$617、$621、$622、$623、$630、$649、$654、$655
- 同盤面丟進去，看 fix 跟 logic 的 conflict 的規則是不是都等效。
- 尋找 logic 的 conflict 規則。

## 12/31

- 抓一些沒有 CONFLICT 的盤面，看看是不是也可以判斷出來。
- 確認沒有 CONFLICT 的盤面是否也被判斷為 CONFLICT。
- 找一題解題時間花很多的，去 trace 他每個階段都解了多少點。
- 第一次 Propagate 填幾個點，紀錄 FP 每個點判斷 CONFLICT 的情況。
- 紀錄解題進入 DFS 幾次。

### TODO

- 檢查是否是資訊錯誤。[o]
  - 線索記錄錯誤。[o]
  - 極左極右記錄錯誤。[o]

### Test

- 產出 incomplete 的盤面去測試，解果顯示全部都被判斷為 conflict。(已解決)

### Progress

- 已能判斷出 incomplete 跟 conflict 的盤面。

## 1/15

- 統計 1000 題各題的時間

## 1/20

- 每一題都只跑到 propagate，比較填點數跟時間。

## 2/5

- 跑文妍學姊的版本看是否跟論文數據一致
- DFS 測試猜點都用同一種策略

## 2/10

- 解完整題，有沒有比較快 (家駿學長 > 我)
- 只比到 propagate，有沒有比較快 ()
- 比 Conflict，誰可以判斷比較多 (一樣多)
- 統整 Conflict 的方法
- 統計 find hash 找不找得到的計數
- 把 4 個方法的所有實驗數據整理出來 (整題、Propagate、Conflict)
- 一行一次來講，為什麼邏輯規則算得比原本的 fix paint 來得久 (因為開始跟結束的判定位置不同)

## 2/18

- 研究家駿的 line solve 為什麼比一般實作更快
- 邏輯規則實作的優化
- 結案報告可以寫的內容
  - 邏輯規則可以解完整題
  - 只比到 Propagate 比學姊快 (數據證實)
  - 為什麼比學長慢那麼多 (原因)

## 結案
### 家駿學長實作加速

- 快取機制，double hashing。
- 只針對有變化的行或列進行 line solve。
- 填點策略選擇 mul。
- 動態選擇填 0 或填 1。
- line solve 所有 function 簡化成一個 function。
- 使用 bitwise。
- Board 的存取從二維陣列改為一維陣列。

### 邏輯規則實作的優化

- DFS。
- Cache。

## 結案報告想法

- 可以只依靠邏輯規則解完整題
  - 加入 Conflict Rule、DFS
- 邏輯規則實作
  - 改用 bitwise
  - Board 儲存方式(二維陣列 -> 一維陣列)
  - 未實作的方法加速方法
    - Cache (Double hash)
    - 可能可以實作更多邏輯規則 (?)

## 3/11
### 發展一個用 fix paint 判斷 conflict 的邏輯規則版本

- 我的版本跟那個版本比較速度

Propagate Time
1~500題	Line solve	Logic solve	學姊Logic solve
Tcga2020	0.0180 sec	0.1430 sec	0.1260 sec
Tcga2021	0.0140 sec	0.1410 sec	0.1400 sec
Tcga2024	0.0130 sec	0.1260 sec	0.1320 sec
