/*
 *	Editor engine
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_B_H
#define _JOE_B_H 1

#ifdef EXTERN
__IDSTRING(rcsid_b_h, "$MirOS: contrib/code/jupp/b.h,v 1.10 2018/01/06 00:28:30 tg Exp $");
#endif

extern unsigned char stdbuf[stdsiz];

extern int force;		/* Set to have final '\n' added to file */
extern int tabwidth;		/* Default tab width */

extern VFILE *vmem;		/* Virtual memory file used for buffer system */

extern const unsigned char *msgs[];

B *bmk(B *prop);
void brm(B *b);

B *bfind(const unsigned char *s);
B *bfind_scratch(const unsigned char *s);
B *bcheck_loaded(const unsigned char *s);
B *bfind_reload(const unsigned char *s);

P *pdup(P *p);
P *pdupown(P *p, P **o);
P *poffline(P *p);
P *ponline(P *p);
B *bonline(B *b);
B *boffline(B *b);

void prm(P *p);
P *pset(P *n, P *p);

P *p_goto_bof(P *p);		/* move cursor to begging of file */
P *p_goto_eof(P *p);		/* move cursor to end of file */
P *p_goto_bol(P *p);		/* move cursor to begging of line */
P *p_goto_eol(P *p);		/* move cursor to end of line */

P *p_goto_indent(P *p,int c);	/* move cursor to indentation point */

int pisbof(P *p);
int piseof(P *p);
int piseol(P *p);
int pisbol(P *p);
int pisbow(P *p);
int piseow(P *p);

#define piscol(p) ((p)->valcol ? (p)->col : (pfcol(p), (p)->col))

int pisblank(P *p);
int piseolblank(P *p);

long pisindent(P *p);
int pispure(P *p,int c);

int pnext(P *p);
int pprev(P *p);

int pgetb(P *p);
int prgetb(P *p);

int pgetc(P *p);
int prgetc(P *p);

P *pgoto(P *p, long int loc);
P *pfwrd(P *p, long int n);
P *pbkwd(P *p, long int n);

P *pfcol(P *p);

P *pnextl(P *p);
P *pprevl(P *p);

P *pline(P *p, long int line);

P *pcolwse(P *p, long int goalcol);
P *pcol(P *p, long int goalcol);
P *pcoli(P *p, long int goalcol);
void pbackws(P *p);
void pfill(P *p, long int to, int usetabs);

P *pfind(P *p, unsigned char *s, int len);
P *pifind(P *p, unsigned char *s, int len);
P *prfind(P *p, unsigned char *s, int len);
P *prifind(P *p, unsigned char *s, int len);

/* copy text between 'from' and 'to' into new buffer */
B *bcpy(P *from, P *to);

void pcoalesce(P *p);

void bdel(P *from, P *to);

/* insert buffer 'b' into another at 'p' */
P *binsb(P *p, B *b);
/* insert a block 'blk' of size 'amnt' into buffer at 'p' */
P *binsm(P *p, const unsigned char *blk, int amnt);

/* insert character 'c' into buffer at 'p' */
P *binsc(P *p, int c);

/* insert byte 'c' into buffer at at 'p' */
P *binsbyte(P *p, unsigned char c);

/* insert zero term. string 's' into buffer at 'p' */
P *binss(P *p, unsigned char *s);

/* B *bload(char *s);
 * Load a file into a new buffer
 *
 * Returns with errno set to 0 for success,
 * -1 for new file (file doesn't exist)
 * -2 for read error
 * -3 for seek error
 * -4 for open error
 */
B *bload(const unsigned char *s);
B *bread(int fi, long int max);
B *borphan(void);

/* Save 'size' bytes beginning at 'p' into file with name in 's' */
int bsave(P *p, unsigned char *s, long int size,int flag);
int bsavefd(P *p, int fd, long int size);

unsigned char *parsens(const unsigned char *s, long int *skip, long int *amnt);

/* Get byte at pointer or return NO_MORE_DATA if pointer is at end of buffer */
int brc(P *p);

/* Get character at pointer or return NO_MORE_DATA if pointer is at end of buffer */
int brch(P *p);

/* Copy 'size' bytes from a buffer beginning at p into block 'blk' */
unsigned char *brmem(P *p, unsigned char *blk, int size);

/* Copy 'size' bytes from a buffer beginning at p into a zero-terminated
 * C-string in an malloc block.
 */
unsigned char *brs(P *p, int size);

/* Copy 'size' bytes from a buffer beginning at p into a variable length string. */
unsigned char *brvs(P *p, int size);

/* Copy line into buffer.  Maximum of size bytes will be copied.  Buffer needs
   to be one bigger for NIL */
unsigned char *brzs(P *p, unsigned char *buf, int size);

B *bnext(void);
B *bprev(void);

#define error berror
extern int berror;

unsigned char **getbufs(void);

#endif
