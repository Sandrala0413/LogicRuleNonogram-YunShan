#include "linesolve.h"
#include <fstream>

#include <stdlib.h>

#include <cstdio>
#include <queue>

// 0x1 = 01 = blank(ZERO)
// 0x2 = 10 = fill(ONE)
// 0x3 = 11 = UNKNOWN
const uint64_t value1[28] = {
    0x1ULL,     // 0 = 01 = 一個空格
    0x9ULL,     // 10 = 1001 = 一個黑格+一個空格
    0x29ULL,    // 110 = 101001 = 兩個黑格+一個空格
    0xA9ULL,    // 1110 
    0x2A9ULL,   // 11110
    0xAA9ULL,   // 111110
    0x2AA9ULL,  // 1111110
    0xAAA9ULL,  // 11111110
    0x2AAA9ULL, // 111111110
    0xAAAA9ULL,
    0x2AAAA9ULL,
    0xAAAAA9ULL,
    0x2AAAAA9ULL,
    0xAAAAAA9ULL,
    0x2AAAAAA9ULL,
    0xAAAAAAA9ULL,
    0x2AAAAAAA9ULL,
    0xAAAAAAAA9ULL,
    0x2AAAAAAAA9ULL,
    0xAAAAAAAAA9ULL,
    0x2AAAAAAAAA9ULL,
    0xAAAAAAAAAA9ULL,
    0x2AAAAAAAAAA9ULL,
    0xAAAAAAAAAAA9ULL,
    0x2AAAAAAAAAAA9ULL,
    0xAAAAAAAAAAAA9ULL,
    0x2AAAAAAAAAAAA9ULL, // 111111111111111111111111110
    0xAAAAAAAAAAAAA9ULL, // 1111111111111111111111111110
};

LineSolve::LineSolve() {
	init();
	init_change();
}

LineSolve::LineSolve(int* d) {
	init();
	load(d);
	init_change();
}

void LineSolve::init() {}

//d[0] = #clue of line 1
//d[1]~d[13] = clues of line 1
void LineSolve::load(int* d) {
	MEMSET_ZERO(low_bound);

	for (int i = 0; i < 50; ++i) {
		int sum = 0;

		clue[i].count = d[i * 14];

		for (int j = 1; j <= d[i * 14]; ++j) {
			clue[i].num[j - 1] = d[i * 14 + j];	//各個線索存進每行
			sum += d[i * 14 + j] + 1;		
			low_bound[i][j] = sum - 1;	//每個線索最早可能結束位置
		}

		genHash(clue[i]);
	}
}

int propagate(LineSolve& ls, Board& board) {
	uint64_t chkline = 0;	//紀錄有哪些行/列有改變
	for (int i = 0; i < 50; ++i) {
		if (board.oldData[i] != board.data[i]) chkline |= 1LL << i;
	}
	uint64_t nextchk = 0LL;
	int lineNum = 0;

	while (1) {
		if (chkline == 0) {
			if (nextchk == 0) break;
			chkline = nextchk;
			nextchk = 0LL;
		}

		lineNum = __builtin_ffsll(chkline) - 1;		//取得從右數起第一個1的位置
		// printf("lineNum = %d\n", lineNum);
		chkline &= chkline - 1;		//清除剛剛處理的那個bit

		uint64_t line = getLine(board, lineNum)
		                << 4;     // 3333333333333333333333300
		__SET(line, 1, BIT_ZERO); // 3333333333333333333333310 把第2格變成空白

		uint64_t newLine = 0LL;

		// 從hash table找是否已經算過符合這個線索的最佳推理值
		if (!findHash(ls.clue[lineNum], line, newLine)) 
		{
			newLine = 0LL;

			const int fixAns =
			    fixBU(ls, lineNum, line, ls.clue[lineNum].count, newLine);
			if (unlikely(LS_NO == fixAns)) {	//LS_NO = 與線索矛盾
				return CONFLICT;
			}

			insertHash(ls.clue[lineNum], line, newLine);
		}

		newLine >>= 4;
		line >>= 4;

		if (line != newLine) {
			board.data[lineNum] = newLine;

			uint64_t p = line ^ newLine;	//紀錄有變化的格子
			int x = 1;

			while (x = __builtin_ffsll(p), x-- != 0) {	//取右邊第一個bit
				//x = 偶數 = 黑格；x = 奇數 = 白格
				uint64_t bit = (x & 0x1) == 0 ? BIT_ONE : BIT_ZERO;	
				p &= p - 1;	//去除最低位1
				x >>= 1;  	//計算第幾格(一格2 bit)

				//找出交叉的line作為下次檢查的line
				if (lineNum < 25) {
					nextchk |= 0x1LL << (x + 25);
					// printf("%d", __GET(bit,0));
					__SET(board.data[x + 25], lineNum, bit);
				} else {
					nextchk |= 0x1LL << x;
					__SET(board.data[x], (lineNum - 25), bit);
				}
			}
		}
	}

	memcpy(board.oldData, board.data, sizeof(board.oldData));

	if (unlikely(getSize(board) != 625)) return INCOMP;

	ls.solvedBoard = board;

	return SOLVED;
}

//bottom up
int fixBU(LineSolve& ls, int lineNum, const uint64_t& line, int j,
          uint64_t& newLine) {
	const int i = 26;
	const int maxShift = 26 - ls.low_bound[lineNum][ls.clue[lineNum].count - 1];
	uint64_t dpTable[14][27] = {};

	uint8_t data[14] = {};
	int low_bound[14] = {};

	memcpy(data + 1, &ls.clue[lineNum].num, sizeof(data[0]) * 13);
	memcpy(low_bound, &ls.low_bound[lineNum], sizeof(low_bound));

	dpTable[0][0] = BIT_ONE;

	for (int jp = 0; jp <= j; ++jp) {
		const int dj = data[jp];
		int ip = low_bound[jp] + 1;

		uint64_t val0 = (uint64_t)BIT_ZERO << (ip << 1);
		int length = ip - dj;
		uint64_t val1 = value1[dj] << (length << 1);
		length--;

		const int ipp = ip + maxShift;

		for (; ip < ipp; ++ip, val0 <<= 2, val1 <<= 2, ++length) {
			if (likely((line & val0) && dpTable[jp][ip - 1]))
				dpTable[jp][ip] |= val0 | dpTable[jp][ip - 1];

			if (unlikely(jp != 0 && line == (line | val1) &&
			             dpTable[jp - 1][length]))
				dpTable[jp][ip] |= val1 | dpTable[jp - 1][length];
		}
	}

	newLine = dpTable[j][i];
	return dpTable[j][i] == 0 ? LS_NO : LS_YES;
}

// Logic Rule
int RLmost_init(LineSolve& ls, Board& board) {
	for (int lineNum = 0; lineNum < 50; ++lineNum) {
		uint64_t line = getLine(board, lineNum);
		uint64_t newLine = ((0x1LL << 50) - 0x1LL);	
		int j = ls.clue[lineNum].count;

		int shift = 0;
		int totalClue = 0;
		for (int i = 0; i < j; i++) {
			totalClue += ls.clue[lineNum].num[i];
		}
		totalClue = totalClue + j - 1;
		shift = 25 - totalClue;
		int point = shift;
		for (int i = 0; i < j; i++) {
			if (shift < ls.clue[lineNum].num[i]) {
				int paint = ls.clue[lineNum].num[i] - shift;	//一定交集的黑格數
				for (int p = point; p < paint + point; p++) {
					if (__GET(line, p) == BIT_ZERO) {
						return CONFLICT;
					} else
						__SET(line, p, BIT_ONE);
				}
			}
			point += ls.clue[lineNum].num[i];
			point++;
		}
		//更新對應的行/列
		for (int i = 0; i < 25; i++) {
			if (__GET(line, i) != __GET(board.data[lineNum], i)) {
				if (lineNum < 25) {
					__SET(board.data[i + 25], lineNum, __GET(line, i));
				} else {
					__SET(board.data[i], (lineNum - 25), __GET(line, i));
				}
			}
		}
		board.data[lineNum] = line;
	}
	memcpy(board.oldData, board.data, sizeof(board.oldData));
	// printf("RLmost_init pixel: %d\n", debugBoard(board));
	if (unlikely(getSize(board) != 625)) return INCOMP;
	ls.solvedBoard = board;
	return SOLVED;
}

int logicSolve(LineSolve& ls, Board& board) {
	int state = INCOMP;
	ls.init_change();
	while (state == INCOMP) {
		memcpy(board.oldData, board.data, sizeof(board.oldData));
		state = logicRule(ls, board);
		ls.linesolvetime++;
	}

	if (state == CONFLICT) return CONFLICT;
	memcpy(board.oldData, board.data, sizeof(board.oldData));
	if (unlikely(getSize(board) != 625)) return INCOMP;
	ls.solvedBoard = board;
	return SOLVED;
}

int left[25], right[25];

//找出每個線索可能的最左和最右位置
int RLmost(LineSolve& ls, int lineNum, const uint64_t& line) {
	//initialize unknown
	std::fill(left, left + 25, 2);
	std::fill(right, right + 25, 2);

	int j = ls.clue[lineNum].count;	//這行的線索數量
	
	int finish = 0, start = 0, k, currentClue = 0;	
	bool error;
	while (finish != SOLVED) {
		//計算從當前線索到最後一個線索需要的總長度
		int total = 0;
		for (int i = currentClue; i < j; i++) {
			total += ls.clue[lineNum].num[i];
		}
		total += j - currentClue - 1;	//加上間隔的空格

		// CONFLICT RULE
		// 從當前 index 開始放置線索會超出範圍時，代表 CONFLICT
		if (25 - start < total) {
			return CONFLICT;
		}

		int state = leftmost(ls, lineNum, line, start, currentClue);
		if (state == CONFLICT) return CONFLICT;
		finish = SOLVED;

		//檢查已填的黑格是否合法
		for (int i = 0; i < 25; i++) {
			if (__GET(line, i) == BIT_ONE) {
				//第一種：黑格在兩個線索之間
				for (k = j - 1; k > 0; k--) {
					//如果黑格在第k個線索的開頭和第k-1個線索的結尾之間->CONFLICT
					if (ls.lft[k].h > i && ls.lft[k - 1].t < i) {
						// printf("lft[%d].h = %d, lft[%d].t = %d\n", k,
						// ls.lft[k].h, k-1, ls.lft[k-1].t);
						finish = CONFLICT;
						break;
					}
				}
				//第二種：黑格在最後一個線索之後
				if (ls.lft[j - 1].t < i) {
					finish = CONFLICT;
					k = j;
				}
				//如果發生衝突
				if (finish == CONFLICT) {
					//從衝突處重新開始
					start = ls.lft[k - 1].h + 1;
					currentClue = k - 1;
					break;
				}
			}
		}
	}

	for (int i = 0; i < j; i++) {
		for (int k = ls.lft[i].h; k <= ls.lft[i].t; k++) {
			left[k] = i + 3;	// 標記位置k屬於線索i
		}
	}

	finish = 0, currentClue = j - 1, start = 24;
	while (finish != SOLVED) {
		int total = 0;
		for (int i = 0; i <= currentClue; i++) {
			total += ls.clue[lineNum].num[i];
		}
		total += currentClue;

		// CONFLICT RULE
		// 從當前 index 開始放置線索會超出範圍時，代表 CONFLICT
		if (start + 1 < total) {
			return CONFLICT;
		}
		int state = rightmost(ls, lineNum, line, start, currentClue);
		if (state == CONFLICT) return CONFLICT;
		finish = SOLVED;

		for (int i = 24; i >= 0; i--) {
			if (__GET(line, i) == BIT_ONE) {
				for (k = 1; k < j; k++) {
					if (ls.rght[k].h > i && ls.rght[k - 1].t < i) {
						// printf("rght[%d].h = %d, rght[%d].t = %d\n", k,
						// ls.rght[k].h, k-1, ls.rght[k-1].t);
						finish = CONFLICT;
						break;
					}
				}
				if (ls.rght[0].h > i) {
					finish = CONFLICT;
					k = 0;
				}
				if (finish == CONFLICT) {
					start = ls.rght[k].t - 1;
					currentClue = k;
					break;
				}
			}
		}
	}

	for (int i = 0; i < j; i++) {
		for (int k = ls.rght[i].h; k <= ls.rght[i].t; k++) {
			right[k] = i + 3;
		}
	}
	
	return SOLVED;
}

//找到每個線索可能的最左側位置
int leftmost(LineSolve& ls, int lineNum, const uint64_t& line, int start,
             int c) {
	int j = ls.clue[lineNum].count;
	// if(lineNum == 27)
	// printf("start = %d, currentClue = %d\n", start, c);
	for (int i = start; i < 25 && c < j; i++) {
		// clue 從 i 開始放超出格子範圍，後面更不可能放下，直接 CONFLICT
		if (i + ls.clue[lineNum].num[c] > 25) break;
		if (i == 24 && c < j - 1) {	//到最後一格了但還有線索沒處理
			// ls.conflict3++;
			return CONFLICT;
		}
		bool error = false;
		//檢查當前位置前後是否有黑格
		if (i - 1 >= 0 && __GET(line, i - 1) == BIT_ONE ||
		    i + ls.clue[lineNum].num[c] < 25 &&
		        __GET(line, i + ls.clue[lineNum].num[c]) == BIT_ONE) {
			continue;
		}
		//檢查從i開始的連續格子是否都不是白格
		for (int k = i; k < i + ls.clue[lineNum].num[c]; k++) {
			if (__GET(line, k) == BIT_ZERO) {
				error = true;
				break;
			}
		}

		//紀錄連續黑格的起始、結束位置
		if (!error) {
			ls.lft[c].h = i;
			ls.lft[c].t = i + ls.clue[lineNum].num[c] - 1;
			// if(lineNum == 27)
			// printf("lft[%d].h = %d, lft[%d].t = %d\n", c, ls.lft[c].h, c,
			// ls.lft[c].t);
			i = ls.lft[c].t + 1;
			c++;
			if (c == j) return SOLVED;
		}
	}
	// 掃完 [start,24] 仍無法放下所有線索 = 真正的 CONFLICT
	return CONFLICT;
}

int rightmost(LineSolve& ls, int lineNum, const uint64_t& line, int start,
              int c) {
	int j = ls.clue[lineNum].count;
	// printf("start = %d, currentClue = %d\n", start, c);
	for (int i = start; i >= 0 && c >= 0; i--) {
		// clue 從 i 往左放超出格子範圍，後面更不可能放下，直接 CONFLICT
		if (i - ls.clue[lineNum].num[c] + 1 < 0) break;
		if (i == 0 && c > 0) {
			// ls.conflict4++;
			return CONFLICT;
		}
		bool error = false;
		if (i + 1 < 25 && __GET(line, i + 1) == BIT_ONE ||
		    i - ls.clue[lineNum].num[c] >= 0 &&
		        __GET(line, i - ls.clue[lineNum].num[c]) == BIT_ONE) {
			continue;
		}
		for (int k = i; k > i - ls.clue[lineNum].num[c]; k--) {
			if (__GET(line, k) == BIT_ZERO) {
				error = true;
				break;
			}
		}
		// printf("i = %d, c = %d\n", i, c);
		if (!error) {
			ls.rght[c].h = i - ls.clue[lineNum].num[c] + 1;
			ls.rght[c].t = i;
			// printf("rght[%d].h = %d, rght[%d].t = %d\n", c, ls.rght[c].h, c,
			// ls.rght[c].t);
			i = ls.rght[c].h - 1;
			c--;
			if (c == -1) return SOLVED;
		}
	}
	// 掃完 [0,start] 仍無法放下所有線索 = 真正的 CONFLICT
	return CONFLICT;
}

// 優化函數：計算每個線索的範圍（規則 2/4/5）
int computeClueScope(LineSolve& ls, int lineNum, int* scope_head, 
                     int* scope_tail, uint16_t& sh_cont, uint16_t& st_cont) {
	int j = ls.clue[lineNum].count;
	sh_cont = 0;
	st_cont = 0;

	std::fill_n(scope_head, 14, 0);
	std::fill_n(scope_tail, 14, 0);
	
	// find the scope of each clue - 優化：只掃一遍 + 位運算
	for (int i = 0; i < 25; i++) {
		if (left[i] > 2) {
			int left_idx = left[i] - 3;
			if (!(sh_cont & (1 << left_idx))) {
				scope_head[left_idx] = i;
				sh_cont |= (1 << left_idx);
			}
		}
		
		if (right[25 - i - 1] > 2) {
			int right_idx = right[25 - i - 1] - 3;
			if (!(st_cont & (1 << right_idx))) {
				scope_tail[right_idx] = 25 - i - 1;
				st_cont |= (1 << right_idx);
			}
		}
	}
	
	return SOLVED;
}

// 優化函數：應用規則 1_5（提示範圍內的黑格合併）
int applyRule1_5(LineSolve& ls, int lineNum, uint64_t& line, 
                 const int* scope_head, const int* scope_tail, 
                 int hPSeg, const int* hPSeg_head, const int* hPSeg_tail) {
	int j = ls.clue[lineNum].count;
	int final[25];
	std::fill_n(final, 25, 2);
	
	// 規則1_5主邏輯
	for (int i = 0; i < hPSeg; i++) {
		for (int k = 0; k < j; k++) {
			int this_clue = ls.clue[lineNum].num[k];
			
			if (scope_head[k] <= hPSeg_head[i] && scope_tail[k] >= hPSeg_tail[i]) {
				// 計算有效範圍
				int h = (scope_tail[k] - this_clue + 1 >= scope_head[k]) ? 
					(scope_tail[k] - this_clue + 1) : scope_head[k];
				int t = (scope_head[k] + this_clue - 1 <= scope_tail[k]) ? 
					(scope_head[k] + this_clue - 1) : scope_tail[k];
				
				// 內層迴圈
				for (int z = h; z <= t - this_clue + 1; z++) {
					bool cont_check = false;
					for (int zz = z; zz < z + this_clue; zz++) {
						if (__GET(line, zz) == BIT_ZERO) {
							cont_check = true;
							break;
						}
					}
					if (!cont_check) {
						if (z > 0) final[z - 1] = 0;
						if (z + this_clue < 25) final[z + this_clue] = 0;
						for (int zz = z; zz < z + this_clue; zz++) {
							if (final[zz] != 0) final[zz] = 1;
						}
					}
				}
			}
		}
	}

	// 應用合併結果
	for (int i = 0; i < 25; i++) {
		if (__GET(line, i) == BIT_UNKNOWN && final[i] == 1) {
			__SET(line, i, BIT_ONE);
		}
	}
	
	return SOLVED;
}

// 優化函數：應用規則 3（段落邊界推理）
int applyRule3(LineSolve& ls, int lineNum, uint64_t& line, int j) {
	int seg_len[j+1] = {0};      // 直接初始化
	int seg_place[j+1] = {0};
	int segment = 0;
	int differ[j+1] = {0};

	int cont = 0;
	for (int i = 1; i < 24; i++) {
		uint64_t curr = __GET(line, i);
		uint64_t prev = __GET(line, i - 1);
		uint64_t next = __GET(line, i + 1);
		
		if (curr == BIT_ZERO) {
			if (prev == BIT_ONE && cont == 0) {
				differ[segment] = 1; // 10
				for (int k = i; k > 0; k--) {
					if (__GET(line, k - 1) != BIT_ONE) {
						seg_place[segment] = k;
						break;
					} else if (k == 1) {
						seg_place[segment] = 0;
					}
				}
				seg_len[segment] = i - seg_place[segment];
				segment++;
			}
			if (next == BIT_ONE) {
				cont = 1;
				differ[segment] = 2; // 01
				seg_place[segment] = i + 1;
				for (int k = i; k < 24; k++) {
					if (__GET(line, k + 1) != BIT_ONE) {
						seg_len[segment] = k - seg_place[segment] + 1;
						break;
					} else if (k == 23) {
						seg_len[segment] = 25 - seg_place[segment];
						break;
					}
				}
				segment++;
			}
		} else if (curr == BIT_UNKNOWN && cont == 1) {
			cont = 0;
		}
	}
	
	// 處理每個段落
	for (int i = 0; i < segment; i++) {
		int min_clue = 24;
		if (differ[i] == 1) { // 10
			// 由 left/right 反推 clue index，需先檢查合法範圍
			if (right[seg_place[i]] <= 2 || left[seg_place[i]] <= 2) continue;
			int right_idx = right[seg_place[i]] - 3;
			int left_idx  = left[seg_place[i]] - 3;
			if (right_idx < 0 || left_idx < 0 || right_idx >= j || left_idx >= j || right_idx > left_idx) {
				continue;
			}
			// 優化：提前計算最小值（k 介於 right_idx..left_idx）
			for (int k = right_idx; k <= left_idx; k++) {
				int clue_val = ls.clue[lineNum].num[k];
				if (clue_val < min_clue) {
					min_clue = clue_val;
				}
			}
			int tail = seg_place[i] + seg_len[i] - 1;
			int head = tail - min_clue + 1;
			
			int cont_check = 0;
			for (int k = tail; k >= head; k--) {
				if (__GET(line, k) == BIT_ZERO) {
					cont_check = 1;
					break;
				}
			}
			
			if (cont_check == 0) {
				for (int k = head; k <= tail; k++) {
					if (__GET(line, k) == BIT_ZERO) {
						return CONFLICT;
					} else {
						__SET(line, k, BIT_ONE);
					}
				}
			}
		} else if (differ[i] == 2) { // 01
				if (right[seg_place[i]] <= 2 || left[seg_place[i]] <= 2) continue;
				int right_idx = right[seg_place[i]] - 3;
				int left_idx  = left[seg_place[i]] - 3;
				if (right_idx < 0 || left_idx < 0 || right_idx >= j || left_idx >= j || right_idx > left_idx) {
					continue;
				}
				for (int k = right_idx; k <= left_idx; k++) {
					int clue_val = ls.clue[lineNum].num[k];
				if (clue_val < min_clue) {
					min_clue = clue_val;
				}
			}
			int head = seg_place[i];
			int tail = seg_place[i] + min_clue - 1;
			
			int cont_check = 0;
			for (int k = head; k <= tail; k++) {
				if (__GET(line, k) == BIT_ZERO) {
					cont_check = 1;
					break;
				}
			}
			
			if (cont_check == 0) {
				for (int k = head; k <= tail; k++) {
					if (__GET(line, k) == BIT_ZERO) {
						return CONFLICT;
					} else {
						__SET(line, k, BIT_ONE);
					}
				}
			}
		}
	}
	
	return SOLVED;
}

// 優化函數：規則 2/4/5 的交叉檢查
int applyCrossCheck(LineSolve& ls, int lineNum, uint64_t& line, 
                    int hPSeg, const int* hPSeg_head, const int* hPSeg_tail,
                    const int* scope_head, const int* scope_tail, int j) {
	int final[25];
	std::fill_n(final, 25, 2);
	
	// 規則 2 4 5的合併
	for (int i = 0; i < hPSeg; i++) {
		int dh = right[hPSeg_head[i]] - 3;
		int dt = left[hPSeg_head[i]] - 3;
		
		if (right[hPSeg_head[i]] != left[hPSeg_head[i]] && 
			dt - dh == 1 &&
			(dh > 0 && ls.lft[dh - 1].h == ls.rght[dh - 1].h || dh == 0)) {
			
			int dh_clue = ls.clue[lineNum].num[dh];
			int dt_clue = ls.clue[lineNum].num[dt];
			
			for (int k = ls.lft[dh].h; k <= ls.rght[dh].h; k++) {
				if ((k > 0 && __GET(line, k - 1) == BIT_ONE) ||
					(k + dh_clue < 25 && __GET(line, k + dh_clue) == BIT_ONE)) {
					continue;
				}
				
				for (int m = ls.lft[dt].h; m <= ls.rght[dt].h; m++) {
					bool error = false;
					
					if ((m > 0 && __GET(line, m - 1) == BIT_ONE) ||
						(m + dt_clue < 25 && __GET(line, m + dt_clue) == BIT_ONE) ||
						(k + dh_clue == m) ||
						(hPSeg_tail[i] < k ||
						(hPSeg_head[i] >= k + dh_clue && hPSeg_tail[i] < m) ||
						hPSeg_head[i] >= m + dt_clue)) {
						continue;
					}
					
					for (int d = k; d < k + dh_clue && !error; d++) {
						error = (__GET(line, d) == BIT_ZERO);
					}
					for (int d = m; d < m + dt_clue && !error; d++) {
						error = (__GET(line, d) == BIT_ZERO);
					}
					
					if (!error) {
						for (int d = k; d < k + dh_clue; d++) {
							final[d] = 1;
						}
						for (int d = m; d < m + dt_clue; d++) {
							final[d] = 1;
						}
					}
				}
			}

			for (int k = ls.lft[dh].h; k <= ls.rght[dh].t; k++) {
				if (__GET(line, k) == BIT_UNKNOWN && final[k] == 2) {
					__SET(line, k, BIT_ZERO);
				}
			}
		}
	}
	
	return SOLVED;
}

std::queue<int> q;
bool inQueue[50];
int solveLine(LineSolve& ls, Board& board, int lineNum, uint64_t& line){
	// printf("Solving line: %d\n", lineNum);
	int j = ls.clue[lineNum].count;

	// 記錄初始狀態
	uint64_t line_temp = line;

	// 應用 RLmost 演算法
	int state = RLmost(ls, lineNum, line);
	
	if (state == CONFLICT)	return CONFLICT;
	
	for (int i = 0; i < 25; i++) {
		if (__GET(line, i) != __GET(line_temp, i)) ls.RLmost_pixel++;
	}
	line_temp = line;
	
	// 計算提示範圍並應用規則 2/4/5
	/*
	計算每個提示的範圍
	**範例**
	提示 -> 2 2
	行 : x x 1 x x x x 1 x x
	左 : x 3 3 x x x 4 4 x x
	右 : x x 3 3 x x x 4 4 x

	計算每個提示的起點和終點
	scope_head[0] = 1
	scope_tail[0] = 3
	scope_head[1] = 6
	scope_tail[1] = 8
	---------------------------------
	將範圍內的空格設為0
	最終: 0 x 1 x 0 0 x 1 x 0
	*/
	// if(hasUnknown){
	int scope_head[14], scope_tail[14];
	uint16_t sh_cont = 0, st_cont = 0;
	computeClueScope(ls, lineNum, scope_head, scope_tail, sh_cont, st_cont);
	
	//規則2 4 5優化(bitmask)，減少25 * clue_length * clue_count次掃描
	uint32_t zeroMask = 0;
	uint32_t oneMask  = 0;

	for (int i = 0; i < 25; ++i) {
		uint64_t p = __GET(line, i);
		if (p == BIT_ZERO) zeroMask |= (1u << i);
		else if (p == BIT_ONE) oneMask |= (1u << i);
	}

	uint32_t finalMask = 0;

	for (int i = 0; i < j; ++i) {
		int clue = ls.clue[lineNum].num[i];
		uint32_t clueMask = (1u << clue) - 1;

		int start = scope_head[i];
		int end   = scope_tail[i] - clue + 1;

		for (int k = start; k <= end; ++k) {
			// 這一段 [k, k+clue) 裡不能有 0
			if (((zeroMask >> k) & clueMask) != 0) continue;

			// 左右不能黏到 1
			if (k > 0      && (oneMask & (1u << (k - 1))))   continue;
			if (k + clue < 25 && (oneMask & (1u << (k + clue)))) continue;

			finalMask |= (clueMask << k);
		}
	}

	// 只保留 0..24 的 bit
	uint32_t mask25 = (1u << 25) - 1;
	uint32_t unknownMask = (~(zeroMask | oneMask)) & mask25;

	// Conflict Rule：有 1 但 finalMask 為 0
	for (int i = 0; i < 25; ++i) {
		if ((oneMask & (1u << i)) && ((finalMask & (1u << i)) == 0)) {
			// ls.conflict5++;
			return CONFLICT;
		}
	}

	// 把「UNKNOWN 且 final==0」的格子設成 0
	uint32_t fillZero = unknownMask & ~finalMask;

	for (int i = 0; i < 25; ++i) {
		if (fillZero & (1u << i)) {
			__SET(line, i, BIT_ZERO);
		}
	}

	for (int i = 0; i < 25; i++) {
		if (__GET(line, i) != __GET(line_temp, i)) ls.step_2_pixel++;
	}
	line_temp = line;
	// 應用規則 1（提示交集）
	/*
	每個提示的範圍
	**範例**
	提示 -> 3 4
	行 : x 1 x x x x x x 1 x
	左 : 3 3 3 x x 4 4 4 4 x
	右 : x 3 3 3 x x 4 4 4 4
	終 : x 1 1 x x x 1 1 1 x
	*/
	for (int i = 0; i < j; i++) {
		int rgh_h = ls.rght[i].h;
		int lft_t = ls.lft[i].t;
		
		for (int k = lft_t; k >= rgh_h; k--) {
			uint64_t pixel = __GET(line, k);
			if (pixel == BIT_ZERO) {
				return CONFLICT;
			} else if (pixel != BIT_ZERO) {
				__SET(line, k, BIT_ONE);
			}
		}
	}
	
	for (int i = 0; i < 25; i++) {
		if (__GET(line, i) != __GET(line_temp, i)) ls.step_6_pixel++;
	}
	line_temp = line;

	// 應用規則 3（段落邊界推理）
	/*
	計算每個段落的範圍
	**範例**
	提示 -> 2 4
	行 : x x 0 1 x x x x x x
	左 : 3 3 x 4 4 4 4 x x x
	右 : x x x 3 3 x 4 4 4 4
	終 : x x 0 1 1 x x x x x

	計算每個段落的範圍
	differ[0] = 2 (01 段落的開始)
	seg_place[0] = 3
	seg_len[0] = 2

	計算最小提示
	min_clue = 2
	---------------------------
	最終: x x 0 1 1 x x x x x
	*/
	if(applyRule3(ls, lineNum, line, j) == CONFLICT)	return CONFLICT;

	for (int i = 0; i < 25; i++) {
		if (__GET(line, i) != __GET(line_temp, i)) ls.step_7_pixel++;
	}
	line_temp = line;

	// 更新棋盤和變化標記
	for (int i = 0; i < 25; i++) {
		uint64_t old_pixel = __GET(board.data[lineNum], i);
		uint64_t new_pixel = __GET(line, i);
		
		if (old_pixel != new_pixel) {
			if (lineNum < 25) {

				int col = i + 25;
				__SET(board.data[col], lineNum, new_pixel);

				if (!inQueue[col]) {
					q.push(col);
					inQueue[col] = true;
				}

			} else {

				int row = i;
				__SET(board.data[row], lineNum - 25, new_pixel);

				if (!inQueue[row]) {
					q.push(row);
					inQueue[row] = true;
				}
			}
		}
	}

	board.data[lineNum] = line;

	return SOLVED;
}

int logicRule(LineSolve& ls, Board& board){
	while(!q.empty())	q.pop();

	for (int i = 25; i < 50; i++) {
		inQueue[i] = false;
	}

	for (int i = 0; i < 25; i++) {
		q.push(i);
		inQueue[i] = true;
	}

	while (!q.empty()) {
		int lineNum = q.front();
		q.pop();
		inQueue[lineNum] = false;
		
		uint64_t line = getLine(board, lineNum);
		int state = solveLine(ls, board, lineNum, line);
		if (state == CONFLICT)	return CONFLICT;
	}
	
	// 檢查是否達到求解完成
	for (int i = 0; i < 50; i++) {
		if (board.oldData[i] != board.data[i]) {
			return INCOMP;
		}
	}

	return SOLVED;
}

void printLog(const std::string& phases, int pixels) {
	FILE* out = fopen("log.txt", "a+");
	fprintf(out, "%s: %d\n", phases.c_str(), pixels);
	fclose(out);
}
