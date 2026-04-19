#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

int count_l(const char *path);

char* parent_path(const char *path, int l);

char* get_name(const char *path);

char *split(const char *path, int n);
