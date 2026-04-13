#ifndef PROB_SOLVER_H
#define PROB_SOLVER_H

#include <deque>

#include "board.h"
#include "fullyprobe.h"
#include "help.h"
#include "linesolve.h"
#include "mirror.h"
#include "scanner.h"

using std::deque;

class NonogramSolver {
	public:
		NonogramSolver() {}
		int doSolve(int *data, int probN);
		void setMethod(int n);
		void dfs(FullyProbe &, LineSolve &, Board);
		void dfs_stack(FullyProbe &, LineSolve &, Board, int);
		Board getSolvedBoard() { return ls.solvedBoard; }
		int pixel = 0;
		int TotalLineSolveTime = 0;
		double propagationTime = 0;

		int times;

		int getRLmostPixel() { return ls.RLmost_pixel; }
		int getStep2Pixel() { return ls.step_2_pixel; }
		int getStep4Pixel() {return ls.step_4_pixel;}
		int getStep5Pixel() { return ls.step_5_pixel; }
		int getStep6Pixel() {return ls.step_6_pixel;}
		int getStep7Pixel() { return ls.step_7_pixel; }
		int getStep8Pixel() {return ls.step_8_pixel;}
		Board getStep7Board() {return ls.step_7_board;}
		int getConflict1() { return ls.conflict1; }
		int getConflict2() { return ls.conflict2; }
		int getConflict3() { return ls.conflict3; }
		int getConflict4() { return ls.conflict4; }
		int getConflict5() { return ls.conflict5; }
	private:
		LineSolve ls;
		FullyProbe fp;
		bool finish;
		int thres;
		int sw;
		deque<Board> queue[2];
		int depth_rec[626];
		int max_depth;
};

#endif
