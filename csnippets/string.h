/* Public Domain */
#ifndef _STR_MISC_H
#define _STR_MISC_H

/* See the comment in csnippets/csnippets.h  */
#include_next <string.h>

char *strtrim(char *str);
char **strexplode(char *string, char seperator, int *size);
int strwildmatch(const char *pattern, const char *string);
bool str_cmp(const char *str, int (*cmp_func) (int));
char *str_convert(const char *str, int (*convert_func) (int));

bool str_startswith(const char *str, const char *start);
bool str_endswith(const char *str, const char *end);

#endif /*  _STR_MISC_H */
