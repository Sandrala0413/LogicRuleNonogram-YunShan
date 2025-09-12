#include "linesolve.h"

#include <stdlib.h>

#include <cstdio>

const uint64_t value1[28] = {
    0x1ULL,     // 0
    0x9ULL,     // 10
    0x29ULL,    // 010
    0xA9ULL,    // 1110
    0x2A9ULL,   // 01110
    0xAA9ULL,   // 111110
    0x2AA9ULL,  // 0111110
    0xAAA9ULL,  // 11111110
    0x2AAA9ULL, // 011111110
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
    0x2AAAAAAAAAAAA9ULL, // 011111111111111111111111110
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

void LineSolve::load(int* d) {
	MEMSET_ZERO(low_bound);

	for (int i = 0; i < 50; ++i) {
		int sum = 0;

		clue[i].count = d[i * 14];

		for (int j = 1; j <= d[i * 14]; ++j) {
			clue[i].num[j - 1] = d[i * 14 + j];
			sum += d[i * 14 + j] + 1;
			low_bound[i][j] = sum - 1;
		}

		genHash(clue[i]);
	}
}

int propagate(LineSolve& ls, Board& board) {
	uint64_t chkline = 0;
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

		lineNum = __builtin_ffsll(chkline) - 1;
		// printf("lineNum = %d\n", lineNum);
		chkline &= chkline - 1;

		uint64_t line = getLine(board, lineNum)
		                << 4;     // 0033333333333333333333333
		__SET(line, 1, BIT_ZERO); // 0133333333333333333333333

		uint64_t newLine = 0LL;

		if (!findHash(ls.clue[lineNum], line, newLine)) // ��֮ǰ�ě]�����
		{
			newLine = 0LL;

			const int fixAns =
			    fixBU(ls, lineNum, line, ls.clue[lineNum].count, newLine);
			if (unlikely(LS_NO == fixAns)) {
				return CONFLICT;
			}

			insertHash(ls.clue[lineNum], line, newLine); // ����line��
		}

		newLine >>= 4;
		line >>= 4;

		if (line != newLine) {
			board.data[lineNum] = newLine;

			uint64_t p = line ^ newLine;
			int x = 1;

			while (x = __builtin_ffsll(p), x-- != 0) {
				uint64_t bit = (x & 0x1) == 0 ? BIT_ONE : BIT_ZERO;
				p &= p - 1;
				x >>= 1;

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
		int totaClue = 0;
		for (int i = 0; i < j; i++) {
			totaClue += ls.clue[lineNum].num[i];
		}
		totaClue = totaClue + j - 1;
		shift = 25 - totaClue;
		int point = shift;
		for (int i = 0; i < j; i++) {
			if (shift < ls.clue[lineNum].num[i]) {
				int paint = ls.clue[lineNum].num[i] - shift;
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

int RLmost(LineSolve& ls, int lineNum, const uint64_t& line) {
	for (int i = 0; i < 25; i++) {
		left[i] = 2;
		right[i] = 2;
	}
	int j = ls.clue[lineNum].count;
	int finish = 0, start = 0, k, currentClue = 0;
	bool error;
	while (finish != SOLVED) {
		int total = 0;
		for (int i = currentClue; i < j; i++) {
			total += ls.clue[lineNum].num[i];
		}
		total += j - currentClue - 1;

		// CONFLICT RULE
		// 從當前 index 開始放置線索會超出範圍時，代表 CONFLICT
		if (25 - start < total) {
			return CONFLICT;
		}

		int state = leftmost(ls, lineNum, line, start, currentClue);
		if (state == CONFLICT) return CONFLICT;
		finish = SOLVED;

		for (int i = 0; i < 25; i++) {
			if (__GET(line, i) == BIT_ONE) {
				for (k = j - 1; k > 0; k--) {
					if (ls.lft[k].h > i && ls.lft[k - 1].t < i) {
						// printf("lft[%d].h = %d, lft[%d].t = %d\n", k,
						// ls.lft[k].h, k-1, ls.lft[k-1].t);
						finish = CONFLICT;
						break;
					}
				}
				if (ls.lft[j - 1].t < i) {
					finish = CONFLICT;
					k = j;
				}
				if (finish == CONFLICT) {
					start = ls.lft[k - 1].h + 1;
					currentClue = k - 1;
					break;
				}
			}
		}
	}

	for (int i = 0; i < j; i++) {
		for (int k = ls.lft[i].h; k <= ls.lft[i].t; k++) {
			left[k] = i + 3;
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

	int from = 0, cl, cr;
	for (int i = 0; i < j; i++) {
		int clue = i + 3;
		cl = 0, cr = 0;
		for (int k = from; k < 25; k++) {
			if (left[k] == clue) {
				cl++;
				if (cl == ls.clue[lineNum].num[i]) from = k;
			}
			if (right[k] == clue) {
				cr++;
			}
			if (cl > ls.clue[lineNum].num[i] || cr > ls.clue[lineNum].num[i]) {
				return CONFLICT;
			}
			if (left[k] > clue && cl < ls.clue[lineNum].num[i] ||
			    right[k] > clue && cr < ls.clue[lineNum].num[i]) {
				return CONFLICT;
			}
		}
		// CONFLICT RULE
		// 線索對應錯誤，代表 CONFLICT
		if (cl < ls.clue[lineNum].num[i] || cr < ls.clue[lineNum].num[i]) {
			return CONFLICT;
		}
	}
	return SOLVED;
}

int leftmost(LineSolve& ls, int lineNum, const uint64_t& line, int start,
             int c) {
	int j = ls.clue[lineNum].count;
	// if(lineNum == 27)
	// printf("start = %d, currentClue = %d\n", start, c);
	for (int i = start; i < 25, c < j; i++) {
		if (i == 24 && c < j - 1) {
			return CONFLICT;
		}
		bool error = false;
		if (i - 1 >= 0 && __GET(line, i - 1) == BIT_ONE ||
		    i + ls.clue[lineNum].num[c] < 25 &&
		        __GET(line, i + ls.clue[lineNum].num[c]) == BIT_ONE) {
			continue;
		}

		for (int k = i; k < i + ls.clue[lineNum].num[c]; k++) {
			if (__GET(line, k) == BIT_ZERO) {
				error = true;
				break;
			}
		}

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
}
int rightmost(LineSolve& ls, int lineNum, const uint64_t& line, int start,
              int c) {
	int j = ls.clue[lineNum].count;
	// printf("start = %d, currentClue = %d\n", start, c);
	for (int i = start; i >= 0, c >= 0; i--) {
		if (i == 0 && c > 0) {
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
}
int logicRule(LineSolve& ls, Board& board) {
	for (int lineNum = 0; lineNum < 50; ++lineNum) {
		if (ls.change[lineNum] != 1) {
			continue;
		}
		uint64_t line = getLine(board, lineNum);
		// uint64_t newLine = ( ( 0x1LL << 50 ) - 0x1LL );
		int j = ls.clue[lineNum].count;

		int state = RLmost(ls, lineNum, line);

		if (state == CONFLICT) {
			return CONFLICT;
		}

		// 規則 2 4 5
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
		int scope_head[j], scope_tail[j], sh_cont[j], st_cont[j], cont = 0;
		int final[25];

		for (int i = 0; i < j; i++) {
			scope_head[i] = 0;
			scope_tail[i] = 0;
			sh_cont[i] = 0;
			st_cont[i] = 0;
		}
		for (int i = 0; i < 25; i++) {
			final[i] = 0;
		}

		// find the scope of each clue
		for (int i = 0; i < 25; i++) {
			if (left[i] > 2 && !sh_cont[left[i] - 3]) {
				scope_head[left[i] - 3] = i;
				sh_cont[left[i] - 3] = 1;
			}
			if (right[25 - i - 1] > 2 && !st_cont[right[25 - i - 1] - 3]) {
				scope_tail[right[25 - i - 1] - 3] = 25 - i - 1;
				st_cont[right[25 - i - 1] - 3] = 1;
			}
		}
		int this_clue, cont_t = 0, cont_h = 0;

		for (int i = 0; i < j; i++) {
			this_clue = ls.clue[lineNum].num[i];
			for (int k = scope_head[i]; k <= scope_tail[i] - this_clue + 1;
			     k++) {
				cont = 0, cont_t = 0, cont_h = 0;
				for (int z = 0; z < this_clue; z++) {
					if (__GET(line, k + z) == BIT_ZERO) {
						cont = 1;
						break;
					}
				}
				if (k + this_clue < 25 &&
				    __GET(line, k + this_clue) == BIT_ONE) {
					cont_t = 1;
				}
				if (k - 1 >= 0 && __GET(line, k - 1) == BIT_ONE) {
					cont_h = 1;
				}
				if (!cont && !cont_t && !cont_h) {
					for (int z = 0; z < this_clue; z++) {
						final[k + z] = 1;
					}
				}
			}
		}

		// Conflict Rule
		for (int i = 0; i < 25; i++) {
			if (final[i] == 0) {
				if (__GET(line, i) == BIT_ONE) return CONFLICT;
				if (__GET(line, i) == BIT_UNKNOWN) __SET(line, i, BIT_ZERO);
			}
		}

		// 規則1_5
		/*
		計算每個提示的範圍
		**範例**
		提示 -> 3 4
		行 : x x x 0 x 1 x x x x x x x
		左 : 3 3 3 0 4 4 4 4 x x x x x
		右 : x x x 0 x 3 3 3 x 4 4 4 4
		終:  x x x 0 x 1 1 x x x x x x

		計算提示的範圍
		hPSeg_head[0] = 5
		hPSeg_tail[0] = 5
		hPSeg_len[0] = 1

		將範圍內的空格設為1
		(如果範圍內有0，則設為0)
		最終: x x x 0 1 1 1 0 x x x x x
		最終: x x x 0 0 1 1 0 x x x x x
		最終: x x x 0 0 1 1 0 0 x x x x
		最終: x x x 0 0 1 1 0 0 x x x x
		---------------------------------
		將範圍內的空格設為1
		最終: x x x 0 x 1 1 x x x x x x
		*/
		int hPSeg = 0;
		int hPSeg_head[13];
		int hPSeg_tail[13];
		int hPSeg_len[13];

		// init array
		for (int i = 0; i < 13; i++) {
			hPSeg_head[i] = 0;
			hPSeg_tail[i] = 0;
			hPSeg_len[i] = 0;
		}
		cont = 0;
		for (int i = 0; i < 25; i++) {
			if (__GET(line, i) == BIT_ONE && !cont) {
				hPSeg_head[hPSeg] = i;
				cont = 1;
			} else if (__GET(line, i) != BIT_ONE && cont) {
				hPSeg_tail[hPSeg] = i - 1;
				hPSeg_len[hPSeg] = i - hPSeg_head[hPSeg];
				hPSeg++;
				cont = 0;
			} else if (i + 1 == 25 && cont) {
				hPSeg_tail[hPSeg] = i;
				hPSeg_len[hPSeg] = i - hPSeg_head[hPSeg] + 1;
				hPSeg++;
				cont = 0;
			}
		}

		for (int i = 0; i < 25; i++) {
			final[i] = 2;
		}

		int h = 0, t = 0;
		for (int i = 0; i < hPSeg; i++) {
			for (int k = 0; k < j; k++) {
				this_clue = ls.clue[lineNum].num[k];
				if (scope_head[k] <= hPSeg_head[i] &&
				    scope_tail[k] >= hPSeg_tail[i]) {
					if (scope_tail[k] - this_clue + 1 >= scope_head[k]) {
						h = scope_tail[k] - this_clue + 1;
					} else {
						h = scope_head[k];
					}
					if (scope_head[k] + this_clue - 1 <= scope_tail[k]) {
						t = scope_head[k] + this_clue - 1;
					} else {
						t = scope_tail[k];
					}
					for (int z = h; z <= t - this_clue + 1; z++) {
						cont = false;
						for (int zz = z; zz < z + this_clue; zz++) {
							if (__GET(line, zz) == BIT_ZERO) cont = true;
						}
						if (!cont) {
							final[z - 1] = 0;
							final[z + this_clue] = 0;
							for (int zz = z; zz < z + this_clue; zz++) {
								if (final != 0) final[zz] = 1;
							}
						}
					}
				}
			}
		}

		// merge
		for (int i = 0; i < 25; i++) {
			if (__GET(line, i) == BIT_UNKNOWN && final[i] == 1) {
				__SET(line, i, BIT_ONE);
				// cau
				// board.rule[7]++;
				// num_s=1;
			}
		}

		for (int i = 0; i < 25; i++) {
			final[i] = 2;
		}

		// 規則 2 4 5的合併(檢查前後提示的範圍)
		for (int i = 0; i < hPSeg; i++) {
			if (right[hPSeg_head[i]] != left[hPSeg_head[i]]) {
				int dh = right[hPSeg_head[i]] - 3, dt = left[hPSeg_head[i]] - 3;
				if (dt - dh == 1 &&
				    (dh > 0 && ls.lft[dh - 1].h == ls.rght[dh - 1].h ||
				     dh == 0)) {
					Pixel ll[2];
					for (int k = ls.lft[dh].h; k <= ls.rght[dh].h; k++) {
						if ((k - 1 >= 0 && __GET(line, k - 1) == BIT_ONE) ||
						    (k + ls.clue[lineNum].num[dh] < 25 &&
						     __GET(line, k + ls.clue[lineNum].num[dh]) ==
						         BIT_ONE)) {
							continue;
						}
						for (int m = ls.lft[dt].h; m <= ls.rght[dt].h; m++) {
							bool error = false;
							if ((m - 1 >= 0 && __GET(line, m - 1) == BIT_ONE) ||
							    (m + ls.clue[lineNum].num[dt] < 25 &&
							     __GET(line, m + ls.clue[lineNum].num[dt]) ==
							         BIT_ONE) ||
							    (k + ls.clue[lineNum].num[dh] == m) ||
							    (hPSeg_tail[i] < k ||
							     (hPSeg_head[i] >=
							          k + ls.clue[lineNum].num[dh] &&
							      hPSeg_tail[i] < m) ||
							     hPSeg_head[i] >=
							         m + ls.clue[lineNum].num[dt])) {
								continue;
							}
							for (int d = k; d < k + ls.clue[lineNum].num[dh];
							     d++) {
								if (__GET(line, d) == BIT_ZERO) {
									error = true;
									break;
								}
							}
							for (int d = m; d < m + ls.clue[lineNum].num[dt];
							     d++) {
								if (__GET(line, d) == BIT_ZERO) {
									error = true;
									break;
								}
							}
							if (!error) {
								for (int d = k;
								     d < k + ls.clue[lineNum].num[dh]; d++) {
									final[d] = 1;
								}
								for (int d = m;
								     d < m + ls.clue[lineNum].num[dt]; d++) {
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
		}

		// 規則1
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
			for (int k = ls.lft[i].t; k >= ls.rght[i].h; k--) {
				if (__GET(line, k) == BIT_ZERO)
					return CONFLICT;
				else
					__SET(line, k, BIT_ONE);
			}
		}

		// 規則3
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
		int seg_len[j + 1];
		int seg_place[j + 1];
		int segment = 0;
		int differ[j + 1];
		for (int i = 0; i < j + 1; i++) {
			seg_len[i] = 0;
			seg_place[i] = 0;
			differ[i] = 0;
		}
		cont = 0;
		for (int i = 1; i < 24; i++) {
			if (__GET(line, i) == BIT_ZERO) {
				if (__GET(line, i - 1) == BIT_ONE && cont == 0) {
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
				if (__GET(line, i + 1) == BIT_ONE) {
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
			} else if (__GET(line, i) == BIT_UNKNOWN && cont == 1) {
				cont = 0;
			}
		}
		for (int i = 0; i < segment; i++) {
			int min_clue = 24, head = 0, tail = 0;
			if (differ[i] == 1) { // 10
				for (int k = right[seg_place[i]] - 3;
				     k <= left[seg_place[i]] - 3; k++) {
					if (ls.clue[lineNum].num[k] < min_clue) {
						min_clue = ls.clue[lineNum].num[k];
					}
				}
				tail = seg_place[i] + seg_len[i] - 1;
				head = tail - min_clue + 1;
				cont = 0;
				for (int k = tail; k >= head; k--) {
					if (__GET(line, k) == BIT_ZERO) {
						cont = 1;
						break;
					}
				}
				if (cont == 0) {
					for (int k = head; k <= tail; k++) {
						if (__GET(line, k) == BIT_ZERO) {
							return CONFLICT;
						} else
							__SET(line, k, BIT_ONE);
					}
				}
			} else if (differ[i] == 2) { // 01
				for (int k = right[seg_place[i]] - 3;
				     k <= left[seg_place[i]] - 3; k++) {
					if (ls.clue[lineNum].num[k] < min_clue) {
						min_clue = ls.clue[lineNum].num[k];
					}
				}
				head = seg_place[i];
				tail = seg_place[i] + min_clue - 1;
				cont = 0;
				for (int k = head; k <= tail; k++) {
					if (__GET(line, k) == BIT_ZERO) {
						cont = 1;
						break;
					}
				}
				if (cont == 0) {
					for (int k = head; k <= tail; k++) {
						if (__GET(line, k) == BIT_ZERO) {
							return CONFLICT;
						} else
							__SET(line, k, BIT_ONE);
					}
				}
			}
		}

		// EXTRA_PLUS
		/*
		檢查額外的規則
		**範例**
		提示 -> 3 1 1
		行 : x x x 0 1 x x x x 0 1
		左 : 3 3 3 x 4 x x x x x 5
		右 : x x x x 3 3 3 x 4 x 5
		終 : x x x 0 1 x x 0 x 0 1

		計算提示的範圍
		dh = 0
		dt = 1

		檢查dh和dt之間的範圍，並確定dh到dt之間的空格
		檢查dt是否在範圍內，並確定下一個位置是否固定
		-----------------------------
		將段落內的空格設為1，並確定dh到dt之間的範圍
		       x x x 0 1 1 1 x x 0 1
		最終結果
		final: x x x 0 1 x x 0 x 0 1
		*/
		for (int i = segment - 1; i >= 0; i--) {
			int stop = 0;
			if (differ[i] == 2) { // 01
				int len = 0;
				// find len
				for (int k = seg_place[i] + seg_len[i]; k < 25; k++) {
					if (__GET(line, k) != BIT_UNKNOWN)
						len++;
					else if (__GET(line, k) != BIT_UNKNOWN) {
						stop = 1;
						break;
					}
				}
				if (stop) break;
				len += seg_len[i];
				int dh = right[seg_place[i]] - 3, dt = left[seg_place[i]] - 3;
				if (dt - dh == 1 &&
				    ls.clue[lineNum].num[dt] <= ls.clue[lineNum].num[dh] &&
				    seg_len[i] != ls.clue[lineNum].num[dh]) {
					if (dt == j - 1 || (dt != j - 1 && ls.rght[dt + 1].h ==
					                                       ls.lft[dt + 1].h)) {
						__SET(line, seg_place[i] + ls.clue[lineNum].num[dh],
						      BIT_ZERO);
					}
				}
			}
		}

		int hasN = 0;
		for (int i = 0; i < 25; i++) {
			if (__GET(line, i) != __GET(board.data[lineNum], i)) {
				if (lineNum < 25) {
					__SET(board.data[i + 25], lineNum, __GET(line, i));
					ls.change[i + 25] = 1;
				} else {
					__SET(board.data[i], (lineNum - 25), __GET(line, i));
					ls.change[i] = 1;
				}
				hasN = 1;
			}
		}
		if (hasN == 1)
			ls.change[lineNum] = 1;
		else
			ls.change[lineNum] = 0;

		board.data[lineNum] = line;
	}

	for (int i = 0; i < 50; i++) {
		// printf("oldData = %llu\nnewData = %llu\n", board.oldData[i],
		// board.data[i] );
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
