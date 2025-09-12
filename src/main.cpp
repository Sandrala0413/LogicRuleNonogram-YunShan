#include <time.h>

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <queue>
#include <vector>

#include "options.h"
#include "probsolver.h"

using namespace std;

void testConflictBoard(int* input_data) {
	clearFile("ConflictBoardsReults.txt");

	FILE* in = fopen("conflict-tcga2024.txt", "r");
	LineSolve ls;
	int probN, totalBoards = 0, cs = 0;
	int incomplete_cnt = 0, conflict_cnt = 0;
	while (fscanf(in, "$%d\n", &probN) != EOF) {
		// FILE* out = fopen("ConflictBoardsReults.txt", "a+");
		// fprintf(out, "$%d\n", totalBoards);
		// fprintf(out, "Clue: %d\n", probN);
		// fclose(out);

		totalBoards++;

		int probData[50 * 14];
		getData(input_data, probN, probData);
		ls.load(probData);

		Board board;
		char pixel;
		for (int i = 0; i < 25; i++) {
			for (int j = 0; j < 25; j++) {
				if (j != 24) {
					fscanf(in, "%c\t", &pixel);
				} else {
					fscanf(in, "%c\n", &pixel);
				}

				if (pixel == '1') {
					__SET(board.data[j], i, BIT_ONE);
					__SET(board.data[i + 25], j, BIT_ONE);
				} else if (pixel == '0') {
					__SET(board.data[j], i, BIT_ZERO);
					__SET(board.data[i + 25], j, BIT_ZERO);
				} else {
					__SET(board.data[j], i, BIT_UNKNOWN);
					__SET(board.data[i + 25], j, BIT_UNKNOWN);
				}
			}
		}

		int state = logicSolve(ls, board);
		if (state == CONFLICT) {
			conflict_cnt++;
		}

		if (state == INCOMP) {
			incomplete_cnt++;
		}
	}
	FILE* out = fopen("ConflictBoardsReults.txt", "a+");
	fprintf(out, "Check Conflicts: %d/%d\n", conflict_cnt, totalBoards);
	fprintf(out, "Check Incomplete: %d/%d\n", incomplete_cnt, totalBoards);
	printf("Check Conflicts: %d/%d\n", conflict_cnt, totalBoards);
	printf("Check Incomplete: %d/%d\n", incomplete_cnt, totalBoards);
	fclose(in);
	fclose(out);
}

int main(int argc, char* argv[]) {
	Options option;
	if (!option.readOptions(argc, argv)) {
		printf("\nAborted: Illegal Options.\n");
		return 0;
	}

	if (option.genLogFile()) {
		printf("\nopen log(%s) and write info failed\n", option.logFileName);
		return 0;
	}

	clearFile(option.outputFileName);

	int* inputData;
	int probData[50 * 14];
	inputData = allocMem(1001 * 50 * 14);
	readFile(option.inputFileName, inputData);

	// testConflictBoard(inputData);
	// return 0;

	NonogramSolver nngSolver;

	nngSolver.setMethod(option.method);

	initialHash();

	vector<Board> answer;
	answer.resize(option.problemEnd - option.problemStart + 1);
	int total = 0;
	int totalDFSTime = 0;
	int ProblemNumber = option.problemEnd - option.problemStart + 1;
	int finishProblem = 0;

	clearFile(option.logFileName);
	clock_t totalTIME = 0;

	nngSolver.propagationTime = 0;
	for (int probN = option.problemStart; probN <= option.problemEnd; ++probN) {
		getData(inputData, probN, probData);

		FILE* out = fopen(option.logFileName, "a+");
		fprintf(out, "$%d\n", probN);
		fclose(out);

		clock_t beginTime = clock();
		if (!nngSolver.doSolve(probData, probN)) {
			printf("Fatal Error: Solve failed\n");
		}
		clock_t endTime = clock();

		Board ans = nngSolver.getSolvedBoard();

		out = fopen("log.txt", "a+");
		fprintf(out, "Time: %3.6lf\n",
		        (double)(endTime - beginTime) / CLOCKS_PER_SEC);
		if (debugBoard(ans) != 625) {
			fprintf(out, "INCOMPLETE\n");
		} else {
			fprintf(out, "SOLVED\n");
		}
		fclose(out);

		total += debugBoard(ans);
		totalDFSTime += nngSolver.times;
		if (nngSolver.pixel == 625) finishProblem++;
		printf("$%-3d: %3.6lf sec\n", probN,
		       (double)(endTime - beginTime) / CLOCKS_PER_SEC);
		totalTIME += endTime - beginTime;
		printBoard(option.outputFileName, ans, probN);
	}
	delete[] inputData;

	printf("--------------------\n");
	printf("Finish Problem=%d\n", finishProblem);
	printf("--------------------\n");
	printf("total pixel: %d \n", total);
	printf("Average pixel: %f\n", (double)total / ProblemNumber);
	printf("--------------------\n");
	printf("Line solve time: %d\n", nngSolver.TotalLineSolveTime);
	printf("--------------------\n");
	printf("Total Propagation Time: %lf sec\n", nngSolver.propagationTime);
	printf("Average Propagation Time: %lf sec\n",
	       nngSolver.propagationTime / ProblemNumber);
	printf("--------------------\n");
	printf("Total TIME: %.4lf sec\n", (double)totalTIME / CLOCKS_PER_SEC);
	printf("Total DFS Time: %d times\n", totalDFSTime);
	printf("Average DFS Time: %3.6lf times\n",
	       (double)totalDFSTime / ProblemNumber);
	printf("--------------------\n");
	return 0;
}
