#include "lib.h"

void umain(int argc, char **argv)
{
	char a[]={"testfdsharing.Y"};
	char *s1[]={{"hello!"},{"world!"},{"haha"},{NULL}};
	
	spawn(a, s1);

}
