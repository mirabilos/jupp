/* Edit buffer window generation
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

#include "config.h"
#include "tty.h"
#include "vfile.h"
#include "termcap.h"
#include "kbd.h"
#include "b.h"
#include "scrn.h"
#include "w.h"
#include "ublock.h"
#include "zstr.h"
#include "blocks.h"
#include "bw.h"

/* Display modes */
int dspasis=0;
int marking=0;
extern int square;
extern int staen;

P *getto(p,cur,top,line)
P *p,*cur,*top;
long line;
{
long dist=MAXLONG;
long d;
P *best;
if(!p)
 {
 if(d=(line-cur->line>=0?line-cur->line:cur->line-line), d<dist)
  dist=d, best=cur;
 if(d=(line-top->line>=0?line-top->line:top->line-line), d<dist)
  dist=d, best=top;
 p=pdup(best);
 pbol(p);
 }
while(line>p->line) if(!pnextl(p)) break;
if(line<p->line)
 {
 while(line<p->line) pprevl(p);
 pbol(p);
 }
return p;
}

/* Scroll window to follow cursor */

int mid=0;

void bwfllw(w)
BW *w;
{
P *newtop;
if(w->cursor->line<w->top->line)
 {
 newtop=pdup(w->cursor);
 pbol(newtop);
 if(mid)
  if(newtop->line>=w->h/2) pline(newtop,newtop->line-w->h/2);
  else pset(newtop,newtop->b->bof);
 if(w->top->line-newtop->line<w->h)
  nscrldn(w->t->t,w->y,w->y+w->h,(int)(w->top->line-newtop->line));
 else msetI(w->t->t->updtab+w->y,1,w->h);
 pset(w->top,newtop);
 prm(newtop);
 }
else if(w->cursor->line>=w->top->line+w->h)
 {
 newtop=pdup(w->top);
 if(mid) newtop=getto(NULL,w->cursor,w->top,w->cursor->line-w->h/2);
 else newtop=getto(NULL,w->cursor,w->top,w->cursor->line-(w->h-1));
 if(newtop->line-w->top->line<w->h)
  nscrlup(w->t->t,
            w->y,
            w->y+w->h,
            (int)(newtop->line-w->top->line));
 else msetI(w->t->t->updtab+w->y,1,w->h);
 pset(w->top,newtop);
 prm(newtop);
 }

/* Adjust column */
if(w->cursor->xcol<w->offset)
 {
 w->offset=w->cursor->xcol;
 msetI(w->t->t->updtab+w->y,1,w->h);
 }
else if(w->cursor->xcol>=w->offset+w->w)
 {
 w->offset=w->cursor->xcol-(w->w-1);
 msetI(w->t->t->updtab+w->y,1,w->h);
 }
}

/* Scroll a buffer window after an insert occured.  'flg' is set to 1 if
 * the first line was split
 */

void bwins(w,l,n,flg)
BW *w;
long l,n;
int flg;
{
if(l+flg+n<w->top->line+w->h && l+flg>=w->top->line && l+flg<=w->b->eof->line)
 {
 if(flg) w->t->t->sary[w->y+l-w->top->line]=w->t->t->li;
 nscrldn(w->t->t,(int)(w->y+l+flg-w->top->line),w->y+w->h,(int)n);
 }
if(l<w->top->line+w->h && l>=w->top->line)
 if(n>=w->h-(l-w->top->line))
  msetI(w->t->t->updtab+w->y+l-w->top->line,1,w->h-(int)(l-w->top->line));
 else
  msetI(w->t->t->updtab+w->y+l-w->top->line,1,(int)n+1);
}

/* Scroll current windows after a delete */

void bwdel(w,l,n,flg)
BW *w;
long l,n;
int flg;
{
/* Update the line where the delete began */
if(l<w->top->line+w->h && l>=w->top->line)
 w->t->t->updtab[w->y+l-w->top->line]=1;

/* Update the line where the delete ended */
if(l+n<w->top->line+w->h && l+n>=w->top->line)
 w->t->t->updtab[w->y+l+n-w->top->line]=1;

if(l<w->top->line+w->h &&
   (l+n>=w->top->line+w->h ||
    l+n==w->b->eof->line && w->b->eof->line>=w->top->line+w->h))
 if(l>=w->top->line)
  /* Update window from l to end */
  msetI(w->t->t->updtab+w->y+l-w->top->line,1,w->h-(int)(l-w->top->line));
 else
  /* Update entire window */
  msetI(w->t->t->updtab+w->y,1,w->h);
else if(l<w->top->line+w->h && l+n==w->b->eof->line &&
        w->b->eof->line<w->top->line+w->h)
 if(l>=w->top->line)
  /* Update window from l to end of file */
  msetI(w->t->t->updtab+w->y+l-w->top->line,1,(int)n);
 else
  /* Update from beginning of window to end of file */
  msetI(w->t->t->updtab+w->y,1,(int)(w->b->eof->line-w->top->line));
else if(l+n<w->top->line+w->h &&
        l+n>w->top->line &&
        l+n<w->b->eof->line)
 if(l+flg>=w->top->line)
  nscrlup(w->t->t,(int)(w->y+l+flg-w->top->line),w->y+w->h,(int)n);
 else
  nscrlup(w->t->t,w->y,w->y+w->h,(int)(l+n-w->top->line));
}

/* Update a single line */

static int lgen(t,y,screen,x,w,p,scr,from,to)
SCRN *t;
int y;
int *screen;	/* Screen line address */
int w;		/* Window */
P *p;		/* Buffer pointer */
long scr;	/* Starting column to display */
long from,to;	/* Range for marked block */
{
int ox=x;
int done=1;
long col=0;
long byte=p->byte;
char *bp;		/* Buffer pointer, 0 if not set */
int amnt;		/* Amount left in this segment of the buffer */
int c, ta, c1;
unsigned char bc;

/* Initialize bp and amnt from p */
if(p->ofst>=p->hdr->hole)
 {
 bp=p->ptr+p->hdr->ehole+p->ofst-p->hdr->hole;
 amnt=SEGSIZ-p->hdr->ehole-(p->ofst-p->hdr->hole);
 }
else
 {
 bp=p->ptr+p->ofst;
 amnt=p->hdr->hole-p->ofst;
 }

if(col==scr) goto loop;
lp:		/* Display next character */
if(amnt) do
 {
 bc= *bp++;
 if(p->b->o.crlf && bc=='\r')
  {
  ++byte;
  if(!--amnt)
   {
   pppl:
   if(bp==p->ptr+SEGSIZ)
    {
    if(pnext(p))
     {
     bp=p->ptr;
     amnt=p->hdr->hole;
     }
    else goto nnnl;
    }
   else
    {
    bp=p->ptr+p->hdr->ehole;
    amnt=SEGSIZ-p->hdr->ehole;
    if(!amnt) goto pppl;
    }
   }
  if(*bp=='\n')
   {
   ++bp;
   ++byte;
   ++amnt;
   goto eobl;
   }
  nnnl: --byte; ++amnt;
  }
 if(square)
  if(bc=='\t')
   {
   long tcol=col+p->b->o.tab-col%p->b->o.tab;
   if(tcol>from && tcol<=to) c1=INVERSE;
   else c1=0;
   }
  else
   if(col>=from && col<to) c1=INVERSE;
   else c1=0;
 else
  if(byte>=from && byte<to) c1=INVERSE;
  else c1=0;
 ++byte;
 if(bc=='\t')
  {
  ta=p->b->o.tab-col%p->b->o.tab;
  if(ta+col>scr)
   {
   ta-=scr-col;
   goto dota;
   }
  if((col+=ta)==scr) { --amnt; goto loop; }
  }
 else if(bc=='\n') goto eobl;
 else if(++col==scr) { --amnt; goto loop; }
 }
 while(--amnt);
if(bp==p->ptr+SEGSIZ)
 {
 if(pnext(p))
  {
  bp=p->ptr;
  amnt=p->hdr->hole;
  goto lp;
  }
 }
else
 {
 bp=p->ptr+p->hdr->ehole;
 amnt=SEGSIZ-p->hdr->ehole;
 goto lp;
 }
goto eof;

loop:		/* Display next character */
if(amnt) do
 {
 bc= *bp++;
 if(p->b->o.crlf && bc=='\r')
  {
  ++byte;
  if(!--amnt)
   {
   ppl:
   if(bp==p->ptr+SEGSIZ)
    {
    if(pnext(p))
     {
     bp=p->ptr;
     amnt=p->hdr->hole;
     }
    else goto nnl;
    }
   else
    {
    bp=p->ptr+p->hdr->ehole;
    amnt=SEGSIZ-p->hdr->ehole;
    if(!amnt) goto ppl;
    }
   }
  if(*bp=='\n')
   {
   ++bp;
   ++byte;
   ++amnt;
   goto eobl;
   }
  nnl: --byte; ++amnt;
  }
 if(square)
  if(bc=='\t')
   {
   long tcol=scr+x-ox+p->b->o.tab-(scr+x-ox)%p->b->o.tab;
   if(tcol>from && tcol<=to) c1=INVERSE;
   else c1=0;
   }
  else
   if(scr+x-ox>=from && scr+x-ox<to) c1=INVERSE;
   else c1=0;
 else
  if(byte>=from && byte<to) c1=INVERSE;
  else c1=0;
 ++byte;
 if(bc=='\t')
  {
  ta=p->b->o.tab-((x-ox+scr)%p->b->o.tab);
  dota:
  do
   {
   outatr(t,screen+x,x,y,' ',c1);
   if(ifhave) goto bye;
   if(++x==w) goto eosl;
   }
   while(--ta);
  }
 else if(bc=='\n') goto eobl;
 else
  {
  xlat(c,bc);
  c^=c1;
  outatr(t,screen+x,x,y,bc,c);
  if(ifhave) goto bye;
  if(++x==w) goto eosl;
  }
 }
 while(--amnt);
if(bp==p->ptr+SEGSIZ)
 {
 if(pnext(p))
  {
  bp=p->ptr;
  amnt=p->hdr->hole;
  goto loop;
  }
 }
else
 {
 bp=p->ptr+p->hdr->ehole;
 amnt=SEGSIZ-p->hdr->ehole;
 goto loop;
 }
goto eof;

eobl:		/* End of buffer line found.  Erase to end of screen line */
++p->line;
eof:
if(x!=w) done=eraeol(t,x,y);
else done=0;

/* Set p to bp/amnt */
bye:
if(bp-p->ptr<=p->hdr->hole) p->ofst=bp-p->ptr;
else p->ofst=bp-p->ptr-(p->hdr->ehole-p->hdr->hole);
p->byte=byte;
return done;

eosl:
if(bp-p->ptr<=p->hdr->hole) p->ofst=bp-p->ptr;
else p->ofst=bp-p->ptr-(p->hdr->ehole-p->hdr->hole);
p->byte=byte;
pnextl(p);
return 0;
}

/* Generate line into an array */

static int lgena(t,y,screen,x,w,p,scr,from,to)
SCRN *t;
int y;
int *screen;	/* Screen line address */
int w;		/* Window */
P *p;		/* Buffer pointer */
long scr;	/* Starting column to display */
long from,to;	/* Range for marked block */
{
int ox=x;
int done=1;
long col=0;
long byte=p->byte;
char *bp;		/* Buffer pointer, 0 if not set */
int amnt;		/* Amount left in this segment of the buffer */
int c, ta, c1;
unsigned char bc;

/* Initialize bp and amnt from p */
if(p->ofst>=p->hdr->hole)
 {
 bp=p->ptr+p->hdr->ehole+p->ofst-p->hdr->hole;
 amnt=SEGSIZ-p->hdr->ehole-(p->ofst-p->hdr->hole);
 }
else
 {
 bp=p->ptr+p->ofst;
 amnt=p->hdr->hole-p->ofst;
 }

if(col==scr) goto loop;
lp:		/* Display next character */
if(amnt) do
 {
 bc= *bp++;
 if(square)
  if(bc=='\t')
   {
   long tcol=col+p->b->o.tab-col%p->b->o.tab;
   if(tcol>from && tcol<=to) c1=INVERSE;
   else c1=0;
   }
  else
   if(col>=from && col<to) c1=INVERSE;
   else c1=0;
 else
  if(byte>=from && byte<to) c1=INVERSE;
  else c1=0;
 ++byte;
 if(bc=='\t')
  {
  ta=p->b->o.tab-col%p->b->o.tab;
  if(ta+col>scr)
   {
   ta-=scr-col;
   goto dota;
   }
  if((col+=ta)==scr) { --amnt; goto loop; }
  }
 else if(bc=='\n') goto eobl;
 else if(++col==scr) { --amnt; goto loop; }
 }
 while(--amnt);
if(bp==p->ptr+SEGSIZ)
 {
 if(pnext(p))
  {
  bp=p->ptr;
  amnt=p->hdr->hole;
  goto lp;
  }
 }
else
 {
 bp=p->ptr+p->hdr->ehole;
 amnt=SEGSIZ-p->hdr->ehole;
 goto lp;
 }
goto eobl;

loop:		/* Display next character */
if(amnt) do
 {
 bc= *bp++;
 if(square)
  if(bc=='\t')
   {
   long tcol=scr+x-ox+p->b->o.tab-(scr+x-ox)%p->b->o.tab;
   if(tcol>from && tcol<=to) c1=INVERSE;
   else c1=0;
   }
  else
   if(scr+x-ox>=from && scr+x-ox<to) c1=INVERSE;
   else c1=0;
 else
  if(byte>=from && byte<to) c1=INVERSE;
  else c1=0;
 ++byte;
 if(bc=='\t')
  {
  ta=p->b->o.tab-((x-ox+scr)%p->b->o.tab);
  dota:
  do
   {
   screen[x]=' '+c1;
   if(++x==w) goto eosl;
   }
   while(--ta);
  }
 else if(bc=='\n') goto eobl;
 else
  {
  xlat(c,bc);
  c^=c1;
  screen[x]=c+bc;
  if(++x==w) goto eosl;
  }
 }
 while(--amnt);
if(bp==p->ptr+SEGSIZ)
 {
 if(pnext(p))
  {
  bp=p->ptr;
  amnt=p->hdr->hole;
  goto loop;
  }
 }
else
 {
 bp=p->ptr+p->hdr->ehole;
 amnt=SEGSIZ-p->hdr->ehole;
 goto loop;
 }
goto eof;
eobl:		/* End of buffer line found.  Erase to end of screen line */
++p->line;
eof:
while(x!=w) screen[x++]=' ';
done=0;

/* Set p to bp/amnt */
bye:
if(bp-p->ptr<=p->hdr->hole) p->ofst=bp-p->ptr;
else p->ofst=bp-p->ptr-(p->hdr->ehole-p->hdr->hole);
p->byte=byte;
return done;

eosl:
if(bp-p->ptr<=p->hdr->hole) p->ofst=bp-p->ptr;
else p->ofst=bp-p->ptr-(p->hdr->ehole-p->hdr->hole);
p->byte=byte;
pnextl(p);
return 0;
}

void gennum(w,screen,t,y,comp)
BW *w;
int *screen;
SCRN *t;
int *comp;
 {
 char buf[12];
 int z;
 int lin=w->top->line+y-w->y;
 if(lin<=w->b->eof->line) sprintf(buf,"%5ld ",w->top->line+y-w->y+1);
 else zcpy(buf,"      ");
 for(z=0;buf[z];++z)
  {
  outatr(t,screen+z,z,y,buf[z],0);
  if(ifhave) return;
  comp[z]=buf[z];
  }
 }

void bwgen(w,linums)
BW *w;
{
int *screen;
P *p=0;
P *q=pdup(w->cursor);
int bot=w->h+w->y;
int y;
int dosquare=0;
long from,to;
long fromline,toline;
SCRN *t=w->t->t;
fromline=toline=from=to=0;

if(markv(0) && markk->b==w->b)
 if(square)
  {
  from=markb->xcol, to=markk->xcol, dosquare=1;
  fromline=markb->line;
  toline=markk->line;
  }
 else from=markb->byte, to=markk->byte;
else if(marking && markb && markb->b==w->b &&
        w->cursor->byte!= markb->byte && !from)
 if(square)
  {
  from=Lmin(w->cursor->xcol,markb->xcol),
    to=Lmax(w->cursor->xcol,markb->xcol);
  fromline=Lmin(w->cursor->line,markb->line);
  toline=Lmax(w->cursor->line,markb->line);
  dosquare=1;
  }
 else from=Lmin(w->cursor->byte,markb->byte),
        to=Lmax(w->cursor->byte,markb->byte);

if(marking) msetI(t->updtab+w->y,1,w->h);

y=w->cursor->line-w->top->line+w->y;
for(screen=t->scrn+y*w->t->w;y!=bot; ++y, screen+=w->t->w)
 {
 if(ifhave && !linums) break;
 if(linums) gennum(w,screen,t,y,t->compose);
 if(t->updtab[y])
  {
  p=getto(p,w->cursor,w->top,w->top->line+y-w->y);
  if(t->insdel && !w->x)
   {
   pset(q,p);
   if(dosquare)
    if(w->top->line+y-w->y>=fromline &&
       w->top->line+y-w->y<=toline)
     lgena(t,y,t->compose,w->x,w->x+w->w,q,w->offset,from,to);
    else
     lgena(t,y,t->compose,w->x,w->x+w->w,q,w->offset,0L,0L);
   else
    lgena(t,y,t->compose,w->x,w->x+w->w,q,w->offset,from,to);
   magic(t,y,screen,t->compose,
         (int)(w->cursor->xcol-w->offset+w->x));
   }
  if(dosquare)
   if(w->top->line+y-w->y>=fromline &&
      w->top->line+y-w->y<=toline)
    t->updtab[y]=lgen(t,y,screen,w->x,w->x+w->w,p,w->offset,
                            from,to);
   else
    t->updtab[y]=lgen(t,y,screen,w->x,w->x+w->w,p,w->offset,
                            0L,0L);
  else
   t->updtab[y]=lgen(t,y,screen,w->x,w->x+w->w,p,w->offset,
                           from,to);
  }
 }
 
y=w->y;
for(screen=t->scrn+w->y*w->t->w; y!=w->y+w->cursor->line-w->top->line;
    ++y, screen+=w->t->w)
 {
 if(ifhave && !linums) break;
 if(linums) gennum(w,screen,t,y,t->compose);
 if(t->updtab[y])
  {
  p=getto(p,w->cursor,w->top,w->top->line+y-w->y);
  if(t->insdel && !w->x)
   {
   pset(q,p);
   if(dosquare)
    if(w->top->line+y-w->y>=fromline &&
       w->top->line+y-w->y<=toline)
     lgena(t,y,t->compose,w->x,w->x+w->w,q,w->offset,from,to);
    else
     lgena(t,y,t->compose,w->x,w->x+w->w,q,w->offset,0L,0L);
   else
    lgena(t,y,t->compose,w->x,w->x+w->w,q,w->offset,from,to);
   magic(t,y,screen,t->compose,
         (int)(w->cursor->xcol-w->offset+w->x));
   }
  if(dosquare)
   if(w->top->line+y-w->y>=fromline &&
      w->top->line+y-w->y<=toline)
    t->updtab[y]=lgen(t,y,screen,w->x,w->x+w->w,p,w->offset,
                            from,to);
   else
    t->updtab[y]=lgen(t,y,screen,w->x,w->x+w->w,p,w->offset,
                            0L,0L);
  else
   t->updtab[y]=lgen(t,y,screen,w->x,w->x+w->w,p,w->offset,
                           from,to);
  }
 }
prm(q);
if(p) prm(p);
}

void bwmove(w,x,y)
BW *w;
int x,y;
{
w->x=x;
w->y=y;
}

void bwresz(w,wi,he)
BW *w;
int wi, he;
{
if(he>w->h && w->y!= -1) msetI(w->t->t->updtab+w->y+w->h,1,he-w->h);
w->w=wi;
w->h=he;
}

BW *bwmk(window,b,prompt)
W *window;
B *b;
int prompt;
{
BW *w=(BW *)malloc(sizeof(BW));
w->parent=window;
w->pid=0;
w->out= -1;
w->b=b;
if(prompt || !window->y && staen)
 w->y=window->y, w->h=window->h;
else
 w->y=window->y+1, w->h=window->h-1;
if(b->oldcur)
 {
 w->top=b->oldtop, b->oldtop=0, w->top->owner=0;
 w->cursor=b->oldcur, b->oldcur=0, w->cursor->owner=0;
 }
else
 {
 w->top=pdup(b->bof);
 w->cursor=pdup(b->bof);
 }
w->t=window->t;
w->object=NULL;
w->offset=0;
w->o=w->b->o;
if(w->o.linums) w->x=window->x+LINCOLS, w->w=window->w-LINCOLS;
else w->x=window->x, w->w=window->w;
if(window==window->main)
 {
 rmkbd(window->kbd);
 window->kbd=mkkbd(getcontext(w->o.context));
 } 
w->top->xcol=0; w->cursor->xcol=0;
return w;
}

void bwrm(w)
BW *w;
{
prm(w->top);
prm(w->cursor);
brm(w->b);
free(w);
}

int ustat(bw)
BW *bw;
{
static char buf[80];
unsigned c=brc(bw->cursor);
if(c==MAXINT)
 sprintf(buf,"** Line %ld  Col %ld  Offset %ld(0x%lx) **",
         bw->cursor->line+1,piscol(bw->cursor)+1,bw->cursor->byte,
         bw->cursor->byte);
else
 sprintf(buf,"** Line %ld  Col %ld  Offset %ld(0x%lx)  Ascii %d(0%o/0x%X) **",
         bw->cursor->line+1,piscol(bw->cursor)+1,bw->cursor->byte,
         bw->cursor->byte,255&c,255&c,255&c);
msgnw(bw,buf);
return 0;
}

int ucrawlr(bw)
BW *bw;
 {
 int amnt=bw->w/2;
 pcol(bw->cursor,bw->cursor->xcol+amnt);
 bw->cursor->xcol+=amnt;
 bw->offset+=amnt;
 updall();
 return 0;
 }

int ucrawll(bw)
BW *bw;
 {
 int amnt=bw->w/2;
 int curamnt=bw->w/2;
 if(amnt>bw->offset) amnt=bw->offset, curamnt=bw->offset;
 if(!bw->offset) curamnt=bw->cursor->xcol;
 if(!curamnt) return -1;
 pcol(bw->cursor,bw->cursor->xcol-curamnt);
 bw->cursor->xcol-=curamnt;
 bw->offset-=amnt;
 updall();
 return 0;
 }

void orphit(bw)
BW *bw;
 {
 ++bw->b->count;
 bw->b->orphan=1;
 pdupown(bw->cursor,&bw->b->oldcur);
 pdupown(bw->top,&bw->b->oldtop);
 }
