#ifndef LINESOLVE_H
#define LINESOLVE_H

#include <stdint.h>

#include "Hash.h"
#include "board.h"
#include "cdef.h"

// don't share with other thread when you wanna parallism
class LineSolve {
	public:
		// problem info
		Clue clue[50];
		Pixel lft[14], rght[14];
		int change[50];
		// additional info
		int low_bound[50][14];

		Board solvedBoard;

		LineSolve();
		LineSolve(int*);
		void load(int*);
		void init_change() {
			for (int i = 0; i < 50; i++) {
				change[i] = 1;
			}
		}

		int linesolvetime = 0;

		int RLmost_pixel = 0;
		int step_2_pixel = 0;
		int step_4_pixel = 0;
		int step_5_pixel = 0;
		int step_6_pixel = 0;
		int step_7_pixel = 0;
		int step_8_pixel = 0;
		Board step_7_board;
		int conflict1 = 0;
		int conflict2 = 0;
		int conflict3 = 0;
		int conflict4 = 0;
		int conflict5 = 0;

	private:
		void init();
};

int fixBU(LineSolve&, int, const uint64_t&, int, uint64_t&);
int propagate(LineSolve&, Board&);

int RLmost_init(LineSolve&, Board&);
int RLmost(LineSolve&, int, const uint64_t&);
int leftmost(LineSolve&, int, const uint64_t&, int, int);
int rightmost(LineSolve&, int, const uint64_t&, int, int);
int logicRule(LineSolve&, Board&);
int logicSolve(LineSolve&, Board&);

// 優化：提取的輔助函數
int computeClueScope(LineSolve& ls, int lineNum, int* scope_head, 
                     int* scope_tail, uint16_t& sh_cont, uint16_t& st_cont);
int applyRule1_5(LineSolve& ls, int lineNum, uint64_t& line, 
                 const int* scope_head, const int* scope_tail, 
                 int hPSeg, const int* hPSeg_head, const int* hPSeg_tail);
int applyRule3(LineSolve& ls, int lineNum, uint64_t& line, int j);
int applyCrossCheck(LineSolve& ls, int lineNum, uint64_t& line, 
                    int hPSeg, const int* hPSeg_head, const int* hPSeg_tail,
                    const int* scope_head, const int* scope_tail, int j);

int checkClueMatch(LineSolve& ls, int lineNum, int clueCount);
void updateRightPositions(LineSolve& ls, int clueCount);
void updateLeftPositions(LineSolve& ls, int clueCount);
int calculateTotalClueLength(LineSolve& ls, int lineNum, int startClue,
                             int endClue);
int processRightmost(LineSolve& ls, int lineNum, const uint64_t& line,
                     int clueCount);
int processLeftmost(LineSolve& ls, int lineNum, const uint64_t& line,
                    int& start, int& currentClue, int clueCount);

void logConflict(const char* path, const std::string& message, LineSolve& ls,
                 int lineNum, const uint64_t& line);
void printLog(const std::string& phases, int pixels);
#endif
