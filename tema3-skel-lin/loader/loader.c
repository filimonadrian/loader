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
static int fd;
struct sigaction old_handler;

void write_pages(so_seg_t *segment, int start_page, int page_size,
		int page_index, int page_address, int file_offset)
{
	void *addr = NULL;
	int *valid_pages = (int *)(segment->data);

	addr = mmap((void *)start_page, page_size, PROT_WRITE,
			MAP_PRIVATE | MAP_FIXED, fd, file_offset);
	if (addr == NULL)
		return;

	if ((page_address + page_size >= segment->file_size)) {
	/* memset to */
	memset((void *)segment->vaddr + segment->file_size, 0,
		(page_index + 1) * page_size - segment->file_size);
	}

	mprotect(addr, page_size, segment->perm);
	valid_pages[page_index] = 1;
}

void map_page(so_seg_t *segment, int start_page, int page_size, int page_index)
{
	void *addr = NULL;
	int *valid_pages = (int *)(segment->data);

	addr = mmap((void *)start_page, page_size, PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	/* if mmap fails */
	if (addr == NULL)
		return;

	/* protect page with segment permissions */
	mprotect(addr, page_size, segment->perm);

	/* mark page as valid */
	valid_pages[page_index] = 1;
}

static void sig_handler(int signum, siginfo_t *info, void *context)
{
	so_seg_t *segment = NULL;
	int *valid_pages;
	int page_size = getpagesize();
	int i = 0;
	int start_page = 0, end_segment = 0;
	int page_index = 0, file_offset = 0, page_address = 0;

	for (i = 0; i < exec->segments_no; i++) {
		segment = &exec->segments[i];
		valid_pages = (int *)(segment->data);

		/* every segment has start, size and permissions 
		 * calculate the end of the segment
		 */
		end_segment = segment->vaddr + segment->mem_size;

		/* if the current address is in this segment */
		if ((int)info->si_addr >= segment->vaddr &&
			(int)info->si_addr < end_segment) {

			/* calculate page index */
			page_index = (int)(info->si_addr - segment->vaddr) /
					page_size;

			/* if the page was already mapped */
			if (valid_pages[page_index] != 0)
				break;

			/* start of page is segment_start + page_offset */
			start_page = segment->vaddr + page_index * page_size;
			/* file offset is segment_offset + page_offset */
			file_offset = segment->offset + page_index * page_size;
			/* page address were error occured */
			page_address = page_index * page_size;

			if (segment->file_size <= page_address) {
				map_page(segment, start_page,
					page_size, page_index);
				return;

			} else {
				write_pages(segment, start_page,
						page_size, page_index,
						page_address, file_offset);
				return;
			}
		}
	}

	old_handler.sa_sigaction(signum, info, context);
}


int so_init_loader(void)
{
	struct sigaction signals;
	int ret = 0;

	memset(&signals, 0, sizeof(struct sigaction));
	signals.sa_sigaction = sig_handler;
	sigemptyset(&signals.sa_mask);
	sigaddset(&signals.sa_mask, SIGSEGV);
	signals.sa_flags = SA_SIGINFO;

	ret = sigaction(SIGSEGV, &signals, &old_handler);

	return ret;
}

int so_execute(char *path, char *argv[])
{
	int i = 0;
	int page_size = 0;

	exec = so_parse_exec(path);
	page_size = getpagesize();

	if (!exec)
		return -1;

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		return -1;
	}

	/* for every segment */
	for (i = 0; i < exec->segments_no; i++) {
		so_seg_t *segment = &exec->segments[i];
		int nr_pages = 0;

		nr_pages = segment->mem_size / page_size + 1;

		segment->data = calloc(0, nr_pages * sizeof(int));
		if (segment->data == NULL)
			return -1;
	}

	so_start_exec(exec, argv);
	close(fd);

	return -1;
}
