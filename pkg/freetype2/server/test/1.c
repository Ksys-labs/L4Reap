#include <stdio.h>
#include <stdlib.h>


main()
{
    int i=0,ch=0;
    while((ch=getchar())!=EOF) {
	    printf("0x%x, ",ch);
	    if((i=(i+1)%8)==0) {
		printf("\n");
	    }
    }
}
