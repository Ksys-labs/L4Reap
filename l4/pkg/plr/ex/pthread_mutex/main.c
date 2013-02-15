#include <stdio.h>
#include <assert.h>
#include <sys/time.h>

#include <pthread.h>
#include <pthread-l4.h>

#include <l4/re/env.h>
#include <l4/sys/kdebug.h>


static int globalcounter;
pthread_mutex_t mtx;

static const unsigned print_iterations = 100;
static const unsigned inc_iterations   = 1000;

static
void *thread(void *data)
{
	(void)data;
#if 0
	while (1) {
#else
	for (unsigned cnt = 0; cnt < print_iterations; ++cnt) {
#endif
		for (unsigned i = 0; i < inc_iterations; ++i) {
			pthread_mutex_lock(&mtx);
			globalcounter++;
			pthread_mutex_unlock(&mtx);
		}

		pthread_mutex_lock(&mtx);
		printf("\033[31mThread: %d\n", globalcounter);
		pthread_mutex_unlock(&mtx);
	}
	return NULL;
}


static int diff_ms(struct timeval *t1, struct timeval *t2)
{
	return (((t1->tv_sec - t2->tv_sec) * 1000000) + 
	        (t1->tv_usec - t2->tv_usec))/1000;
}


int main(int argc, char **argv)
{
	(void)argc; (void)argv;

	pthread_t pt;

	struct timeval start, stop;

	gettimeofday(&start, NULL);

	pthread_mutex_init(&mtx, 0);

	int res = pthread_create(&pt, NULL, thread, NULL);
	assert(res == 0);

#if 0
	while (1) {
#else
	for (unsigned cnt = 0; cnt < print_iterations; ++cnt) {
#endif
		for (unsigned i = 0; i < inc_iterations; ++i) {
			pthread_mutex_lock(&mtx);
			globalcounter++;
			pthread_mutex_unlock(&mtx);
		}

		pthread_mutex_lock(&mtx);
		printf("\033[32mMain: %d\n", globalcounter);
		pthread_mutex_unlock(&mtx);
	}

	pthread_join(pt, NULL);

	gettimeofday(&stop, NULL);

	unsigned long ms = diff_ms(&stop, &start);
	printf("Start %ld.%ld --- Stop %ld.%ld --- Diff %ld.%03ld\n",
	       start.tv_sec, start.tv_usec, stop.tv_sec, stop.tv_usec,
	       ms / 1000, ms % 1000);

	//enter_kdebug("before return");

	return 0;
}
