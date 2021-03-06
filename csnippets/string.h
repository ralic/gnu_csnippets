/* Public Domain */
#ifndef _STR_MISC_H
#define _STR_MISC_H

/* See the comment in csnippets/csnippets.h  */
#include_next <string.h>

#define str_lower(str)	strconv(str, tolower)
#define str_upper(str)	strconv(str, toupper)

#define strcmp_lo(str)  strccmp(str, islower)
#define strcmp_up(str)  strccmp(str, isupper)

/* Shamelessly stolen from ccan/str.  */
#define streq(a, b) (strcmp((a), (b)) == 0)
#define strstarts(str,prefix) (strncmp((str),(prefix),strlen(prefix)) == 0)

static inline bool strends(const char *str, const char *postfix)
{
	if (strlen(str) < strlen(postfix))
		return false;

	return streq(str + strlen(str) - strlen(postfix), postfix);
}
/* End CCan's code.  */

char *strtrim(char *str);

/** Split string into tokens using a character seperator.
 *
 * Example:
 * \code
 *	char *str = "hello,world,this,is,a,random,string"
 *	int size;
 *	char **splited = strexplode(str, ',', &size);
 * \endcode
 */
char **strexplode(char *string, char seperator, int *size);

/** Wildcard string matching.
 *  Example:
 *  \code
 *	char *string = "hello_world";
 *	if (strwildmatch("_*", string))
 *		...
 *  \endcode
 *
 *  Returns: 1 on success, 0 on failure.
 */
int strwildmatch(const char *pattern, const char *string);

/* Compare each character of a string using the
 * compare function specified.  */
bool strccmp(const char *str, int (*cmp) (int));

/* Convert a string with convfun function and return
 * a malloc'd version of it.  */
char *strconv(const char *str, int (*conv) (int));

#endif /*  _STR_MISC_H */

