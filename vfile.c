/* $MirOS: contrib/code/jupp/vfile.c,v 1.9 2017/12/02 00:16:44 tg Exp $ */
/*
 *	Software virtual memory system
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <fcntl.h>
#include <limits.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <unistd.h>

#include "blocks.h"
#include "queue.h"
#include "path.h"
#include "utils.h"
#include "vfile.h"
#include "vs.h"

static VFILE vfiles = { {&vfiles, &vfiles} };	/* Known vfiles */
static VPAGE *freepages = NULL;	/* Linked list of free pages */
static VPAGE *htab[HTSIZE];	/* Hash table of page headers */
static long curvalloc = 0;	/* Amount of memory in use */
static long maxvalloc = ILIMIT;	/* Maximum allowed */
unsigned char *vbase;			/* Data first entry in vheader refers to */
VPAGE **vheaders = NULL;	/* Array of header addresses */
static int vheadsz = 0;		/* No. entries allocated to vheaders */

static unsigned int joe_random(void);

void vflsh(void)
{
	VPAGE *vp;
	VPAGE *vlowest;
	long addr;
	long last;
	VFILE *vfile;
	int x;

	for (vfile = vfiles.link.next; vfile != &vfiles; vfile = vfile->link.next) {
		last = -1;
	      loop:
		addr = LONG_MAX;
		vlowest = NULL;
		for (x = 0; x != HTSIZE; x++)
			for (vp = htab[x]; vp; vp = vp->next)
				if (vp->addr < addr && vp->addr > last && vp->vfile == vfile && (vp->addr >= vfile->size || (vp->dirty && !vp->count))) {
					addr = vp->addr;
					vlowest = vp;
				}
		if (vlowest) {
			if (!vfile->name)
				vfile->name = mktmp(NULL,
				    vfile->fd ? NULL : &vfile->fd);
			if (!vfile->fd)
				vfile->fd = open((char *)(vfile->name), O_RDWR);
			lseek(vfile->fd, addr, 0);
			if (addr + PGSIZE > vsize(vfile)) {
				joe_write(vfile->fd, vlowest->data, vsize(vfile) - addr);
				vfile->size = vsize(vfile);
			} else {
				joe_write(vfile->fd, vlowest->data, PGSIZE);
				if (addr + PGSIZE > vfile->size)
					vfile->size = addr + PGSIZE;
			}
			vlowest->dirty = 0;
			last = addr;
			goto loop;
		}
	}
}

void vflshf(VFILE *vfile)
{
	VPAGE *vp;
	VPAGE *vlowest;
	long addr;
	int x;

      loop:
	addr = LONG_MAX;
	vlowest = NULL;
	for (x = 0; x != HTSIZE; x++)
		for (vp = htab[x]; vp; vp = vp->next)
			if (vp->addr < addr && vp->dirty && vp->vfile == vfile && !vp->count) {
				addr = vp->addr;
				vlowest = vp;
			}
	if (vlowest) {
		if (!vfile->name)
			vfile->name = mktmp(NULL,
			    vfile->fd ? NULL : &vfile->fd);
		if (!vfile->fd) {
			vfile->fd = open((char *)(vfile->name), O_RDWR);
		}
		lseek(vfile->fd, addr, 0);
		if (addr + PGSIZE > vsize(vfile)) {
			joe_write(vfile->fd, vlowest->data, vsize(vfile) - addr);
			vfile->size = vsize(vfile);
		} else {
			joe_write(vfile->fd, vlowest->data, PGSIZE);
			if (addr + PGSIZE > vfile->size)
				vfile->size = addr + PGSIZE;
		}
		vlowest->dirty = 0;
		goto loop;
	}
}

static unsigned char *mema(int align, int size)
{
	unsigned char *z = (unsigned char *) joe_malloc(align + size);

	return z + align - physical(z) % align;
}

unsigned char *vlock(VFILE *vfile, unsigned long addr)
{
	VPAGE *vp, *pp;
	int x, y;
	long ofst = (addr & (PGSIZE - 1));

	addr -= ofst;

	for (vp = htab[((addr >> LPGSIZE) + (unsigned long) vfile) & (HTSIZE - 1)]; vp; vp = vp->next)
		if (vp->vfile == vfile && (unsigned long)vp->addr == addr) {
			++vp->count;
			return vp->data + ofst;
		}

	if (freepages) {
		vp = freepages;
		freepages = vp->next;
		goto gotit;
	}

	if (curvalloc + PGSIZE <= maxvalloc) {
		vp = (VPAGE *) joe_malloc(sizeof(VPAGE) * INC);
		if (vp) {
			vp->data = (unsigned char *) mema(PGSIZE, PGSIZE * INC);
			if (vp->data) {
				int q;

				curvalloc += PGSIZE * INC;
				if (!vheaders) {
					vheaders = (VPAGE **) joe_malloc((vheadsz = INC) * sizeof(VPAGE *));
					vbase = vp->data;
				} else if (physical(vp->data) < physical(vbase)) {
					VPAGE **t = vheaders;
					int amnt = (physical(vbase) - physical(vp->data)) >> LPGSIZE;

					vheaders = (VPAGE **) joe_malloc((amnt + vheadsz) * sizeof(VPAGE *));
					mmove(vheaders + amnt, t, vheadsz * sizeof(VPAGE *));
					vheadsz += amnt;
					vbase = vp->data;
					joe_free(t);
				} else if (((physical(vp->data + PGSIZE * INC) - physical(vbase)) >> LPGSIZE) > (unsigned long)vheadsz) {
					vheaders = (VPAGE **)
					    joe_realloc(vheaders, (vheadsz = (((physical(vp->data + PGSIZE * INC) - physical(vbase)) >> LPGSIZE))) * sizeof(VPAGE *));
				}
				for (q = 1; q != INC; ++q) {
					vp[q].next = freepages;
					freepages = vp + q;
					vp[q].data = vp->data + q * PGSIZE;
					vheader(vp->data + q * PGSIZE) = vp + q;
				}
				vheader(vp->data) = vp;
				goto gotit;
			}
			joe_free(vp);
			vp = NULL;
		}
	}

	for (y = HTSIZE, x = (joe_random() & (HTSIZE - 1)); y; x = ((x + 1) & (HTSIZE - 1)), --y)
		for (pp = (VPAGE *) (htab + x), vp = pp->next; vp; pp = vp, vp = vp->next)
			if (!vp->count && !vp->dirty) {
				pp->next = vp->next;
				goto gotit;
			}
	vflsh();
	for (y = HTSIZE, x = (joe_random() & (HTSIZE - 1)); y; x = ((x + 1) & (HTSIZE - 1)), --y)
		for (pp = (VPAGE *) (htab + x), vp = pp->next; vp; pp = vp, vp = vp->next)
			if (!vp->count && !vp->dirty) {
				pp->next = vp->next;
				goto gotit;
			}
	if (write(2, "vfile: out of memory\n", 21)) {}
	exit(1);

      gotit:
	vp->addr = addr;
	vp->vfile = vfile;
	vp->dirty = 0;
	vp->count = 1;
	vp->next = htab[((addr >> LPGSIZE) + (unsigned long)vfile) & (HTSIZE - 1)];
	htab[((addr >> LPGSIZE) + (unsigned long)vfile) & (HTSIZE - 1)] = vp;

	if (addr < (unsigned long)vfile->size) {
		if (!vfile->fd) {
			vfile->fd = open((char *)(vfile->name), O_RDWR);
		}
		lseek(vfile->fd, addr, 0);
		if (addr + PGSIZE > (unsigned long)vfile->size) {
			joe_read(vfile->fd, vp->data, vfile->size - addr);
			mset(vp->data + vfile->size - addr, 0, PGSIZE - (int) (vfile->size - addr));
		} else
			joe_read(vfile->fd, vp->data, PGSIZE);
	} else
		mset(vp->data, 0, PGSIZE);

	return vp->data + ofst;
}

VFILE *vtmp(void)
{
	VFILE *new = (VFILE *) joe_malloc(sizeof(VFILE));

	new->fd = 0;
	new->name = NULL;
	new->alloc = 0;
	new->size = 0;
	new->left = 0;
	new->lv = 0;
	new->vpage = NULL;
	new->flags = 1;
	new->vpage1 = NULL;
	new->addr = -1;
	return enqueb_f(VFILE, link, &vfiles, new);
}

void vclose(VFILE *vfile)
{
	VPAGE *vp, *pp;
	int x;

	if (vfile->vpage)
		vunlock(vfile->vpage);
	if (vfile->vpage1)
		vunlock(vfile->vpage1);
	if (vfile->name) {
		if (vfile->flags)
			unlink((char *)vfile->name);
		else
			vflshf(vfile);
		vsrm(vfile->name);
	}
	if (vfile->fd)
		close(vfile->fd);
	joe_free(deque_f(VFILE, link, vfile));
	for (x = 0; x != HTSIZE; x++)
		for (pp = (VPAGE *) (htab + x), vp = pp->next; vp;)
			if (vp->vfile == vfile) {
				pp->next = vp->next;
				vp->next = freepages;
				freepages = vp;
				vp = pp->next;
			} else {
				pp = vp;
				vp = vp->next;
			}
}

long my_valloc(VFILE *vfile, long int size)
{
	long start = vsize(vfile);

	vfile->alloc = start + size;
	if (vfile->lv) {
		if (vheader(vfile->vpage)->addr + PGSIZE > vfile->alloc)
			vfile->lv = PGSIZE - (vfile->alloc - vheader(vfile->vpage)->addr);
		else
			vfile->lv = 0;
	}
	return start;
}

#if ((HTSIZE) <= 0x8000)
/* Borland LCG */
static unsigned int
joe_random(void)
{
	static unsigned int lcg_state = 5381;

	return (((lcg_state = 22695477 * lcg_state + 1) >> 16) & 0x7FFF);
}
#endif
