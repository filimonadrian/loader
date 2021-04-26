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
#include <sys/mman.h>

#include "exec_parser.h"

static so_exec_t *exec;
int fd = 0;

void find_segment_err()
{
	
}

static void sig_handler(int signum, siginfo_t *info, void *context)
{
	so_seg_t *segment = NULL;
	void *ret = NULL;
	int page_size = getpagesize();
	int i = 0;
	int *valid_pages;
	int start_page = 0, end_page = 0;
	int high_offset = 0, low_offset = 0;
	int page_index = 0;

	// return;
	for (i = 0; i < exec->segments_no; i++) {
		segment = &exec->segments[i];
		valid_pages = (int *)(segment->data);

		/* every segment has start, size and permissions */
		start_page = segment->vaddr;
		end_page = segment->vaddr + segment->mem_size;

		/* if the current address is in this segment */
		if ((int)info->si_addr >= start_page &&
			(int)info->si_addr < end_page) {

			// fprintf(stderr, "EXECUTIA %d\n", i);
			/* calculate page index */
			page_index = (int)(info->si_addr - start_page) / page_size;

			/* if the page was already accessed */
			if (valid_pages[page_index] != 0) {
				break;
			}

			high_offset = (((int)info->si_addr - start_page)
					/ page_size) * page_size;


			low_offset = segment->offset + high_offset;

			if (high_offset >= segment->file_size) {
				ret = mmap((void *)(start_page + high_offset), page_size,
					PROT_WRITE,
					MAP_PRIVATE | MAP_ANONYMOUS,
					-1, 0);

				/* if mmap fails */
				if (ret == NULL)
				return;

				/* protect page with segment permissions */
				mprotect(ret, page_size, segment->perm);
				/* mark page as valid */
				valid_pages[high_offset] = 1;
				return;

			} else {
				ret = mmap((void *)(start_page + high_offset), page_size,
					PROT_WRITE,
					MAP_PRIVATE | MAP_FIXED,
					fd, low_offset);
				if (ret == NULL)
					return;

				if ((high_offset + page_size >=
					segment->file_size)) {

					int first = segment->vaddr + segment->file_size;
					int second = ((int)(info->si_addr - segment->vaddr));
					second /= page_size;
					second += 1;
					second *= page_size;
					second -= segment->file_size;

					memset((void *)first, 0, second);
				}

				mprotect(ret, page_size, segment->perm);
				high_offset = ((int)info->si_addr -
						segment->vaddr) / page_size;
				valid_pages[high_offset] = 1;
				return;
			}
		}
	}
}


int so_init_loader(void)
{
	struct sigaction signals;
	int ret = 0;
	/* initialize on-demand loader */

	memset(&signals, 0, sizeof(struct sigaction));
	sigemptyset(&signals.sa_mask);
	sigaddset(&signals.sa_mask, SIGSEGV);

	signals.sa_flags = SA_SIGINFO;
	signals.sa_sigaction = sig_handler;

	ret = sigaction(SIGSEGV, &signals, NULL);

	return ret;
}

int so_execute(char *path, char *argv[])
{
	int i = 0, j = 0;
	int page_size = 0;

	exec = so_parse_exec(path);
	page_size = getpagesize();

	if (!exec)
		return -1;

	fd = open(path, O_RDONLY, 0644);
	if (fd < 0) {
		return -1;
	}

	// fprintf(stderr, "Segments no: %d\n", exec->segments_no);
	/* for every segment */
	for (i = 0; i < exec->segments_no; i++) {
		// fprintf(stderr, "EXECUTIA %d\n", i);
		so_seg_t *segment = &exec->segments[i];
		int nr_pages = 0;
		/* array for all pages to check if they are already mapped */
		// int *valid_pages;

		nr_pages = segment->mem_size / page_size + 1;

		segment->data = calloc (0, nr_pages * sizeof(int));
		if (segment->data == NULL)
			return -1;

		// for (j = 0; j < nr_pages; j++) {
		// 	((int*)segment->data)[j] = 0;
		// }
	}

	so_start_exec(exec, argv);

	// return -1;
	return 0;
}
