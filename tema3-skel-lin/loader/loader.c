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
struct sigaction old_handler;
struct sigaction new_handler;

static void sig_handler(int signum, siginfo_t *info, void *context)
{
	int page_size = getpagesize();
	int i = 0;
	so_seg_t *segment;
	int *valid_pages;
	int start_page = 0, end_page = 0;
	int high_offset = 0, low_offset = 0;
	void *ret = NULL;

	for (i = 0; i < exec->segments_no; i++) {
		segment = &exec->segments[i];
		valid_pages = (int *)(segment->data);

		/* every segment has start, size and permissions */
		start_page = segment->vaddr;
		end_page = segment->vaddr + segment->mem_size;

		/* if the current address is in this segment */
		if ((int)info->si_addr >= start_page && (int)info->si_addr < end_page) {

			/* calculate page index */
			int page_index = (int)(info->si_addr - start_page) / page_size;

			/* if the page was already accessed */
			if (valid_pages[page_index] != 0) {
				break;
			}

			high_offset = (((int)info->si_addr - start_page)
					/ page_size) * page_size;


			low_offset = segment->offset + high_offset;

			if (high_offset > segment->file_size) {
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
	old_handler.sa_sigaction(signum, info, context);
}


int so_init_loader(void)
{
	/* TODO: initialize on-demand loader */

	// struct sigaction signals;
	int ret = 0;
	memset(&new_handler, 0, sizeof(struct sigaction));
	sigemptyset(&new_handler.sa_mask);

	new_handler.sa_flags = SA_RESETHAND;
	new_handler.sa_handler = (void *)sig_handler;

	ret = sigaction(SIGSEGV, &new_handler, &old_handler);
	if (ret < 0)
		fprintf(stderr, "Sigaction\n");

	return -1;
}

int so_execute(char *path, char *argv[])
{
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
			return -1;

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
