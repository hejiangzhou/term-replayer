#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define BUFFER_SIZE 32768

static size_t ntime, npos;
static FILE* fp;
static int fd_char;
static volatile int stop;

static void terminate()
{
	stop = 1;
}

static int read_next_time()
{
	if (fscanf(fp, "%lu %lu", &ntime, &npos) != 2) {
		terminate();
		return -1;
	}
	return 0;
}

static void write_chars()
{
	static char buf[BUFFER_SIZE];
	static size_t buf_sz, buf_pos;
	static size_t last_pos;
	static size_t time;
	size_t len, sz;

	if (time == ntime) {
		len = npos - last_pos;
		last_pos = npos;
		while (len != 0) {
			if (buf_pos == buf_sz) {
				buf_sz = read(fd_char, buf, BUFFER_SIZE);
				if (buf_sz == 0) {
					terminate();
					return;
				}
				buf_pos = 0;
			}
			sz = buf_sz - buf_pos;
			sz = (len > sz ? sz : len);
			write(1, &buf[buf_pos], sz);
			len -= sz;
			buf_pos += sz;
		}
		read_next_time();
	}
	time++;
}

static void timer_handler(int signum)
{
	write_chars();
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
	char fn_char[len + 6];
	sigset_t set;
	int duration;
	void (*sighdl)(int);


	fp = fopen(argv[1], "r");
	sprintf(fn_char, "%s.char", argv[1]);
	fd_char = open(fn_char, O_RDONLY);
	if (fscanf(fp, "%d", &duration) != 1) {
		return 1;
	}
	read_next_time();
	sighdl = signal(SIGALRM, &timer_handler);
	set_timer(duration);

	sigemptyset(&set);
	while (!stop)
		sigsuspend(&set);
	signal(SIGALRM, sighdl);

	close(fd_char);
	fclose(fp);
	
	printf("\n==replay completed==\n");

	return 0;
}
