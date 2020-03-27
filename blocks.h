/*
 *	Fast block move/copy subroutines
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef JUPP_BLOCKS_H
#define JUPP_BLOCKS_H

#ifdef EXTERN_B_C
__IDSTRING(rcsid_blocks_h, "$MirOS: contrib/code/jupp/blocks.h,v 1.9 2020/03/27 06:38:55 tg Exp $");
#endif

/* char *mmove(char *d,char *s,int sz); Copy 'sz' bytes from 's' to 'd'.
 * Chooses either mbkwd or mfwrd to do this such that the data won't get
 * clobbered.
 */
void *mmove(void *d, const void *s, int sz);

/* char *mset(char *d,char c,int sz); Set 'sz' bytes at 'd' to 'c'.
 * If 'sz'==0 nothing happens
 * Return original value of 'd'
 */
unsigned char *mset(void *dest, unsigned char c, int sz);

/* int *msetI(int *d,int c,int sz); Set 'sz' ints at 'd' to 'c'.
 * If 'sz'==0 nothing happens
 * Returns orininal value of 'd'
 */
int *msetI(void *dest, int c, int sz);

/* void **msetP(void **d,void *c,int sz); Set 'sz' pointers at 'd' to 'c'.
 * If 'sz'==0 nothing happens
 * Returns orininal value of 'd'
 */
void **msetP(void **dest, void *c, int sz);

/* int mcnt(char *blk,char c,int size);
 *
 * Count the number of occurances a character appears in a block
 */
int mcnt(const unsigned char *blk, unsigned char c, int size);

#endif
