/*
 * Loader Implementation
 *
 * 2018, Operating Systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "exec_parser.h"

static so_exec_t *exec;

static void sig_handler(int signum, siginfo_t *info)
{
	int page_size = getpagesize();
	int i = 0;
	so_seg_t *segment;
	int *valid_pages;
	int start_page = 0;

	for (i = 0; i < exec.segments_no; i++) {
		segment = &exec->segments[i]
		valid_pages = (int *)(segment->data);

		/* every segment has start, size and permissions */
		start_page = segment->vaddr;

		/* if the current address is in this segment */
		if (info->si_addr >= start_page &&
			info->si_addr < start_page + segment->mem_size) {

			/* calculate page index */
			page_index = (my_info->si_addr - start_page) / page_size;

			/* if the page was already accessed */
			if (valid_pages[page_index] != 0) {
				break;
			}

			if ()











		}

		
	}



	switch (signum) {
	case SIGSEGV:
		break;
	default:
		printf("ciouciuc");
	}

	fflush(stdout);
}


int so_init_loader(void)
{
	/* TODO: initialize on-demand loader */

	struct sigaction signals;
	int ret = 0;

	memset(&signals, 0, sizeof(struct sigaction));
	sigemptyset(&signals.sa_mask);

	signals.sa_flags = SA_RESETHAND;
	signals.sa_handler = sig_handler;

	ret = sigaction(SIGSEGV, &signals, NULL);
	if (ret < 0)
		fprintf(stderr, "Sigaction\n");

	return -1;
}

int so_execute(char *path, char *argv[])
{
	int fd = 0;
	int i = 0, j = 0;
	int page_size = 0;
	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	fd = open(path, O_RDONLY, 0644);
	if (fd < 0) {
		fprintf(stderr, "Oppening file");
		return -1;
	}

	page_size = getpagesize();

	/* for every segment */
	for (i = 0; i < exec->segments_no; i++) {
		so_seg_t *segment = &exec->segments[i];
		int nr_pages = 0;
		/* array for all pages to check if there are already mapped */
		int *valid_pages;

		if (segment->data == NULL)
			return ENOMEM;

		nr_pages = segment->mem_size / page_size + 1;

		valid_pages = malloc (nr_pages * sizeof(int));
		nr_pages = segment->mem_size / page_size + 1;

		for (j = 0; j < nr_pages; j++) {
			((int*)segment->data)[j] = 0;
		}
	}
	so_start_exec(exec, argv);

	return -1;
}
