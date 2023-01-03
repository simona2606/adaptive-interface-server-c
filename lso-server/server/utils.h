#ifndef __H_UTILS__
#define __H_UTILS__

#include <stdio.h>
#include <stdlib.h>

void str_overwrite_stdout();
void str_trim_lf (char* arr, int length);
int split (char *str, char c, char ***arr);

#endif
