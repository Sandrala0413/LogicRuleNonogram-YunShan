#ifndef OPTIONS_H
#define OPTIONS_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "cdef.h"

class Options {
	public:
		int problemStart;
		int problemEnd;
		int method;
		char inputFileName[512];
		char outputFileName[512];
		char logFileName[100];
		bool selfCheck;
		bool keeplog;
		bool simple;

		Options() {
			problemStart = 1;
			problemEnd = 1000; /* 原本為1000，改為1 */
			method = CH_SQRT;
			memset(inputFileName, 0, sizeof(inputFileName));
			memset(outputFileName, 0, sizeof(outputFileName));
			memset(logFileName, 0, sizeof(logFileName));
			strcpy(inputFileName, INPUT_FILE_NAME);
			strcpy(outputFileName, OUTPUT_FILE_NAME);
			strcpy(logFileName, LOG_FINLE_NAME);
			selfCheck = true;
			keeplog = false;
			simple = false;
		}

		void print(bool detail) {
			printf("Input file = [%s]\n", inputFileName);

			if (detail) {
				printf("Output file = [%s]\n", outputFileName);
				printf("Problems select from [%d] to [%d]\t", problemStart,
				       problemEnd);
				printf("Method = [%d]\n", method);
				printf("Write log = [%s]\n", keeplog ? "Yes" : "No");
				if (keeplog)
					printf("\tLog file = [%s]\n", logFileName[0] == 0
					                                  ? "log_<timestamp>.txt"
					                                  : logFileName);
				printf("Self answer check = [%s]\t", selfCheck ? "Yes" : "No");
				printf("Only Print When Each 100 Solved = [%s]\n",
				       simple ? "Yes" : "No");
			}
		}

		int readOptions(int argc, char **argv);
		/*
		 * write log info about options and return log file name
		 */
		int genLogFile();
		void printUsage(const char *name);
};

#endif
