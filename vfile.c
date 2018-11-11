/*
 *	Software virtual memory system
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

__RCSID("$MirOS: contrib/code/jupp/vfile.c,v 1.17 2018/11/11 18:15:39 tg Exp $");

#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "blocks.h"
#include "queue.h"
#include "path.h"
#include "tty.h"
#include "utils.h"
#include "vfile.h"
#include "vs.h"

				/* Known vfiles */
static VFILE vfiles = { {&vfiles, &vfiles}, 0, 0, 0, 0, NULL, 0, NULL, 0, NULL, NULL, 0, 0 };
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
	const char *wtf;

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
			if (vfile->fd < 0) {
				wtf = "open";
				goto eek;
			}
			if (lseek(vfile->fd, addr, 0) < 0) {
				/* should not happen, what now? */
				wtf = "lseek";
				close(vfile->fd);
 eek:
				vfile->fd = 0;
				fprintf(stderr, "\nvfile %s failed! \n", wtf);
				continue;
			}
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

/* write changed pages for a specific file to the disk */
static void vflshf(VFILE *vfile);

static void
vflshf(VFILE *vfile)
{
	VPAGE *vp;
	VPAGE *vlowest;
	long addr;
	int x;
	const char *wtf;

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
		if (!vfile->fd)
			vfile->fd = open((char *)(vfile->name), O_RDWR);
		if (vfile->fd < 0) {
			wtf = "open";
			goto eek;
		}
		if (lseek(vfile->fd, addr, 0) < 0) {
			/* should not happen, what now? */
			wtf = "lseek";
			close(vfile->fd);
 eek:
			vfile->fd = 0;
			fprintf(stderr, "\nvfile %s failed! \n", wtf);
			/* only called from vclose via main, maybe harmless? */
			return;
		}
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
	unsigned char *z = malloc(align + size);

	return z + (align - ((size_t)z % align));
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
		vp = malloc(sizeof(VPAGE) * INC);
		if (vp) {
			vp->data = (unsigned char *) mema(PGSIZE, PGSIZE * INC);
			if (vp->data) {
				int q;

				curvalloc += PGSIZE * INC;
				if (!vheaders) {
					vheaders = malloc((vheadsz = INC) * sizeof(VPAGE *));
					vbase = vp->data;
				} else if ((size_t)vp->data < (size_t)vbase) {
					VPAGE **t = vheaders;
					int amnt = (((size_t)vbase) - ((size_t)vp->data)) >> LPGSIZE;

					vheaders = malloc((amnt + vheadsz) * sizeof(VPAGE *));
					mmove(vheaders + amnt, t, vheadsz * sizeof(VPAGE *));
					vheadsz += amnt;
					vbase = vp->data;
					free(t);
				} else if (((((size_t)vp->data + PGSIZE * INC) - ((size_t)vbase)) >> LPGSIZE) > (unsigned long)vheadsz) {
					vheaders = realloc(vheaders,
					    (vheadsz = (((((size_t)vp->data + PGSIZE * INC) - ((size_t)vbase)) >> LPGSIZE))) * sizeof(VPAGE *));
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
			free(vp);
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
		if (!vfile->fd && (vfile->fd = open((char *)(vfile->name),
		    O_RDWR)) < 0)
			vfile->fd = 0;
		if (!vfile->fd || lseek(vfile->fd, addr, 0) < 0) {
			static char washere = 0;

			if (!washere++)
				ttabrt(0, "vlock: open or lseek failed");
			if (write(2, "vlock: open or lseek failed twice\n", 26)) {}
			exit(1);
		}
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
	VFILE *new = malloc(sizeof(VFILE));

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
	free(deque_f(VFILE, link, vfile));
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
