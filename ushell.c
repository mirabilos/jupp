/* Shell-window functions
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
#include "b.h"
#include "bw.h"
#include "w.h"
#include "pw.h"
#include "qw.h"
#include "vs.h"
#include "va.h"
#include "ufile.h"
#include "main.h"
#include "ushell.h"

extern int orphan;

/* Executed when shell process terminates */

static void cdone(bw)
BW *bw;
 {
 bw->pid=0;
 close(bw->out); bw->out= -1;
 if(piseof(bw->cursor))
  {
  binss(bw->cursor,"** Program finished **\n");
  peof(bw->cursor);
  bw->cursor->xcol=piscol(bw->cursor);
  }
 else
  {
  P *q=pdup(bw->b->eof);
  binss(q,"** Program finished **\n");
  prm(q);
  }
 }

/* Executed for each chunk of data we get from the shell */

static void cdata(bw,dat,siz)
BW *bw;
char *dat;
 {
 P *q=pdup(bw->cursor);
 P *r=pdup(bw->b->eof);
 char bf[1024];
 int x, y;
 for(x=y=0;x!=siz;++x)
  if(dat[x]==13 || dat[x]==0);
  else if(dat[x]==8 || dat[x]==127)
   if(y) --y;
   else
    if(piseof(bw->cursor))
     {
     pset(q,bw->cursor), prgetc(q), bdel(q,bw->cursor);
     bw->cursor->xcol=piscol(bw->cursor);
     }
    else pset(q,r), prgetc(q), bdel(q,r);
  else bf[y++]=dat[x];
 if(y)
  if(piseof(bw->cursor))
   {
   binsm(bw->cursor,bf,y);
   peof(bw->cursor);
   bw->cursor->xcol=piscol(bw->cursor);
   }
  else binsm(r,bf,y);
 prm(r);
 prm(q);
 }

static int cstart(bw,name,s,obj,notify)
BW *bw;
char *name;
char **s;
void *obj;
int *notify;
 {
#ifdef __MSDOS__
 if(notify) *notify=1;
 varm(s);
 msgnw(bw,"Sorry, no sub-processes in DOS (yet)");
 return -1;
#else
 MPX *m;
 if(notify) *notify=1;
 if(bw->pid && orphan)
  {
  msgnw(bw,"Program already running in this window");
  varm(s);
  return -1;
  }
 if(doedit(bw,vsncpy(NULL,0,sc("")),NULL,NULL))
  {
  varm(s);
  return -1;
  }
 bw=(BW *)maint->curwin->object;
 if(!(m=mpxmk(&bw->out,name,s,cdata,bw,cdone,bw)))
  {
  varm(s);
  msgnw(bw,"No ptys available");
  return -1;
  }
 else bw->pid= m->pid;
 return 0;
#endif
 }

int ubknd(bw)
BW *bw;
 {
 char **a;
 char *s;
 a=vamk(3);
 s=vsncpy(NULL,0,sz(getenv("SHELL"))); a=vaadd(a,s);
 s=vsncpy(NULL,0,sc("-i")); a=vaadd(a,s);
 return cstart(bw,getenv("SHELL"),a,NULL,NULL);
 }

/* Run a program in a window */

static int dorun(bw,s,object,notify)
BW *bw;
char *s;
void *object;
int *notify;
 {
 char **a=vamk(10);
 char *cmd=vsncpy(NULL,0,sc("/bin/sh"));
 a=vaadd(a,cmd);
 cmd=vsncpy(NULL,0,sc("-c"));
 a=vaadd(a,cmd);
 a=vaadd(a,s);
 return cstart(bw,"/bin/sh",a,NULL,notify);
 }

B *runhist=0;

int urun(bw)
BW *bw;
 {
 if(wmkpw(bw,"Program to run: ",&runhist,dorun,"Run",NULL,NULL,NULL,NULL))
  return 0;
 else return -1;
 }

/* Kill program */

int pidabort(bw,c,object,notify)
BW *bw;
void *object;
int *notify;
 {
 if(notify) *notify=1;
 if(c!='y' && c!='Y') return -1;
 if(bw->pid) { kill(bw->pid,1); return -1; }
 else return -1;
 }

int ukillpid(bw)
BW *bw;
 {
 if(bw->pid)
  {
  if(mkqw(bw,sc("Kill program (y,n,^C)?"),pidabort,NULL,NULL,NULL)) return 0;
  else return -1;
  }
 else return 0;
 }
