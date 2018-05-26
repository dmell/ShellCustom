/*

definition of substring function, used in various point of the program

*/


#include "commons.h"


// return a substring of the source string considering the given indexes
char * substring (const char * src, int first, int last)
{
	char * res = malloc(last - first + 2);   // allocate the new string

	int i;
	int j = 0;
	for (i = first; i <= last; i++)
	{
		res[j] = src[i];  // copy the wanted characters
		j++;
	}
	res[j] = '\0';

	return res;
}
