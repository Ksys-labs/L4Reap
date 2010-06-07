#include <stdio.h>
#include <l4/sys/kdebug.h>

void fork_to_background(void);
void fork_to_background(void)
{
	printf("unimplemented: %s\n", __func__);
	enter_kdebug();
}
