 /* Text editing windows
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
#include "w.h"
#include "termcap.h"
#include "vfile.h"
#include "b.h"
#include "tty.h"
#include "scrn.h"
#include "bw.h"
#include "zstr.h"
#include "vs.h"
#include "help.h"
#include "undo.h"
#include "main.h"
#include "macro.h"
#include "uedit.h"
#include "ufile.h"
#include "ushell.h"
#include "qw.h"
#include "tw.h"

char *ctime();
extern char *exmsg;
extern int square;
int staen=0;
int staupd=0;
int keepup=0;

/* Move text window */

static void movetw(bw,x,y)
BW *bw;
int x,y;
 {
 TW *tw=(TW *)bw->object;
 if(y || !staen)
  {
  if(!tw->staon)
   { /* Scroll down and shrink */
   nscrldn(bw->parent->t->t,y,bw->parent->nh+y,1);
   }
  bwmove(bw,x+(bw->o.linums?LINCOLS:0),y+1);
  tw->staon=1;
  }
 else
  {
  if(tw->staon)
   { /* Scroll up and grow */
   nscrlup(bw->parent->t->t,y,bw->parent->nh+y,1);
   }
  bwmove(bw,x+(bw->o.linums?LINCOLS:0),y);
  tw->staon=0;
  }
 }

/* Resize text window */

static void resizetw(bw,wi,he)
BW *bw;
int wi,he;
 {
 if(bw->parent->ny || !staen)
  bwresz(bw,wi-(bw->o.linums?LINCOLS:0),he-1);
 else
  bwresz(bw,wi-(bw->o.linums?LINCOLS:0),he);
 }

char *stagen(stalin,bw,s,fill)
char *stalin;
BW *bw;
char *s;
 {
 char buf[80];
 int x;
 W *w=bw->parent;
 stalin=vstrunc(stalin,0);
 while(*s)
  {
  if(*s=='%' && s[1])
   switch(*++s)
    {
    case 't':
     {
     long n=time(NULL);
     int l;
     char *d=ctime(&n);
     l=(d[11]-'0')*10+d[12]-'0';
     if(l>12) l-=12;
     sprintf(buf,"%2.2d",l);
     if(buf[0]=='0') buf[0]=fill;
     stalin=vsncpy(sv(stalin),buf,2);
     stalin=vsncpy(sv(stalin),d+13,3);
     }
    break;

    case 'u':
     {
     long n=time(NULL);
     char *d=ctime(&n);
     stalin=vsncpy(sv(stalin),d+11,5);
     }
    break;

    case 'T':
    if(bw->o.overtype) stalin=vsadd(stalin,'O');
    else stalin=vsadd(stalin,'I');
    break;
    
    case 'W':
    if(bw->o.wordwrap) stalin=vsadd(stalin,'W');
    else stalin=vsadd(stalin,fill);
    break;

    case 'I':
    if(bw->o.autoindent) stalin=vsadd(stalin,'A');
    else stalin=vsadd(stalin,fill);
    break;

    case 'X':
    if(square) stalin=vsadd(stalin,'X');
    else stalin=vsadd(stalin,fill);
    break;

    case 'n':
    stalin=vsncpy(sv(stalin),sz(bw->b->name?bw->b->name:"Unnamed"));
    break;

    case 'm':
    if(bw->b->changed)
     stalin=vsncpy(sv(stalin),sc("(Modified)"));
    break;

    case 'R':
    if(bw->b->rdonly)
     stalin=vsncpy(sv(stalin),sc("(Read only)"));
    break;

    case '*':
    if(bw->b->changed)
     stalin=vsadd(stalin,'*');
    else
     stalin=vsadd(stalin,fill);
    break;

    case 'r':
    sprintf(buf,"%-4ld",bw->cursor->line+1);
    for(x=0;buf[x];++x) if(buf[x]==' ') buf[x]=fill;
    stalin=vsncpy(sv(stalin),sz(buf));
    break;

    case 'o':
    sprintf(buf,"%-4ld",bw->cursor->byte);
    for(x=0;buf[x];++x) if(buf[x]==' ') buf[x]=fill;
    stalin=vsncpy(sv(stalin),sz(buf));
    break;

    case 'O':
    sprintf(buf,"%-4lX",bw->cursor->byte);
    for(x=0;buf[x];++x) if(buf[x]==' ') buf[x]=fill;
    stalin=vsncpy(sv(stalin),sz(buf));
    break;

    case 'a':
    if(!piseof(bw->cursor)) sprintf(buf,"%3d",255&brc(bw->cursor));
    else sprintf(buf,"   ");
    for(x=0;buf[x];++x) if(buf[x]==' ') buf[x]=fill;
    stalin=vsncpy(sv(stalin),sz(buf));
    break;

    case 'A':
    if(!piseof(bw->cursor)) sprintf(buf,"%2.2X",255&brc(bw->cursor));
    else sprintf(buf,"  ");
    for(x=0;buf[x];++x) if(buf[x]==' ') buf[x]=fill;
    stalin=vsncpy(sv(stalin),sz(buf));
    break;

    case 'c':
    sprintf(buf,"%-3ld",piscol(bw->cursor)+1);
    for(x=0;buf[x];++x) if(buf[x]==' ') buf[x]=fill;
    stalin=vsncpy(sv(stalin),sz(buf));
    break;

    case 'p':
    if(bw->b->eof->byte)
     sprintf(buf,"%3ld",bw->cursor->byte*100/bw->b->eof->byte);
    else
     sprintf(buf,"100");
    for(x=0;buf[x];++x) if(buf[x]==' ') buf[x]=fill;
    stalin=vsncpy(sv(stalin),sz(buf));
    break;

    case 'l':
    sprintf(buf,"%-4ld",bw->b->eof->line+1);
    for(x=0;buf[x];++x) if(buf[x]==' ') buf[x]=fill;
    stalin=vsncpy(sv(stalin),sz(buf));
    break;

    case 'k':
     {
     int i;
     char *cpos=buf;
     buf[0]=0;
     if(w->kbd->x && w->kbd->seq[0])
      for(i=0;i!=w->kbd->x;++i)
       {
       int c=w->kbd->seq[i]&127;
       if(c<32) cpos[0]='^', cpos[1]=c+'@', cpos+=2;
       else if(c==127) cpos[0]='^', cpos[1]='?', cpos+=2;
       else cpos[0]=c, cpos+=1;
       }
     *cpos++=fill;
     while(cpos-buf<4) *cpos++=fill;
     stalin=vsncpy(sv(stalin),buf,cpos-buf);
     }
    break;

    case 'S':
    if(bw->pid) stalin=vsncpy(sv(stalin),sc("*SHELL*"));
    break;

    case 'M':
    if(recmac)
     {
     sprintf(buf,"(Macro %d recording...)",recmac->n);
     stalin=vsncpy(sv(stalin),sz(buf));
     }
    break;

    default: stalin=vsadd(stalin,*s);
    }
  else stalin=vsadd(stalin,*s);
  ++s;
  }
 return stalin;
 }

static void disptw(bw,flg)
BW *bw;
 {
 W *w=bw->parent;
 TW *tw=(TW *)bw->object;

 if(bw->o.linums!=bw->linums)
  {
  bw->linums=bw->o.linums;
  resizetw(bw,w->w,w->h);
  movetw(bw,w->x,w->y);
  bwfllw(bw);
  }

 w->cury=bw->cursor->line-bw->top->line+bw->y-w->y;
 w->curx=bw->cursor->xcol-bw->offset+(bw->o.linums?LINCOLS:0);

 if((staupd || keepup || bw->cursor->line!=tw->prevline ||
     bw->b->changed!=tw->changed) && (w->y || !staen))
  {
  int fill;
  tw->prevline=bw->cursor->line;
  tw->changed=bw->b->changed;
  if(bw->o.rmsg[0]) fill=bw->o.rmsg[0];
  else fill=' ';
  tw->stalin=stagen(tw->stalin,bw,bw->o.lmsg,fill);
  tw->staright=stagen(tw->staright,bw,bw->o.rmsg,fill);
  if(fmtlen(tw->staright)<w->w)
   {
   int x=fmtpos(tw->stalin,w->w-fmtlen(tw->staright));
   if(x>sLEN(tw->stalin)) tw->stalin=vsfill(sv(tw->stalin),fill,x-sLEN(tw->stalin));
   tw->stalin=vsncpy(tw->stalin,fmtpos(tw->stalin,w->w-fmtlen(tw->staright)),sv(tw->staright));
   }
  tw->stalin=vstrunc(tw->stalin,fmtpos(tw->stalin,w->w));
  genfmt(w->t->t,w->x,w->y,0,tw->stalin,0);
  w->t->t->updtab[w->y]=0;
  }

 if(flg) bwgen(bw,bw->o.linums);
 }

/* Split current window */

void iztw(tw,y)
TW *tw;
 {
 tw->stalin=0;
 tw->staright=0;
 tw->changed= -1;
 tw->prevline= -1;
 tw->staon=(!staen || y);
 }

extern int dostaupd;

int usplitw(bw)
BW *bw;
 {
 W *w=bw->parent;
 int newh=getgrouph(w);
 W *new;
 TW *newtw;
 BW *newbw;
 dostaupd=1;
 if(newh/2<FITHEIGHT) return -1;
 new=wcreate(w->t,w->watom,findbotw(w),NULL,w,newh/2+(newh&1),NULL,NULL);
 if(!new) return -1;
 wfit(new->t);
 new->object=(void *)(newbw=bwmk(new,bw->b,0));
 ++bw->b->count;
 newbw->offset=bw->offset;
 newbw->object=(void *)(newtw=(TW *)malloc(sizeof(TW)));
 iztw(newtw,new->y);
 pset(newbw->top,bw->top);
 pset(newbw->cursor,bw->cursor);
 newbw->cursor->xcol=bw->cursor->xcol;
 new->t->curwin=new;
 return 0;
 }

int uduptw(bw)
BW *bw;
 {
 W *w=bw->parent;
 int newh=getgrouph(w);
 W *new;
 TW *newtw;
 BW *newbw;
 dostaupd=1;
 new=wcreate(w->t,w->watom,findbotw(w),NULL,NULL,newh,NULL,NULL);
 if(!new) return -1;
 if(demotegroup(w)) new->t->topwin=new;
 new->object=(void *)(newbw=bwmk(new,bw->b,0));
 ++bw->b->count;
 newbw->offset=bw->offset;
 newbw->object=(void *)(newtw=(TW *)malloc(sizeof(TW)));
 iztw(newtw,new->y);
 pset(newbw->top,bw->top);
 pset(newbw->cursor,bw->cursor);
 newbw->cursor->xcol=bw->cursor->xcol;
 new->t->curwin=new;
 wfit(w->t);
 return 0;
 }

/* User routine for aborting a text window */

int naborttw(bw,k,object,notify)
BW *bw;
void *object;
int *notify;
{
W *w=bw->parent;
B *b;
TW *tw=(TW *)bw->object;
if(notify) *notify=1;
if(k!='y' && k!='Y') return -1;

genexmsg(bw,0,NULL);

if(countmain(w->t)==1)
 if(b=borphan())
  {
  void *object=bw->object;
  bwrm(bw);
  w->object=(void *)(bw=bwmk(w,b,0));
  wredraw(bw->parent);
  bw->object=object;
  return 0;
  }
bwrm(bw);
vsrm(tw->stalin);
free(tw);
w->object=0;
wabort(w);		/* Eliminate this window and it's children */
return 0;
}

static void instw(bw,b,l,n,flg)
BW *bw;
B *b;
long l,n;
int flg;
 {
 if(b==bw->b) bwins(bw,l,n,flg);
 }

static void deltw(bw,b,l,n,flg)
BW *bw;
B *b;
long l,n;
int flg;
 {
 if(b==bw->b) bwdel(bw,l,n,flg);
 }

int uabort();

static WATOM watomtw=
{
"main",
disptw,
bwfllw,
0,
rtntw,
utypebw,
resizetw,
movetw,
instw,
deltw,
TYPETW
};

int uabort(bw,k)
BW *bw;
{
if(bw->parent->watom!=&watomtw)
 return wabort(bw->parent);
if(bw->pid && bw->cursor->byte==bw->b->eof->byte && k!= MAXINT)
 {
 char c=k;
 write(bw->out,&c,1);
 return 0;
 }
if(bw->pid) return ukillpid(bw);
if(bw->b->changed && bw->b->count==1)
 if(mkqw(bw,sc("Lose changes to this file (y,n,^C)? "),naborttw,NULL,NULL,NULL)) return 0;
 else return -1;
else return naborttw(bw,'y',NULL,NULL);
}

/* Abort buffer */

int uabortbuf(bw)
BW *bw;
 {
 W *w=bw->parent;
 B *b;
 if(bw->pid) return ukillpid(bw);

 if(okrepl(bw)) return -1;

 if(b=borphan())
  {
  void *object=bw->object;
  bwrm(bw);
  w->object=(void *)(bw=bwmk(w,b,0));
  wredraw(bw->parent);
  bw->object=object;
  return 0;
  }

 return naborttw(bw,'y',NULL,NULL);
 }

/* Kill this window */

int utw0(b)
BASE *b;
 {
 BW *bw=b->parent->main->object;
 if(countmain(b->parent->t)==1) return -1;
 if(bw->pid) { return ukillpid(bw); }
 if(bw->b->count==1) orphit(bw);
 return uabort(bw,MAXINT);
 }

/* Only one window */

int utw1(b)
BASE *b;
 {
 W *starting=b->parent;
 W *main=starting->main;
 SCREEN *t=main->t;
 int yn;

 do
  {
  yn=0;
  loop:
  do wnext(t); while(t->curwin->main==main && t->curwin!=starting);
  if(t->curwin->main!=main)
   {
   if(((BW *)t->curwin->main->object)->pid)
    {
    msgnw(t->curwin->main->object,"Process running in this window");
    return -1;
    }
   utw0(t->curwin->main->object), yn=1;
   goto loop;
   }
  } while(yn);
 return 0;
 }

void setline(b,line)
B *b;
long line;
 {
 W *w=maint->curwin;
 do
  if(w->watom->what==TYPETW)
   {
   BW *bw=w->object;
   if(bw->b==b)
    {
    long oline=bw->top->line;
    pline(bw->top,line);
    pline(bw->cursor,line);
    if(w->y>=0 && bw->top->line>oline && bw->top->line-oline<bw->h)
     nscrlup(w->t->t,bw->y,bw->y+bw->h,(int)(bw->top->line-oline));
    else if(w->y>=0 && bw->top->line<oline && oline-bw->top->line<bw->h)
     nscrldn(w->t->t,bw->y,bw->y+bw->h,(int)(oline-bw->top->line));
    }
   }
  while((w=w->link.next)!=maint->curwin);
 }

/* Create a text window.  It becomes the last window on the screen */

BW *wmktw(t,b)
SCREEN *t;
B *b;
 {
 W *w;
 BW *bw;
 TW *tw;
 w=wcreate(t,&watomtw,NULL,NULL,NULL,t->h,NULL,NULL);
 wfit(w->t);
 w->object=(void *)(bw=bwmk(w,b,0));
 bw->object=(void *)(tw=(TW *)malloc(sizeof(TW)));
 iztw(tw,w->y);
 return bw;
 }
