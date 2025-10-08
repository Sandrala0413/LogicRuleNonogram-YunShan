#include "probsolver.h"

#define FIRST_PROPAGATE

int NonogramSolver::doSolve(int* data, int probN) {
	FILE* out;
	Board b;
	ls.load(data);
	fp.clear();

#ifdef FIRST_PROPAGATE
	double beginTime = clock();
	RLmost_init(ls, b);
	logicSolve(ls, b);
	double endTime = clock();

	ls.solvedBoard = b;
	propagationTime += (endTime - beginTime) / CLOCKS_PER_SEC;
	TotalLineSolveTime += ls.linesolvetime;

	out = fopen("log.txt", "a+");
	fprintf(out, "Propagate Pixel: %d\n", debugBoard(b));
	fprintf(out, "Propagate Time: %lf sec\n",
	        (double)(endTime - beginTime) / CLOCKS_PER_SEC);
	fclose(out);

	return 1;
#elif defined(FIRST_FP)
	RLmost_init(ls, b);
	fp2(fp, ls, b);

	ls.solvedBoard = b;

	return 1;
#elif defined(ONLY_PRPAGATE)
	double beginTime = clock();
	RLmost_init(ls, b);
	int new_pixel = debugBoard(b);
	while (new_pixel > 0) {
		memcpy(b.oldData, b.data, sizeof(b.oldData));
		logicRule(ls, b);
		new_pixel = new_pixel - debugBoard(b);
	}
	logicSolve(ls, b);
	double endTime = clock();

	propagationTime += (endTime - beginTime) / CLOCKS_PER_SEC;

	return 1;
#else
	clock_t beginTime = clock();
	RLmost_init(ls, b);
	logicSolve(ls, b);
	clock_t endTime = clock();

	propagationTime += endTime - beginTime;
#endif

	if (fp2(fp, ls, b) != SOLVED) {
		// printLog("fp2", debugBoard(b));
		finish = false;
		times = 0;
		thres = 20;
		sw = 0;
		max_depth = 0;
		MEMSET_ZERO(depth_rec);

		dfs_stack(fp, ls, b, 0);
		// printLog("dfs times", times);
	}
	pixel = debugBoard(ls.solvedBoard);

	out = fopen("log.txt", "a+");
	fprintf(out, "DFS times: %d\n", times);
	fclose(out);

	return 1;
}

void NonogramSolver::setMethod(int n) { fp.method = n; }

void NonogramSolver::dfs_stack(FullyProbe& fp, LineSolve& ls, Board b,
                               int depth) {
	if (depth > 625) {
		puts("Aborted: depth > 625");
		exit(1);
	}

	if (depth > max_depth) max_depth = depth;

	times++;

	int res = fp2(fp, ls, b);
	if (res == SOLVED) {
		finish = true;
		return;
	}

	if (res == CONFLICT) {
		return;
	}

	Board b1 = fp.max_g1;
	Board b0 = fp.max_g0;

	dfs_stack(fp, ls, b0, depth + 1);
	if (finish == true) return;

	dfs_stack(fp, ls, b1, depth + 1);
}

void NonogramSolver::dfs(FullyProbe& fp, LineSolve& ls, Board b) {
	queue[sw].push_back(b);

	while (1) {
		// cout << queue[sw].size() << endl;
		times++;
		if (queue[sw].size() == 0) sw = sw == 1 ? 0 : 1;
		Board current = queue[sw].back();
		queue[sw].pop_back();

		if (0) // times % thres == 0 )
		{
			// if(times>=1000)printf("SW! %d\n",times);
			thres *= 2;
			int nsw = sw == 1 ? 0 : 1;
			if (queue[nsw].size() != 0) {
				queue[sw].push_back(current);
				current = queue[sw].front();
				queue[sw].pop_front();
			}
			sw = nsw;
		}

#ifdef MIRROR
		if (times % 1000 == 1) {
			// puts("Fix");
			mirror t(fp, current, PATTERN_DROP_THRESHOLD);
			if (true == t.generatePattern(current, MAX_PERM)) {
				for (auto subBoard : t.pattern) queue.push_back(subBoard);
				current = queue.back();
				queue.pop_back();
			}
		}
#endif

		int res = fp2(fp, ls, current);
		if (res == SOLVED) {
			finish = true;
			return;
		}

		if (res == CONFLICT) continue;

		queue[sw].push_back(fp.max_g1);
		queue[sw].push_back(fp.max_g0);

		if (finish == true) return;
	}
}
