/*
 *	Key-map handler
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef JUPP_KBD_H
#define JUPP_KBD_H

#ifdef EXTERN
__IDSTRING(rcsid_kbd_h, "$MirOS: contrib/code/jupp/kbd.h,v 1.7 2020/03/27 06:38:56 tg Exp $");
#endif

/* KMAP *mkkmap(void);
 * Create an empty keymap
 */
KMAP *mkkmap(void);

/* void rmkmap(KMAP *kmap);
 * Free a key map
 */
void rmkmap(KMAP *kmap);

/* int kadd(KMAP *kmap,char *seq,void *bind);
 * Add a key sequence binding to a key map
 *
 * Returns 0 for success
 *        -1 for for invalid key sequence
 *
 * A valid key sequence is one or more keys seperated with spaces.  A key
 * is a single character or one of the following strings:
 *
 *    ^?	                   127 (DEL)
 *
 *    ^@   -   ^_                  Control characters
 *
 *    SP                           32 (space character)
 *
 *    UP, DOWN, LEFT, RIGHT,
 *    F0 - F10, DEL, INS, HOME,
 *    END, PGUP, PGDN              termcap special characters
 *
 * In addition, the last key of a key sequence may be replaced with
 * a range-fill of the form: <KEY> TO <KEY>
 *
 * So for example, if the sequence: ^K A TO Z
 * is specified, then the key sequences
 * ^K A, ^K B, ^K C, ... ^K Z are all bound.
 */
int kadd(CAP *cap, KMAP *kmap, unsigned char *seq, void *bind);

/* void kcpy(KMAP *dest,KMAP *src);
 * Copy all of the entries in the 'src' keymap into the 'dest' keymap
 */
void kcpy(KMAP *dest, KMAP *src);

/* int kdel(KMAP *kmap,char *seq);
 * Delete a binding from a keymap
 *
 * Returns 0 for success
 *        -1 if the given key sequence was invalid
 *         1 if the given key sequence did not exist
 */
int kdel(KMAP *kmap, unsigned char *seq);

/* KBD *mkkbd(KMAP *kmap);
   Create a keyboard handler which uses the given keymap
*/
KBD *mkkbd(KMAP *kmap);

/* void rmkbd(KBD *);
 *
 * Eliminate a keyboard handler
 */
void rmkbd(KBD *k);

/* void *dokey(KBD *kbd,int k);
   Handle a key for a KBD:

     Returns 0 for invalid or prefix keys

     Returns binding for a completed key sequence
*/
void *dokey(KBD *kbd, int n);

#endif
