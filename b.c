/* Editor engine
   Copyright (C) 1992 Joseph H. Allen

This file is part of JOE (Joe's Own Editor)

JOE is free software; you can redistribute it and/or modify it under the 
terms of the GNU General Public License as published by the Free Software 
Foundation; either version 1, or (at your option) any later version.  

JOE is distributed in the hope that it will be useful, but WITHOUT ANY 
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more 
details.  

You should have received a copy of the GNU General Public License along with 
JOE; see the file COPYING.  If not, write to the Free Software Foundation, 
675 Mass Ave, Cambridge, MA 02139, USA.  */ 

#include <stdio.h>
#ifndef __MSDOS__
#include <pwd.h>
#endif
#include <errno.h>

#include "config.h"
#include "blocks.h"
#include "undo.h"
#include "vs.h"
#include "va.h"
#include "zstr.h"
#include "path.h"
#include "w.h"
#include "tty.h"
#include "scrn.h"
#include "main.h"
#include "bw.h"
#include "uerror.h"

#include "b.h"

char stdbuf[stdsiz];

extern int errno;

int error;
int force=0;
VFILE *vmem;

char *msgs[]=
 { 
 "Error writing file",
 "Error opening file",
 "Error seeking file",
 "Error reading file",
 "New File"
 };

/* Get size of gap (amount of free space) */

#define GGAPSZ(hdr) ((hdr)->ehole-(hdr)->hole)

/* Get number of characters in gap buffer */

#define GSIZE(hdr) (SEGSIZ-GGAPSZ(hdr))

/* Set position of gap */

static void gstgap(hdr,ptr,ofst)
H *hdr;
char *ptr;
int ofst;
 {
 if(ofst>hdr->hole)
  mfwrd(ptr+hdr->hole,ptr+hdr->ehole,ofst-hdr->hole), vchanged(ptr);
 else if(ofst<hdr->hole)
  mbkwd(ptr+hdr->ehole-(hdr->hole-ofst),ptr+ofst,hdr->hole-ofst), vchanged(ptr);
 hdr->ehole=ofst+hdr->ehole-hdr->hole;
 hdr->hole=ofst;
 }

/* Insert a block */

static void ginsm(hdr,ptr,ofst,blk,size)
H *hdr;
char *ptr;
int ofst;
char *blk;
int size;
 {
 if(ofst!=hdr->hole) gstgap(hdr,ptr,ofst);
 mcpy(ptr+hdr->hole,blk,size);
 hdr->hole+=size;
 vchanged(ptr);
 }

/* Read block */

static void grmem(hdr,ptr,ofst,blk,size)
H *hdr;
char *ptr;
int ofst;
char *blk;
int size;
 {
 if(ofst<hdr->hole)
  if(size>hdr->hole-ofst)
   mcpy(blk,ptr+ofst,hdr->hole-ofst),
   mcpy(blk+hdr->hole-ofst,ptr+hdr->ehole,size-(hdr->hole-ofst));
  else mcpy(blk,ptr+ofst,size);
 else mcpy(blk,ptr+ofst+hdr->ehole-hdr->hole,size);
 }

/* Header allocation */

static H nhdrs={{&nhdrs,&nhdrs}};
static H ohdrs={{&ohdrs,&ohdrs}};

static H *halloc()
 {
 H *h;
 if(qempty(H,link,&ohdrs))
  {
  h=(H *)alitem(&nhdrs,sizeof(H));
  h->seg=valloc(vmem,(long)SEGSIZ);
  }
 else h=deque(H,link,ohdrs.link.next);
 h->hole=0;
 h->ehole=SEGSIZ;
 h->nlines=0;
 izque(H,link,h);
 return h;
 }

static void hfree(h)
H *h;
 {
 enquef(H,link,&ohdrs,h);
 }

static void hfreechn(h)
H *h;
 {
 splicef(H,link,&ohdrs,h);
 }

/* Pointer allocation */

static P frptrs={{&frptrs,&frptrs}};

static P *palloc()
 {
 return alitem(&frptrs,sizeof(P));
 }

static void pfree(p)
P *p;
 {
 enquef(P,link,&frptrs,p);
 }

/* Doubly linked list of buffers and free buffer structures */

static B bufs={{&bufs,&bufs}};
static B frebufs={{&frebufs,&frebufs}};

B *bnext()
 {
 B *b;
 do
  {
  b=bufs.link.prev;
  deque(B,link,&bufs);
  enqueb(B,link,b,&bufs);
  }
  while(b->internal);
 return b;
 }

B *bprev()
 {
 B *b;
 do
  {
  b=bufs.link.next;
  deque(B,link,&bufs);
  enquef(B,link,b,&bufs);
  }
  while(b->internal);
 return b;
 }

/* Make a buffer out of a chain */

static B *bmkchn(chn,prop,amnt,nlines)
H *chn;
B *prop;
long amnt, nlines;
 {
 B *b=alitem(&frebufs,sizeof(B));
 b->undo=undomk(b);
 if(prop) b->o=prop->o;
 else b->o=pdefault;
 mset(b->marks,0,sizeof(b->marks));
 b->rdonly=0;
 b->orphan=0;
 b->oldcur=0;
 b->oldtop=0;
 b->backup=1;
 b->internal=1;
 b->changed=0;
 b->count=1;
 b->name=0;
 b->er= -3;
 b->bof=palloc(); izque(P,link,b->bof);
 b->bof->end=0;
 b->bof->b=b;
 b->bof->owner=0;
 b->bof->hdr=chn;
 b->bof->ptr=vlock(vmem,b->bof->hdr->seg);
 b->bof->ofst=0;
 b->bof->byte=0;
 b->bof->line=0;
 b->bof->col=0;
 b->bof->xcol=0;
 b->bof->valcol=1;
 b->eof=pdup(b->bof);
 b->eof->end=1;
 vunlock(b->eof->ptr);
 b->eof->hdr=chn->link.prev;
 b->eof->ptr=vlock(vmem,b->eof->hdr->seg);
 b->eof->ofst=GSIZE(b->eof->hdr);
 b->eof->byte=amnt;
 b->eof->line=nlines;
 b->eof->valcol=0;
 enquef(B,link,&bufs,b);
 pcoalesce(b->bof);
 pcoalesce(b->eof);
 return b;
 }

/* Create an empty buffer */

B *bmk(prop)
B *prop;
 {
 return bmkchn(halloc(),prop,0L,0L);
 }

/* Eliminate a buffer */

extern B *errbuf;

void brm(b)
B *b;
 {
 if(b && !--b->count)
  {
  if(b->changed) abrerr(b->name);
  if(b==errbuf) errbuf=0;
  if(b->undo) undorm(b->undo);
  hfreechn(b->eof->hdr);
  while(!qempty(P,link,b->bof)) prm(b->bof->link.next);
  prm(b->bof);
  if(b->name) free(b->name);
  demote(B,link,&frebufs,b);
  }
 }

P *poffline(p)
P *p;
 {
 if(p->ptr)
  {
  vunlock(p->ptr);
  p->ptr=0;
  }
 return p;
 }

P *ponline(p)
P *p;
 {
 if(!p->ptr) p->ptr=vlock(vmem,p->hdr->seg);
 return p;
 }

B *boffline(b)
B *b;
 {
 P *p=b->bof;
 do poffline(p); while((p=p->link.next)!=b->bof);
 return b;
 }

B *bonline(b)
B *b;
 {
 P *p=b->bof;
 do ponline(p); while((p=p->link.next)!=b->bof);
 return b;
 }

P *pdup(p)
P *p;
 {
 P *n=palloc();
 n->end=0;
 n->ptr=0;
 n->owner=0;
 enquef(P,link,p,n);
 return pset(n,p);
 }

P *pdupown(p,o)
P *p;
P **o;
 {
 P *n=palloc();
 n->end=0;
 n->ptr=0;
 n->owner=o;
 enquef(P,link,p,n);
 pset(n,p);
 if(*o) prm(*o);
 *o=n;
 return n;
 }

void prm(p)
P *p;
 {
 if(!p) return;
 if(p->owner) *p->owner=0;
 if(p->ptr) vunlock(p->ptr);
 pfree(deque(P,link,p));
 }

P *pset(n,p)
P *n, *p;
 {
 if(n!=p)
  {
  n->b=p->b;
  n->ofst=p->ofst;
  n->hdr=p->hdr;
  if(n->ptr) vunlock(n->ptr);
  if(p->ptr) { n->ptr=p->ptr; vupcount(n->ptr); }
  else n->ptr=vlock(vmem,n->hdr->seg);
  n->byte=p->byte;
  n->line=p->line;
  n->col=p->col;
  n->valcol=p->valcol;
  }
 return n;
 }

P *pbof(p)
P *p;
 {
 return pset(p,p->b->bof);
 }

P *peof(p)
P *p;
 {
 return pset(p,p->b->eof);
 }

int pisbof(p)
P *p;
 {
 return p->hdr==p->b->bof->hdr && !p->ofst;
 }

int piseof(p)
P *p;
 {
 return p->ofst==GSIZE(p->hdr);
 }

int piseol(p)
P *p;
 {
 int c;
 if(piseof(p)) return 1;
 c=brc(p);
 if(c=='\n') return 1;
 if(p->b->o.crlf)
  if(c=='\r')
   {
   P *q=pdup(p);
   pfwrd(q,1L);
   if(pgetc(q)=='\n')
    {
    prm(q);
    return 1;
    }
   else prm(q);
   }
 return 0;
 }

int pisbol(p)
P *p;
 {
 char c;
 if(pisbof(p)) return 1;
 c=prgetc(p); pgetc(p);
 return c=='\n';
 }

int pisbow(p)
P *p;
 {
 P *q=pdup(p);
 int c=brc(p);
 int d=prgetc(q);
 prm(q);
 if(crest(c) && !crest(d)) return 1;
 else return 0;
 }

int piseow(p)
P *p;
 {
 P *q=pdup(p);
 int d=brc(q);
 int c=prgetc(q);
 prm(q);
 if(crest(c) && !crest(d)) return 1;
 else return 0;
 }

int pisblank(p)
P *p;
 {
 P *q=pdup(p);
 pbol(q);
 while(cwhite(brc(q))) pgetc(q);
 if(piseol(q))
  {
  prm(q);
  return 1;
  }
 else
  {
  prm(q);
  return 0;
  }
 }

long pisindent(p)
P *p;
 {
 P *q=pdup(p);
 long col;
 pbol(q);
 while(cwhite(brc(q))) pgetc(q);
 col=q->col;
 prm(q);
 return col;
 }

int pnext(p)
P *p;
 {
 if(p->hdr==p->b->eof->hdr)
  {
  p->ofst=GSIZE(p->hdr);
  return 0;
  }
 p->hdr=p->hdr->link.next; p->ofst=0;
 vunlock(p->ptr); p->ptr=vlock(vmem,p->hdr->seg);
 return 1;
 }

int pprev(p)
P *p;
 {
 if(p->hdr==p->b->bof->hdr)
  {
  p->ofst=0;
  return 0;
  }
 p->hdr=p->hdr->link.prev;
 p->ofst=GSIZE(p->hdr);
 vunlock(p->ptr); p->ptr=vlock(vmem,p->hdr->seg);
 return 1;
 }

int pgetc(p)
P *p;
 {
 char c;
 if(p->ofst==GSIZE(p->hdr)) return MAXINT;
 if(p->ofst>=p->hdr->hole) c=p->ptr[p->ofst+p->hdr->ehole-p->hdr->hole];
 else c=p->ptr[p->ofst];
 if(++p->ofst==GSIZE(p->hdr)) pnext(p); 
 ++p->byte;
 if(c=='\n') ++p->line, p->col=0, p->valcol=1;
 else if(p->b->o.crlf && c=='\r')
  {
  if(brc(p)=='\n') return pgetc(p);
  else ++p->col;
  }
 else
  {
  if(c=='\t') p->col+=p->b->o.tab-p->col%p->b->o.tab;
  else ++p->col;
  }
 return c;
 }

P *pfwrd(p,n)
P *p;
long n;
 {
 if(!n) return p;
 p->valcol=0;
 do
  {
  if(p->ofst==GSIZE(p->hdr))
   do
    {
    if(!p->ofst)
     p->byte+=GSIZE(p->hdr), n-=GSIZE(p->hdr), p->line+=p->hdr->nlines;
    if(!pnext(p)) return 0;
    }
    while(n>GSIZE(p->hdr));
  if(p->ofst>=p->hdr->hole)
   { if(p->ptr[p->ofst+p->hdr->ehole-p->hdr->hole]=='\n') ++p->line; }
  else if(p->ptr[p->ofst]=='\n') ++p->line;
  ++p->byte; ++p->ofst;
  }
  while(--n);
 if(p->ofst==GSIZE(p->hdr)) pnext(p);
 return p;
 }

int prgetc1(p)
P *p;
 {
 unsigned char c;
 if(!p->ofst) if(!pprev(p)) return MAXINT;
 --p->ofst;
 if(p->ofst>=p->hdr->hole) c=p->ptr[p->ofst+p->hdr->ehole-p->hdr->hole];
 else c=p->ptr[p->ofst];
 --p->byte;
 if(c=='\n') --p->line, p->valcol=0;
 else
  {
  if(c=='\t') p->valcol=0;
  --p->col;
  }
 return c;
 }

int prgetc(p)
P *p;
 {
 int c=prgetc1(p);
 if(p->b->o.crlf && c=='\n')
  {
  c=prgetc1(p);
  if(c=='\r') return '\n';
  if(c!=MAXINT) pgetc(p);
  c='\n';
  }
 return c;
 }

P *pbkwd(p,n)
P *p;
long n;
 {
 if(!n) return p;
 p->valcol=0;
 do
  {
  if(!p->ofst)
   do
    {
    if(p->ofst)
     p->byte-=p->ofst, n-=p->ofst, p->line-=p->hdr->nlines;
    if(!pprev(p)) return 0;
    }
    while(n>GSIZE(p->hdr));
  --p->ofst; --p->byte;
  if(p->ofst>=p->hdr->hole)
   { if(p->ptr[p->ofst+p->hdr->ehole-p->hdr->hole]=='\n') --p->line; }
  else if(p->ptr[p->ofst]=='\n') --p->line;
  }
  while(--n);
 return p;
 }

P *pgoto(p,loc)
P *p;
long loc;
 {
 if(loc>p->byte) pfwrd(p,loc-p->byte);
 else if(loc<p->byte) pbkwd(p,p->byte-loc);
 return p;
 }

P *pfcol(p)
P *p;
 {
 H *hdr=p->hdr;
 int ofst=p->ofst;
 pbol(p);
 while(p->ofst!=ofst || p->hdr!=hdr) pgetc(p);
 return p;
 }

P *pbol(p)
P *p;
 {
 if(pprevl(p)) pgetc(p);
 p->col=0; p->valcol=1;
 return p;
 }

P *peol(p)
P *p;
 {
 if(p->b->o.crlf)
  while(!piseol(p)) pgetc(p);
 else
  while(p->ofst!=GSIZE(p->hdr))
   {
   unsigned char c;
   if(p->ofst>=p->hdr->hole) c=p->ptr[p->ofst+p->hdr->ehole-p->hdr->hole];
   else c=p->ptr[p->ofst];
   if(c=='\n') break;
   else
    {
    ++p->byte;
    ++p->ofst;
    if(c=='\t') p->col+=p->b->o.tab-p->col%p->b->o.tab;
    else ++p->col;
    if(p->ofst==GSIZE(p->hdr)) pnext(p); 
    }
   }
 return p;
 }

P *pnextl(p)
P *p;
 {
 char c;
 do
  {
  if(p->ofst==GSIZE(p->hdr))
   do
    {
    p->byte+=GSIZE(p->hdr)-p->ofst;
    if(!pnext(p)) return 0;
    }
    while(!p->hdr->nlines);
  if(p->ofst>=p->hdr->hole) c=p->ptr[p->ofst+p->hdr->ehole-p->hdr->hole];
  else c=p->ptr[p->ofst];
  ++p->byte; ++p->ofst;
  }
  while(c!='\n');
 ++p->line;
 p->col=0; p->valcol=1;
 if(p->ofst==GSIZE(p->hdr)) pnext(p);
 return p;
 }

P *pprevl(p)
P *p;
 {
 char c;
 p->valcol=0;
 do
  {
  if(!p->ofst)
   do
    {
    p->byte-=p->ofst;
    if(!pprev(p)) return 0;
    }
    while(!p->hdr->nlines);
  --p->ofst; --p->byte;
  if(p->ofst>=p->hdr->hole) c=p->ptr[p->ofst+p->hdr->ehole-p->hdr->hole];
  else c=p->ptr[p->ofst];
  }
  while(c!='\n');
 --p->line;
 if(p->b->o.crlf && c=='\n')
  {
  int k=prgetc1(p);
  if(k!='\r' && k!=MAXINT) pgetc(p);
  }
 return p;
 }

P *pline(p,line)
P *p;
long line;
 {
 if(line>p->b->eof->line) { pset(p,p->b->eof); return p; }
 if(line<Labs(p->line-line)) pset(p,p->b->bof);
 if(Labs(p->b->eof->line-line)<Labs(p->line-line)) pset(p,p->b->eof);
 if(p->line==line) { pbol(p); return p; }
 while(line>p->line) pnextl(p);
 if(line<p->line)
  {
  while(line<p->line) pprevl(p);
  pbol(p);
  }
 return p;
 }

P *pcol(p,goalcol)
P *p;
long goalcol;
 {
 pbol(p);
 do
  {
  unsigned char c;
  int wid;
  if(p->ofst==GSIZE(p->hdr)) break;
  if(p->ofst>=p->hdr->hole) c=p->ptr[p->ofst+p->hdr->ehole-p->hdr->hole];
  else c=p->ptr[p->ofst];
  if(c=='\n') break;
  if(p->b->o.crlf && c=='\r' && piseol(p)) break;
  if(c=='\t') wid=p->b->o.tab-p->col%p->b->o.tab;
  else wid=1;
  if(p->col+wid>goalcol) break;
  if(++p->ofst==GSIZE(p->hdr)) pnext(p); 
  ++p->byte; p->col+=wid;
  } while(p->col!=goalcol);
 return p;
 }

P *pcolwse(p,goalcol)
P *p;
long goalcol;
 {
 int c;
 pcol(p,goalcol);
 do c=prgetc(p); while(c==' ' || c=='\t');
 if(c!=MAXINT) pgetc(p);
 return p;
 }

P *pcoli(p,goalcol)
P *p;
long goalcol;
 {
 pbol(p);
 while(p->col<goalcol)
  {
  unsigned char c;
  if(p->ofst==GSIZE(p->hdr)) break;
  if(p->ofst>=p->hdr->hole) c=p->ptr[p->ofst+p->hdr->ehole-p->hdr->hole];
  else c=p->ptr[p->ofst];
  if(c=='\n') break;
#ifdef __MSDOS
  if(c=='\r' && piseol(p)) break;
#endif
  else if(c=='\t') p->col+=p->b->o.tab-p->col%p->b->o.tab;
  else ++p->col;
  if(++p->ofst==GSIZE(p->hdr)) pnext(p); 
  ++p->byte;
  }
 return p;
 }

void pfill(p,to,usetabs)
P *p;
long to;
 {
 piscol(p);
 if(usetabs)
  while(p->col<to)
   if(p->col+p->b->o.tab-p->col%p->b->o.tab<=to) binsc(p,'\t'), pgetc(p);
   else binsc(p,' '), pgetc(p);
 else while(p->col<to) binsc(p,' '), pgetc(p);
 }

void pbackws(p)
P *p;
 {
 int c;
 P *q=pdup(p);
 do c=prgetc(q); while(c==' ' || c=='\t');
 if(c!=MAXINT) pgetc(q);
 bdel(q,p);
 prm(q);
 }

static char frgetc(p)
P *p;
 {
 if(!p->ofst) pprev(p);
 --p->ofst;
 if(p->ofst>=p->hdr->hole) return p->ptr[p->ofst+p->hdr->ehole-p->hdr->hole];
 else return p->ptr[p->ofst];
 }

static void ffwrd(p,n)
P *p;
 {
 while(n>GSIZE(p->hdr)-p->ofst)
  {
  n-=GSIZE(p->hdr)-p->ofst;
  if(!pnext(p)) return;
  }
 if((p->ofst+=n)==GSIZE(p->hdr)) pnext(p);
 }

static P *ffind(p,s,len)
P *p;
unsigned char *s;
 {
 long amnt=p->b->eof->byte-p->byte;
 int x;
 unsigned char table[256], c;
 if(len>amnt) return 0;
 if(!len) return p;
 p->valcol=0;
 mset(table,255,256); for(x=0;x!=len-1;++x) table[s[x]]=x;
 ffwrd(p,len); amnt-=len; x=len; do
  if((c=frgetc(p))!=s[--x])
   {
   if(table[c]==255) ffwrd(p,len+1), amnt-=x+1;
   else if(x<=table[c]) ffwrd(p,len-x+1), --amnt;
   else ffwrd(p,len-table[c]), amnt-=x-table[c];
   if(amnt<0) return 0;
   else x=len;
   }
  while(x);
 return p;
 }

static P *fifind(p,s,len)
P *p;
unsigned char *s;
 {
 long amnt=p->b->eof->byte-p->byte;
 int x;
 unsigned char table[256], c;
 if(len>amnt) return 0;
 if(!len) return p;
 p->valcol=0;
 mset(table,255,256); for(x=0;x!=len-1;++x) table[s[x]]=x;
 ffwrd(p,len); amnt-=len; x=len; do
  if((c=toup(frgetc(p)))!=s[--x])
   {
   if(table[c]==255) ffwrd(p,len+1), amnt-=x+1;
   else if(x<=table[c]) ffwrd(p,len-x+1), --amnt;
   else ffwrd(p,len-table[c]), amnt-=x-table[c];
   if(amnt<0) return 0;
   else x=len;
   }
  while(x);
 return p;
 }

static P *getto(p,q)
P *p, *q;
 {
 while(p->hdr!=q->hdr || p->ofst!=q->ofst)
  {
  if(p->ofst>=p->hdr->hole)
   { if(p->ptr[p->ofst+p->hdr->ehole-p->hdr->hole]=='\n') ++p->line; }
  else if(p->ptr[p->ofst]=='\n') ++p->line;
  ++p->byte; ++p->ofst;
  if(p->ofst==GSIZE(p->hdr)) pnext(p);
  while(!p->ofst && p->hdr!=q->hdr)
   {
   p->byte+=GSIZE(p->hdr), p->line+=p->hdr->nlines;
   pnext(p);
   }
  }
 return p;
 }

P *pfind(p,s,len)
P *p;
char *s;
 {
 P *q=pdup(p);
 if(ffind(q,s,len)) { getto(p,q); prm(q); return p; }
 else { prm(q); return 0; }
 }

P *pifind(p,s,len)
P *p;
char *s;
 {
 P *q=pdup(p);
 if(fifind(q,s,len)) { getto(p,q); prm(q); return p; }
 else { prm(q); return 0; }
 }

static void fbkwd(p,n)
P *p;
 {
 while(n>p->ofst)
  {
  n-=p->ofst;
  if(!pprev(p)) return;
  }
 if(p->ofst>=n) p->ofst-=n;
 else p->ofst=0;
 }

static int fpgetc(p)
P *p;
 {
 char c;
 if(p->ofst==GSIZE(p->hdr)) return MAXINT;
 if(p->ofst>=p->hdr->hole) c=p->ptr[p->ofst+p->hdr->ehole-p->hdr->hole];
 else c=p->ptr[p->ofst];
 if(++p->ofst==GSIZE(p->hdr)) pnext(p);
 return c;
 }

static P *frfind(p,s,len)
P *p;
unsigned char *s;
 {
 long amnt=p->byte;
 int x;
 unsigned char table[256], c;
 if(len>p->b->eof->byte-p->byte)
  {
  x=len-(p->b->eof->byte-p->byte);
  if(amnt<x) return 0;
  amnt-=x;
  fbkwd(p,x);
  }
 if(!len) return p;
 p->valcol=0;
 mset(table,255,256); for(x=len;--x;table[s[x]]=len-x-1);
 x=0; do
  if((c=fpgetc(p))!=s[x++])
   {
   if(table[c]==255) fbkwd(p,len+1), amnt-=len-x+1;
   else if(len-table[c]<=x) fbkwd(p,x+1), --amnt;
   else fbkwd(p,len-table[c]), amnt-=len-table[c]-x;
   if(amnt<0) return 0;
   else x=0;
   }
  while(x!=len);
 fbkwd(p,len);
 return p;
 }

static P *frifind(p,s,len)
P *p;
unsigned char *s;
 {
 long amnt=p->byte;
 int x;
 unsigned char table[256], c;
 if(len>p->b->eof->byte-p->byte)
  {
  x=len-(p->b->eof->byte-p->byte);
  if(amnt<x) return 0;
  amnt-=x;
  fbkwd(p,x);
  }
 if(!len) return p;
 p->valcol=0;
 mset(table,255,256); for(x=len;--x;table[s[x]]=len-x-1);
 x=0; do
  if((c=toup(fpgetc(p)))!=s[x++])
   {
   if(table[c]==255) fbkwd(p,len+1), amnt-=len-x+1;
   else if(len-table[c]<=x) fbkwd(p,x+1), --amnt;
   else fbkwd(p,len-table[c]), amnt-=len-table[c]-x;
   if(amnt<0) return 0;
   else x=0;
   }
  while(x!=len);
 fbkwd(p,len);
 return p;
 }

static P *rgetto(p,q)
P *p, *q;
 {
 while(p->hdr!=q->hdr || p->ofst!=q->ofst)
  {
  if(!p->ofst)
   do
    {
    if(p->ofst)
     p->byte-=p->ofst, p->line-=p->hdr->nlines;
    pprev(p);
    }
    while(p->hdr!=q->hdr);
  --p->ofst; --p->byte;
  if(p->ofst>=p->hdr->hole)
   { if(p->ptr[p->ofst+p->hdr->ehole-p->hdr->hole]=='\n') --p->line; }
  else if(p->ptr[p->ofst]=='\n') --p->line;
  }
 return p;
 }

P *prfind(p,s,len)
P *p;
char *s;
 {
 P *q=pdup(p);
 if(frfind(q,s,len)) { rgetto(p,q); prm(q); return p; }
 else { prm(q); return 0; }
 }

P *prifind(p,s,len)
P *p;
char *s;
 {
 P *q=pdup(p);
 if(frifind(q,s,len)) { rgetto(p,q); prm(q); return p; }
 else { prm(q); return 0; }
 }

B *bcpy(from,to)
P *from, *to;
 {
 H anchor, *l;
 char *ptr;
 P *q;

 if(from->byte>=to->byte) return bmk(from->b);

 q=pdup(from);
 izque(H,link,&anchor);

 if(q->hdr==to->hdr)
  {
  l=halloc(); ptr=vlock(vmem,l->seg);
  if(q->ofst!=q->hdr->hole) gstgap(q->hdr,q->ptr,q->ofst);
  l->nlines=mcnt(q->ptr+q->hdr->ehole,'\n',l->hole=to->ofst-q->ofst);
  mcpy(ptr,q->ptr+q->hdr->ehole,l->hole);
  vchanged(ptr); vunlock(ptr);
  enqueb(H,link,&anchor,l);
  }
 else
  {
  l=halloc(); ptr=vlock(vmem,l->seg);
  if(q->ofst!=q->hdr->hole) gstgap(q->hdr,q->ptr,q->ofst);
  l->nlines=mcnt(q->ptr+q->hdr->ehole,'\n',l->hole=SEGSIZ-q->hdr->ehole);
  mcpy(ptr,q->ptr+q->hdr->ehole,l->hole);
  vchanged(ptr); vunlock(ptr);
  enqueb(H,link,&anchor,l);
  pnext(q);
  while(q->hdr!=to->hdr)
   {
   l=halloc(); ptr=vlock(vmem,l->seg);
   l->nlines=q->hdr->nlines;
   mcpy(ptr,q->ptr,q->hdr->hole);
   mcpy(ptr+q->hdr->hole,q->ptr+q->hdr->ehole,SEGSIZ-q->hdr->ehole);
   l->hole=GSIZE(q->hdr);
   vchanged(ptr); vunlock(ptr);
   enqueb(H,link,&anchor,l);
   pnext(q);
   }
  if(to->ofst)
   {
   l=halloc(); ptr=vlock(vmem,l->seg);
   if(to->ofst!=to->hdr->hole) gstgap(to->hdr,to->ptr,to->ofst);
   l->nlines=mcnt(to->ptr,'\n',to->ofst);
   mcpy(ptr,to->ptr,l->hole=to->ofst);
   vchanged(ptr); vunlock(ptr);
   enqueb(H,link,&anchor,l);
   }
  }

 l=anchor.link.next;
 deque(H,link,&anchor);
 prm(q);

 return bmkchn(l,from->b,to->byte-from->byte,to->line-from->line);
 }

/* Coalesce small blocks into a single larger one */

void pcoalesce(p)
P *p;
 {
 if(p->hdr!=p->b->eof->hdr &&
    GSIZE(p->hdr)+GSIZE(p->hdr->link.next)<=SEGSIZ-SEGSIZ/4)
  {
  H *hdr=p->hdr->link.next;
  char *ptr=vlock(vmem,hdr->seg);
  int osize=GSIZE(p->hdr);
  int size=GSIZE(hdr);
  P *q;
  gstgap(hdr,ptr,size);
  ginsm(p->hdr,p->ptr,GSIZE(p->hdr),ptr,size);
  p->hdr->nlines+=hdr->nlines;
  vunlock(ptr);
  hfree(deque(H,link,hdr));
  for(q=p->link.next;q!=p;q=q->link.next)
   if(q->hdr==hdr)
    {
    q->hdr=p->hdr;
    if(q->ptr) { vunlock(q->ptr); q->ptr=vlock(vmem,q->hdr->seg); }
    q->ofst+=osize;
    }
  }
 if(p->hdr!=p->b->bof->hdr &&
    GSIZE(p->hdr)+GSIZE(p->hdr->link.prev)<=SEGSIZ-SEGSIZ/4)
  {
  H *hdr=p->hdr->link.prev;
  char *ptr=vlock(vmem,hdr->seg);
  int size=GSIZE(hdr);
  P *q;
  gstgap(hdr,ptr,size);
  ginsm(p->hdr,p->ptr,0,ptr,size);
  p->hdr->nlines+=hdr->nlines;
  vunlock(ptr);
  hfree(deque(H,link,hdr));
  p->ofst+=size;
  for(q=p->link.next;q!=p;q=q->link.next)
   if(q->hdr==hdr)
    {
    q->hdr=p->hdr;
    if(q->ptr) vunlock(q->ptr); q->ptr=vlock(vmem,q->hdr->seg);
    }
   else if(q->hdr==p->hdr) q->ofst+=size;
  }
 }

/* Delete the text between two pointers from a buffer and return it in a new
 * buffer.
 *
 * This routine calls these functions:
 *  gstgap	- to position gaps
 *  halloc	- to allocate new header/segment pairs
 *  vlock	- virtual memory routines
 *  vunlock
 *  vchanged
 *  vupcount
 *  mcpy	- to copy deleted text
 *  mcnt	- to count NLs
 *  snip	- queue routines
 *  enqueb
 *  splicef
 *  scrdel	- to tell screen update to scroll when NLs are deleted
 *  bmkchn	- to make a buffer out of a chain
 */

/* This is only to be used for bdel() */
static B *bcut(from,to)
P *from, *to;
 {
 H *h,				/* The deleted text */
   *i;
 char *ptr;
 P *p;
 long nlines;			/* No. EOLs to delete */
 long amnt;			/* No. bytes to delete */
 int toamnt;			/* Amount to delete from segment in 'to' */
 int bofmove=0;		/* Set if bof got deleted */
 
 if(!(amnt=to->byte-from->byte))
  return 0;			/* ...nothing to delete */
 
 nlines=to->line-from->line;
 
 if(from->hdr==to->hdr)
  { /* Delete is within a single segment */
  /* Move gap to deletion point */
  if(from->ofst!=from->hdr->hole)
   gstgap(from->hdr,from->ptr,from->ofst);
 
  /* Store the deleted text */
  h=halloc();
  ptr=vlock(vmem,h->seg);
  mcpy(ptr,from->ptr+from->hdr->ehole,(int)amnt);
  h->hole=amnt;
  h->nlines=nlines;
  vchanged(ptr); vunlock(ptr);
 
  /* Delete */
  from->hdr->ehole+=amnt;
  from->hdr->nlines-=nlines;
 
  toamnt=amnt;
  }
 else
  { /* Delete crosses segments */
  H *a;
  if(toamnt=to->ofst)
   {
   /* Delete beginning of to */
   /* Move gap to deletion point */
   /* To could be deleted if it's at the end of the file */
   if(to->ofst!=to->hdr->hole)
    gstgap(to->hdr,to->ptr,to->ofst);
  
   /* Save deleted text */
   i=halloc();
   ptr=vlock(vmem,i->seg);
   mcpy(ptr,to->ptr,to->hdr->hole);
   i->hole=to->hdr->hole;
   i->nlines=mcnt(to->ptr,'\n',to->hdr->hole);
   vchanged(ptr); vunlock(ptr);
 
   /* Delete */
   to->hdr->nlines-=i->nlines;
   to->hdr->hole=0;
   }
  else i=0;
 
  /* Delete end of from */
  if(!from->ofst)
   {
   /* ..unless from needs to be deleted too */
   a=from->hdr->link.prev, h=0;
   if(a==from->b->eof->hdr) bofmove=1;
   }
  else
   {
   a=from->hdr;
   /* Move gap to deletion point */
   if(from->ofst!=from->hdr->hole)
    gstgap(from->hdr,from->ptr,from->ofst);
 
   /* Save deleted text */
   h=halloc();
   ptr=vlock(vmem,h->seg);
   mcpy(ptr,from->ptr+from->hdr->ehole,SEGSIZ-from->hdr->ehole);
   h->hole=SEGSIZ-from->hdr->ehole;
   h->nlines=mcnt(ptr,'\n',h->hole);
   vchanged(ptr); vunlock(ptr);
 
   /* Delete */
   from->hdr->nlines-=h->nlines;
   from->hdr->ehole=SEGSIZ;
   }
 
  /* Make from point to header/segment of to */
  from->hdr=to->hdr;
  vunlock(from->ptr); from->ptr=to->ptr; vupcount(to->ptr);
  from->ofst=0;
 
  /* Delete headers/segments between a and to->hdr (if there are any) */
  if(a->link.next!=to->hdr)
   if(!h)
    {
    h=snip(H,link,a->link.next,to->hdr->link.prev);
    if(i) enqueb(H,link,h,i);
    }
   else
    {
    splicef(H,link,h,snip(H,link,a->link.next,to->hdr->link.prev));
    if(i) enqueb(H,link,h,i);
    }
  else
   if(!h) h=i;
   else if(i) enqueb(H,link,h,i);
  }

  /* If to is empty, then it must have been at the end of the file.  If
     the file did not become empty, delete to */
  if(!GSIZE(to->hdr) && from->byte)
   {
   H *ph=from->hdr->link.prev;
   hfree(deque(H,link,from->hdr)); vunlock(from->ptr);
   from->hdr=ph;
   from->ptr=vlock(vmem,from->hdr->seg);
   from->ofst=GSIZE(ph);
   vunlock(from->b->eof->ptr);
   from->b->eof->ptr=from->ptr;
   vupcount(from->ptr);
   from->b->eof->hdr=from->hdr;
   from->b->eof->ofst=from->ofst;
   }
 
 /* The deletion is now done */
 
 /* Scroll if necessary */
 
 if(bofmove) pset(from->b->bof,from);
 if(nlines && !pisbol(from))
  {
  scrdel(from->b,from->line,nlines,1);
  delerr(from->b->name,from->line,nlines,0);
  }
 else
  {
  scrdel(from->b,from->line,nlines,0);
  delerr(from->b->name,from->line,nlines,1);
  }
 
 /* Fix pointers */

 for(p=from->link.next;p!=from;p=p->link.next)
  if(p->line==from->line && p->byte>from->byte) p->valcol=0; 
 for(p=from->link.next;p!=from;p=p->link.next)
  if(p->byte>=from->byte)
   if(p->byte<=from->byte+amnt)
    if(p->ptr) pset(p,from);
    else poffline(pset(p,from));
   else
    {
    if(p->hdr==to->hdr) p->ofst-=toamnt;
    p->byte-=amnt;
    p->line-=nlines;
    }

 pcoalesce(from);

 /* Make buffer out of deleted text and return it */
 
 return bmkchn(h,from->b,amnt,nlines);
 }

void bdel(from,to)
P *from, *to;
 {
 if(to->byte-from->byte)
  {
  B *b=bcut(from,to);
  if(from->b->undo) undodel(from->b->undo,from->byte,b);
  else brm(b);
  from->b->changed=1;
  }
 }

/* Split a block at p's ofst */
/* p is placed in the new block such that it points to the same text but with
 * p->ofst==0
 */

static void bsplit(p)
P *p;
 {
 if(p->ofst)
  {
  H *hdr;
  char *ptr;
  P *pp;

  hdr=halloc();
  ptr=vlock(vmem,hdr->seg);

  if(p->ofst!=p->hdr->hole) gstgap(p->hdr,p->ptr,p->ofst);
  mcpy(ptr,p->ptr+p->hdr->ehole,SEGSIZ-p->hdr->ehole);
  hdr->hole=SEGSIZ-p->hdr->ehole;
  hdr->nlines=mcnt(ptr,'\n',hdr->hole);
  p->hdr->nlines-=hdr->nlines;
  vchanged(ptr);
  p->hdr->ehole=SEGSIZ;

  enquef(H,link,p->hdr,hdr);

  vunlock(p->ptr);

  for(pp=p->link.next;pp!=p;pp=pp->link.next)
   if(pp->hdr==p->hdr && pp->ofst>=p->ofst)
    {
    pp->hdr=hdr;
    if(pp->ptr) { vunlock(pp->ptr); pp->ptr=ptr; vupcount(ptr); }
    pp->ofst-=p->ofst;
    }

  p->ptr=ptr;
  p->hdr=hdr;
  p->ofst=0;
  }
 }

/* Make a chain out of a block of memory */
/* The block must not be empty */

static H *bldchn(blk,size,nlines)
char *blk;
int size;
long *nlines;
 {
 H anchor, *l;
 *nlines=0;
 izque(H,link,&anchor);
 do
  {
  char *ptr;
  int amnt;
  ptr=vlock(vmem,(l=halloc())->seg);
  if(size>SEGSIZ) amnt=SEGSIZ;
  else amnt=size;
  mcpy(ptr,blk,amnt);
  l->hole=amnt; l->ehole=SEGSIZ; (*nlines)+=(l->nlines=mcnt(ptr,'\n',amnt));
  vchanged(ptr); vunlock(ptr);
  enqueb(H,link,&anchor,l);
  blk+=amnt; size-=amnt;
  }
  while(size);
 l=anchor.link.next;
 deque(H,link,&anchor);
 return l;
 }

/* Insert a chain into a buffer */
/* This does not update pointers */

static void inschn(p,a)
P *p;
H *a;
 {
 if(!p->b->eof->byte)
  { /* P's buffer is empty: replace the empty segment in p with a */
  hfree(p->hdr); p->hdr=a;
  vunlock(p->ptr); p->ptr=vlock(vmem,a->seg);
  pset(p->b->bof,p);

  p->b->eof->hdr=a->link.prev;
  vunlock(p->b->eof->ptr); p->b->eof->ptr=vlock(vmem,p->b->eof->hdr->seg);
  p->b->eof->ofst=GSIZE(p->b->eof->hdr);
  }
 else if(piseof(p))
  { /* We're at the end of the file: append a to the file */
  p->b->eof->hdr=a->link.prev;
  spliceb(H,link,p->b->bof->hdr,a);
  vunlock(p->b->eof->ptr); p->b->eof->ptr=vlock(vmem,p->b->eof->hdr->seg);
  p->b->eof->ofst=GSIZE(p->b->eof->hdr);
  p->hdr=a;
  vunlock(p->ptr); p->ptr=vlock(vmem,p->hdr->seg); p->ofst=0;
  }
 else if(pisbof(p))
  { /* We're at the beginning of the file: insert chain and set bof pointer */
  p->hdr=spliceb(H,link,p->hdr,a);
  vunlock(p->ptr); p->ptr=vlock(vmem,a->seg);
  pset(p->b->bof,p);
  }
 else
  { /* We're in the middle of the file: split and insert */
  bsplit(p);
  p->hdr=spliceb(H,link,p->hdr,a);
  vunlock(p->ptr); p->ptr=vlock(vmem,a->seg);
  }
 }

static void fixupins(p,amnt,nlines,hdr,hdramnt)
P *p;
long amnt;
long nlines;
H *hdr;
int hdramnt;
 {
 P *pp;
 if(nlines && !pisbol(p)) scrins(p->b,p->line,nlines,1);
 else scrins(p->b,p->line,nlines,0);
 inserr(p->b->name,p->line,nlines);

 for(pp=p->link.next;pp!=p;pp=pp->link.next)
  if(pp->line==p->line &&
     (pp->byte>p->byte || pp->end && pp->byte==p->byte)) pp->valcol=0;
 for(pp=p->link.next;pp!=p;pp=pp->link.next)
  if(pp->byte==p->byte && !pp->end)
   if(pp->ptr) pset(pp,p);
   else poffline(pset(pp,p));
  else if(pp->byte>p->byte || pp->end && pp->byte==p->byte)
   {
   pp->byte+=amnt;
   pp->line+=nlines;
   if(pp->hdr==hdr) pp->ofst+=hdramnt;
   }
 if(p->b->undo) undoins(p->b->undo,p,amnt);
 p->b->changed=1;
 }

/* Insert a buffer at pointer position */
/* The buffer goes away */

P *binsb(p,b)
P *p;
B *b;
 {
 if(b->eof->byte)
  {
  P *q=pdup(p);
  inschn(q,b->bof->hdr);
  b->eof->hdr=halloc();
  fixupins(q,b->eof->byte,b->eof->line,NULL,0);
  pcoalesce(q);
  prm(q);
  }
 brm(b);
 return p;
 }

P *binsm(p,blk,amnt)
P *p;
char *blk;
int amnt;
 {
 long nlines;
 H *h=0;
 int hdramnt;
 P *q;
 if(!amnt) return p;
 q=pdup(p);
 if(amnt<=GGAPSZ(q->hdr))
  {
  h=q->hdr;
  hdramnt=amnt;
  ginsm(q->hdr,q->ptr,q->ofst,blk,amnt);
  q->hdr->nlines+=(nlines=mcnt(blk,'\n',amnt));
  }
 else if(!q->ofst && q->hdr!=q->b->bof->hdr && amnt<=GGAPSZ(q->hdr->link.prev))
  {
  pprev(q); 
  ginsm(q->hdr,q->ptr,q->ofst,blk,amnt);
  q->hdr->nlines+=(nlines=mcnt(blk,'\n',amnt));
  }
 else
  {
  H *a=bldchn(blk,amnt,&nlines);
  inschn(q,a);
  }
 fixupins(q,(long)amnt,nlines,h,hdramnt);
 pcoalesce(q);
 prm(q);
 return p;
 }

P *binsc(p,c)
P *p;
char c;
 {
 if(p->b->o.crlf && c=='\n') return binsm(p,"\r\n",2);
 else return binsm(p,&c,1);
 }

P *binss(p,s)
P *p;
char *s;
 {
 return binsm(p,s,zlen(s));
 }

/* Read 'size' bytes from file or stream.  Stops and returns amnt. read
 * when requested size has been read or when end of file condition occurs.
 * Returns with -2 in error for read error or 0 in error for success.
 */

static int bkread(fi,buff,size)
char *buff;
 {
 int a,b;
 if(!size) { error=0; return 0; }
 for(a=b=0;(a<size) && ((b=jread(fi,buff+a,size-a))>0);a+=b);
 if(b<0) error= -2;
 else error=0;
 return a;
 }

/* Read up to 'max' bytes from a file into a buffer */
/* Returns with 0 in error or -2 in error for read error */

B *bread(fi,max)
long max;
 {
 H anchor, *l;
 long lines=0, total=0;
 int amnt;
 char *seg;
 izque(H,link,&anchor);
 error=0;
 while(seg=vlock(vmem,(l=halloc())->seg),
       !error && (amnt=bkread(fi,seg,max>=SEGSIZ?SEGSIZ:(int)max)))
  {
  total+=amnt;
  max-=amnt;
  l->hole=amnt;
  lines+=(l->nlines=mcnt(seg,'\n',amnt));
  vchanged(seg); vunlock(seg);
  enqueb(H,link,&anchor,l);
  }
 hfree(l); vunlock(seg);
 if(!total) return bmk(NULL);
 l=anchor.link.next;
 deque(H,link,&anchor);
 return bmkchn(l,NULL,total,lines);
 }

/* Parse file name.
 *
 * Removes ',xxx,yyy' from end of name and puts their value into skip and amnt
 * Replaces ~user/ with directory of given user
 * Replaces ~/ with $HOME
 *
 * Returns new variable length string.
 */

char *parsens(s,skip,amnt)
char *s;
long *skip, *amnt;
 {
 char *n=vsncpy(NULL,0,sz(s));
 int x;
 *skip=0;
 *amnt= MAXLONG;
 for(x=sLEN(n)-1;x>0 && (n[x]>='0' && n[x]<='9' || n[x]=='x' || n[x]=='X');--x);
 if(n[x]==',')
  {
  n[x]=0;
  if(n[x+1]=='x' || n[x+1]=='X') sscanf(n+x+2,"%lx",skip);
  else if(n[x+1]=='0' && (n[x+2]=='x' || n[x+2]=='X')) sscanf(n+x+3,"%lx",skip);
  else if(n[x+1]=='0') sscanf(n+x+1,"%lo",skip);
  else sscanf(n+x+1,"%ld",skip);
  for(--x;x>0 && (n[x]>='0' && n[x]<='9' || n[x]=='x' || n[x]=='X');--x);
  if(n[x]==',')
   {
   n[x]=0;
   *amnt= *skip;
   if(n[x+1]=='x' || n[x+1]=='X') sscanf(n+x+2,"%lx",skip);
   else if(n[x+1]=='0' && (n[x+2]=='x' || n[x+2]=='X')) sscanf(n+x+3,"%lx",skip);
   else if(n[x+1]=='0') sscanf(n+x+1,"%lo",skip);
   else sscanf(n+x+1,"%ld",skip);
   }
  }
#ifndef __MSDOS__
 if(n[0]=='~')
  {
  for(x=1;n[x] && n[x]!='/';++x);
  if(n[x]=='/')
   if(x==1)
    {
    char *z;
    s=getenv("HOME");
    z=vsncpy(NULL,0,sz(s));
    z=vsncpy(z,sLEN(z),sz(n+x));
    vsrm(n);
    n=z;
    }
   else
    {
    struct passwd *passwd;
    n[x]=0;
    passwd=getpwnam(n+1);
    n[x]='/';
    if(passwd)
     {
     char *z=vsncpy(NULL,0,sz(passwd->pw_dir));
     z=vsncpy(z,sLEN(z),sz(n+x));
     vsrm(n);
     n=z;
     }
    }
  }
#endif
 return n;
 }

/* Load file into new buffer and return the new buffer */
/* Returns with error set to 0 for success,
 * -1 for new file (file doesn't exist)
 * -2 for read error
 * -3 for seek error
 * -4 for open error
 */

B *bload(s)
char *s;
 {
 char buffer[SEGSIZ];
 FILE *fi;
 B *b;
 long skip,amnt;
 char *n;
 int nowrite=0;

 if(!s || !s[0])
  {
  error= -1; 
  b=bmk(NULL);
  setopt(&b->o,"");
  b->rdonly=b->o.readonly;
  b->er=error;
  return b;
  }

 n=parsens(s,&skip,&amnt);

 /* Open file or stream */
 ossep(n);
#ifndef __MSDOS__
 if(n[0]=='!')
  {
  nescape(maint->t);
  ttclsn();
  fi=popen(n+1,"r");
  }
 else
#endif
 if(!zcmp(n,"-")) fi=stdin;
 else
  {
  fi=fopen(n,"r+");
  if(!fi) nowrite=1;
  else fclose(fi);
  fi=fopen(n,"r");
  if(!fi) nowrite=0;
  }
 joesep(n);

 /* Abort if couldn't open */
 if(!fi)
  {
  if(errno==ENOENT) error= -1;
  else error= -4;
  b=bmk(NULL);
  setopt(&b->o,n);
  b->rdonly=b->o.readonly;
  goto opnerr;
  }

 /* Skip data if we need to */
 if(skip && lseek(fileno(fi),skip,0)<0)
  {
  int r;
  while(skip>SEGSIZ)
   {
   r=bkread(fileno(fi),buffer,SEGSIZ);
   if(r!=SEGSIZ || error) { error= -3; goto err; }
   skip-=SEGSIZ;
   }
  skip-=bkread(fileno(fi),buffer,(int)skip);
  if(skip || error) { error= -3; goto err; }
  }

 /* Read from stream into new buffer */
 b=bread(fileno(fi),amnt);
 setopt(&b->o,n);
 b->rdonly=b->o.readonly;

 /* Close stream */
 err:;
#ifndef __MSDOS__
 if(s[0]=='!') pclose(fi);
 else
#endif
 if(zcmp(n,"-")) fclose(fi);

 opnerr:;
 if(s[0]=='!') ttopnn(), nreturn(maint->t);

 /* Set name */
 b->name=joesep(zdup(s));

 /* Set flags */
 if(error || s[0]=='!' || skip || amnt!=MAXLONG) b->backup=1, b->changed=0;
 else if(!zcmp(n,"-")) b->backup=1, b->changed=1;
 else b->backup=0, b->changed=0;
 if(nowrite) b->rdonly=b->o.readonly=1;

 /* Eliminate parsed name */
 vsrm(n);

 b->er=error;
 return b;
 }

/* Find already loaded buffer or load file into new buffer */

B *bfind(s)
char *s;
 {
 B *b;
 if(!s || !s[0])
  {
  error= -1; 
  b=bmk(NULL);
  setopt(&b->o,"");
  b->rdonly=b->o.readonly;
  b->internal=0;
  b->er=error;
  return b;
  }
 for(b=bufs.link.next;b!=&bufs;b=b->link.next)
  if(b->name && !zcmp(s,b->name))
   {
   if(!b->orphan) ++b->count;
   else b->orphan=0;
   error=0;
   b->internal=0;
   return b;
   }
 b=bload(s);
 b->internal=0;
 return b;
 }

char **getbufs()
 {
 char **s=vamk(16);
 B *b;
 for(b=bufs.link.next;b!=&bufs;b=b->link.next)
  if(b->name) s=vaadd(s,vsncpy(NULL,0,sz(b->name)));
 return s;
 }

/* Find an orphaned buffer */

B *borphan()
 {
 B *b;
 for(b=bufs.link.next;b!=&bufs;b=b->link.next)
  if(b->orphan)
   {
   b->orphan=0;
   return b;
   }
 return 0;
 }

/* Write 'size' bytes from file beginning at 'p' to open file 'fd'.
 * Returns error.
 * error is set to -5 for write error or 0 for success.
 * Don't attempt to write past the end of the file
 */

int bsavefd(p,fd,size)
P *p;
long size;
 {
 P *np=pdup(p);
 int amnt;
 while(size>(amnt=GSIZE(np->hdr)-np->ofst))
  {
  if(np->ofst<np->hdr->hole)
   {
   if(jwrite(fd,np->ptr+np->ofst,np->hdr->hole-np->ofst)<0)
    goto err;
   if(jwrite(fd,np->ptr+np->hdr->ehole,SEGSIZ-np->hdr->ehole)<0)
    goto err;
   }
  else
   if(jwrite(fd,np->ptr+np->ofst+GGAPSZ(np->hdr),amnt)<0)
    goto err;
  size-=amnt;
  pnext(np);
  }
 if(size)
  if(np->ofst<np->hdr->hole)
   if(size>np->hdr->hole-np->ofst)
    {
    if(jwrite(fd,np->ptr+np->ofst,np->hdr->hole-np->ofst)<0)
     goto err;
    if(jwrite(fd,np->ptr+np->hdr->ehole,(int)size-np->hdr->hole+np->ofst)<0)
     goto err;
    }
   else
    {
    if(jwrite(fd,np->ptr+np->ofst,(int)size)<0)
     goto err;
    }
  else if(jwrite(fd,np->ptr+np->ofst+GGAPSZ(np->hdr),(int)size)<0)
   goto err;
 prm(np);
 return error=0;
 err:;
 prm(np);
 return error=5;
 }

/* Save 'size' bytes beginning at 'p' in file 's' */

int bsave(p,s,size)
P *p;
char *s;
long size;
 {
 FILE *f;
 long skip,amnt;

 s=parsens(s,&skip,&amnt);

 if(amnt<size) size=amnt;

 ossep(s);
#ifndef __MSDOS__
 if(s[0]=='!')
  {
  nescape(maint->t);
  ttclsn();
  f=popen(s+1,"w");
  }
 else
#endif
 if(s[0]=='>' && s[1]=='>') f=fopen(s+2,"a");
 else if(!zcmp(s,"-"))
  {
  nescape(maint->t);
  ttclsn();
  f=stdout;
  }
 else
  if(skip || amnt!=MAXLONG) f=fopen(s,"r+");
  else f=fopen(s,"w");
 joesep(s);

 if(!f)
  {
  error= -4;
  goto opnerr;
  }
 fflush(f);

 if(skip && lseek(fileno(f),skip,0)<0) { error= -3; goto err; }

 bsavefd(p,fileno(f),size);

 if(!error && force && size && !skip && amnt==MAXLONG)
  {
  P *q=pdup(p);
  char nl='\n';
  pfwrd(q,size-1);
  if(brc(q)!='\n' && jwrite(fileno(f),&nl,1)<0) error= -5;
  prm(q);
  }

 err:;
#ifndef __MSDOS__
 if(s[0]=='!') pclose(f);
 else
#endif
 if(zcmp(s,"-")) fclose(f);
 else fflush(f);

 opnerr:;
 if(s[0]=='!' || !zcmp(s,"-")) ttopnn(), nreturn(maint->t);
 return error;
 }

int brc(p)
P *p;
 {
 if(p->ofst==GSIZE(p->hdr)) return MAXINT;
 if(p->ofst>=p->hdr->hole) return p->ptr[p->ofst+p->hdr->ehole-p->hdr->hole];
 else return p->ptr[p->ofst];
 }

char *brmem(p,blk,size)
P *p;
char *blk;
int size;
 {
 char *bk=blk;
 P *np;
 int amnt;
 np=pdup(p);
 while(size>(amnt=GSIZE(np->hdr)-np->ofst))
  {
  grmem(np->hdr,np->ptr,np->ofst,bk,amnt);
  bk+=amnt;
  size-=amnt;
  pnext(np);
  }
 if(size) grmem(np->hdr,np->ptr,np->ofst,bk,size);
 prm(np);
 return blk;
 }

char *brs(p,size)
P *p;
int size;
 {
 char *s=(char *)malloc(size+1);
 s[size]=0;
 return brmem(p,s,size);
 }

char *brvs(p,size)
P *p;
int size;
 {
 char *s=vstrunc(NULL,size);
 return brmem(p,s,size);
 }

/* Save edit buffers when editor dies */

extern char *ctime();

void ttsig(sig)
 {
 long tim=time(0);
 B *b;
 FILE *f=fopen("DEADJOE","a");
 fprintf(f,"\n*** Modified files in JOE when it aborted on %s",ctime(&tim));
 if(sig) fprintf(f,"*** JOE was aborted by signal %d\n",sig);
 else fprintf(f,"*** JOE was aborted because the terminal closed\n");
 fflush(f);
 for(b=bufs.link.next;b!=&bufs;b=b->link.next)
  if(b->changed)
   {
   if(b->name) fprintf(f,"\n*** File \'%s\'\n",b->name);
   else fprintf(f,"\n*** File \'(Unnamed)\'\n");
   fflush(f);
   bsavefd(b->bof,fileno(f),b->eof->byte);
   }
 if(sig) ttclsn();
 _exit(1);
 }
