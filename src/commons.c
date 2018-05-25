#include "commons.h"

char * substring (char * src, int first, int last)
{
	//printf("%s\n", src);
	char * res = malloc(last - first + 2);
	int i;
	int j = 0;
	for (i = first; i <= last; i++)
	{
		res[j] = src[i];
		j++;
	}
	res[j] = '\0';
	//printf("%s\n", res);
	return res;
}
