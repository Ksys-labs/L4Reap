#include <stdio.h>
#include <assert.h>

#include <pthread.h>
#include <pthread-l4.h>

#include <l4/re/env.h>
#include <l4/sys/kdebug.h>


static int globalcounter;
pthread_mutex_t mtx;

static
void *thread(void *data)
{
	(void)data;
	while (1) {
		for (unsigned i = 0; i < 10000; ++i) {
			pthread_mutex_lock(&mtx);
			globalcounter++;
			pthread_mutex_unlock(&mtx);
		}
		printf("\033[31mThread: %d\n", globalcounter);
	}
	return NULL;
}

int main(int argc, char **argv)
{
	(void)argc; (void)argv;

	pthread_t pt;
	pthread_mutex_init(&mtx, 0);

	int res = pthread_create(&pt, NULL, thread, NULL);
	assert(res == 0);

	while (1) {
		for (unsigned i = 0; i < 10000; ++i) {
			pthread_mutex_lock(&mtx);
			globalcounter++;
			pthread_mutex_unlock(&mtx);
		}
		printf("\033[32mMain: %d\n", globalcounter);
	}

	pthread_join(pt, NULL);

	enter_kdebug("before return");

	return 0;
}
