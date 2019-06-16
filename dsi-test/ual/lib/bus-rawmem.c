/**
 * @copyright Copyright (c) 2016 CERN
 * @author: Federico Vaga <federico.vaga@cern.ch>
 * @license: LGPLv3
 */
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>

#include "ual-int.h"


/**
 * Internal raw memory descriptor
 */
struct ual_bar_rawmem {
	int fd; // dev/mem file descriptor
	void *virt_base;
};

static int ual_rawmem_map(struct ual_bar *bar)
{
	struct ual_desc_rawmem *raw = &bar->desc.rawmem;
	struct ual_bar_rawmem *braw;

	braw = bar->bus_data;
	if (!braw) {
		errno = UAL_ERR_NOT_OPEN;
		return -1;
	}

	if (braw->fd >= 0) {
		errno = UAL_ERR_ALREADY_MAPPED;
		return -1;
	}


    braw->fd = open("/dev/mem", O_RDWR | O_SYNC);

	if(braw->fd < 0)
		return -1;


	if (raw->size & (getpagesize()-1)) {
		errno = UAL_ERR_INVALID_SIZE;
		return -1;
	}

	if (raw->offset & (getpagesize()-1)) {
		errno = UAL_ERR_INVALID_OFFSET;
		return -1;
	}
        bar->ptr = mmap(NULL, raw->size, PROT_READ | PROT_WRITE,
			MAP_SHARED, braw->fd, raw->offset);

	if ((long)bar->ptr == -1) {
		close(bar->fd);
	        braw->fd = -1;
		return -1;
	}

	return 0;
}

static int ual_rawmem_unmap(struct ual_bar *bar)
{
	struct ual_desc_rawmem *raw = &bar->desc.rawmem;
	struct ual_bar_rawmem *braw = bar->bus_data;

	if (!braw) {
		errno = UAL_ERR_NOT_OPEN;
		return -1;
	}

	if (braw->fd < 0) {
		errno = UAL_ERR_NOT_MAPPED;
		return -1;
	}

	munmap(bar->ptr, raw->size);
	close(braw->fd);
	braw->fd = -1;

	return 0;
}


static int ual_rawmem_open(struct ual_bar *bar)
{
	struct ual_bar_rawmem *braw;

	braw = malloc(sizeof(struct ual_bar_rawmem));
	if (!braw)
		return -1;
	braw->fd = -1;
	bar->bus_data = braw;

    return 0;
}

static int ual_rawmem_close(struct ual_bar *bar)
{
	struct ual_bar_rawmem *braw = bar->bus_data;

	if (!braw) {
		errno = UAL_ERR_NOT_OPEN;
		return -1;
	}

	free(braw);
	bar->bus_data = NULL;

	return 0;
}



static struct ual_bus_operations ual_rawmem_op = {
	.open = ual_rawmem_open,
	.close = ual_rawmem_close,
	.map = ual_rawmem_map,
	.unmap = ual_rawmem_unmap,
};

struct ual_bus ual_rawmem = {
	.name = "RawMem",
	.type = UAL_BUS_RAWMEM,
	.op = &ual_rawmem_op,
};
