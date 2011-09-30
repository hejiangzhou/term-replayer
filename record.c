#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define BUFFER_SIZE 32768
#define DEFAULT_DURATION 50

static FILE* fp;
static size_t size;

static void copy(int dest_fd, int src_fd, size_t* size)
{
	char buf[BUFFER_SIZE];
	ssize_t sz;
	while ((sz = read(src_fd, buf, BUFFER_SIZE)) > 0) {
		*size += sz;
		write(dest_fd, buf, sz);
	}
}

static void note_time()
{
	static size_t time = 0;
	static size_t last_size = 0;

	if (size != last_size) {
		fprintf(fp, "%lu %lu\n", time, size);
		last_size = size;
	}
	time++;
}

static void timer_handler(int signum)
{
	note_time();
}

static void set_timer(int duration)
{
	struct itimerval tv;

	tv.it_interval.tv_sec = 0;
	tv.it_interval.tv_usec = duration * 1000;
	tv.it_value.tv_sec = 0;
	tv.it_value.tv_usec = 1;

	setitimer(ITIMER_REAL, &tv, NULL);
}

int main(int argc, char** argv)
{
	int len = strlen(argv[1]);
	int fd_char;
	char fn_char[len + 6];
	void (*sighdl)(int);

	sprintf(fn_char, "%s.char", argv[1]);
	fd_char = creat(fn_char, 0644);

	fp = fopen(argv[1], "w");
	fprintf(fp, "%u\n", DEFAULT_DURATION);

	sighdl = signal(SIGALRM, &timer_handler);
	set_timer(DEFAULT_DURATION);
	copy(fd_char, 0, &size);

	signal(SIGALRM, sighdl);
	note_time();

	fclose(fp);
	close(fd_char);

	return 0;
}

