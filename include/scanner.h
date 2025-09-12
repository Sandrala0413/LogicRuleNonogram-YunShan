#ifndef SCANNER_H
#define SCANNER_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>

#include "cdef.h"

void readFile(char*, int*);
void getData(int*, int, int*);
inline void testFail(int exp, const char* s) {
	if (exp) return;
	puts(s);
	exit(1);
}

#endif
