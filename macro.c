/* Keyboard macros
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

#include "main.h"
#include "qw.h"
#include "pw.h"
#include "bw.h"
#include "vs.h"
#include "undo.h"
#include "cmd.h"
#include "ublock.h"
#include "umath.h"
#include "uedit.h"
#include "zstr.h"
#include "macro.h"

MACRO *freemacros=0;

/* Create a macro */

MACRO *mkmacro(k,arg,n,cmd)
CMD *cmd;
 {
 MACRO *macro;
 if(!freemacros)
  {
  int x;
  macro=(MACRO *)malloc(sizeof(MACRO)*64);
  for(x=0;x!=64;++x)
   macro[x].steps=(MACRO **)freemacros,
   freemacros=macro+x;
  }
 macro=freemacros;
 freemacros=(MACRO *)macro->steps;
 macro->steps=0;
 macro->size=0;
 macro->arg=arg;
 macro->n=n;
 macro->cmd=cmd;
 macro->k=k;
 return macro;
 }

/* Eliminate a macro */

void rmmacro(macro)
MACRO *macro;
 {
 if(macro)
  {
  if(macro->steps)
   {
   int x;
   for(x=0;x!=macro->n;++x) rmmacro(macro->steps[x]);
   free(macro->steps);
   }
  macro->steps=(MACRO **)freemacros;
  freemacros=macro;
  }
 }

/* Add a step to block macro */

void addmacro(macro,m)
MACRO *macro, *m;
 {
 if(macro->n==macro->size)
  if(macro->steps)
   macro->steps=(MACRO **)realloc(macro->steps,(macro->size+=8)*sizeof(MACRO *));
  else
   macro->steps=(MACRO **)malloc((macro->size=8)*sizeof(MACRO *));
 macro->steps[macro->n++]=m;
 }

/* Duplicate a macro */

MACRO *dupmacro(mac)
MACRO *mac;
 {
 MACRO *m=mkmacro(mac->k,mac->arg,mac->n,mac->cmd);
 if(mac->steps)
  {
  int x;
  m->steps=(MACRO **)malloc((m->size=mac->n)*sizeof(MACRO *));
  for(x=0;x!=m->n;++x) m->steps[x]=dupmacro(mac->steps[x]);
  }
 return m;
 }

/* Set key part of macro */

MACRO *macstk(m,k)
MACRO *m;
 {
 m->k=k;
 return m;
 }

/* Set arg part of macro */

MACRO *macsta(m,a)
MACRO *m;
 {
 m->arg=a;
 return m;
 }

/* Parse text into a macro
 * sta is set to:  ending position in buffer for no error.
 *                 -1 for syntax error
 *                 -2 for need more input
 */

MACRO *mparse(m,buf,sta)
MACRO *m;
char *buf;
int *sta;
 {
 int y, c, x=0;

 macroloop:

 /* Skip whitespace */
 while(cwhite(buf[x])) ++x;

 /* Do we have a string? */
 if(buf[x]=='\"')
  {
  ++x;
  while(buf[x] && buf[x]!='\"')
   {
   if(buf[x]=='\\' && buf[x+1])
    {
    ++x;
    switch(buf[x])
     {
    case 'n': buf[x]=10; break;
    case 'r': buf[x]=13; break;
    case 'b': buf[x]=8; break;
    case 'f': buf[x]=12; break;
    case 'a': buf[x]=7; break;
    case 't': buf[x]=9; break;
    case 'x':
     c=0;
     if(buf[x+1]>='0' && buf[x+1]<='9') c=c*16+buf[++x]-'0';
     else if(buf[x+1]>='a' && buf[x+1]<='f' ||
             buf[x+1]>='A' && buf[x+1]<='F') c=c*16+(buf[++x]&0xF)+9;
     if(buf[x+1]>='0' && buf[x+1]<='9') c=c*16+buf[++x]-'0';
     else if(buf[x+1]>='a' && buf[x+1]<='f' ||
             buf[x+1]>='A' && buf[x+1]<='F') c=c*16+(buf[++x]&0xF)+9;
     buf[x]=c;
     break;
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
    case '8': case '9':
     c=buf[x]-'0';
     if(buf[x+1]>='0' && buf[x+1]<='7') c=c*8+buf[++x]-'0';
     if(buf[x+1]>='0' && buf[x+1]<='7') c=c*8+buf[++x]-'0';
     buf[x]=c;
     break;
     }
    }
   if(m)
    {
    if(!m->steps)
     {
     MACRO *macro=m;
     m=mkmacro(MAXINT,1,0,NULL);
     addmacro(m,macro);
     }
    }
   else m=mkmacro(MAXINT,1,0,NULL);
   addmacro(m,mkmacro(buf[x],1,0,findcmd("type")));
   ++x;
   }
  if(buf[x]=='\"') ++x;
  }

 /* Do we have a command? */
 else
  {
  for(y=x;
      buf[y] && buf[y]!=',' && buf[y]!=' ' && buf[y]!='\t' && buf[y]!='\n' && buf[x]!='\r';
      ++y);
  if(y!=x)
   {
   CMD *cmd;
   c=buf[y]; buf[y]=0;
   cmd=findcmd(buf+x);
   if(!cmd)
    {
    *sta = -1;
    return 0;
    }
   else if(m)
    {
    if(!m->steps)
     {
     MACRO *macro=m;
     m=mkmacro(MAXINT,1,0,NULL);
     addmacro(m,macro);
     }
    addmacro(m,mkmacro(MAXINT,1,0,cmd));
    }
   else m=mkmacro(MAXINT,1,0,cmd);
   buf[x=y]=c;
   }
  }

 /* Skip whitespace */
 while(cwhite(buf[x])) ++x;

 /* Do we have a comma? */
 if(buf[x]==',')
  {
  ++x;
  while(cwhite(buf[x])) ++x;
  if(buf[x] && buf[x]!='\r' && buf[x]!='\n') goto macroloop;
  *sta= -2;
  return m;
  }

 /* Done */
 *sta=x;
 return m;
 }

/* Convert macro to text */

static char *ptr;
static int first;
static int instr;

char *unescape(ptr,c)
char *ptr;
 {
 if(c=='"') *ptr++='\\', *ptr++='"';
 else if(c=='\'') *ptr++='\\', *ptr++='\'';
 else if(c<32 || c>126)
  {
  *ptr++='\\';
  *ptr++='x';
  *ptr++="0123456789ABCDEF"[c>>4];
  *ptr++="0123456789ABCDEF"[c&15];
  }
 else *ptr++=c;
 return ptr;
 }

void domtext(m)
MACRO *m;
 {
 int x;
 if(!m) return;
 if(m->steps)
  for(x=0;x!=m->n;++x) domtext(m->steps[x]);
 else
  {
  if(instr && zcmp(m->cmd->name,"type")) *ptr++='\"', instr=0;
  if(first) first=0;
  else if(!instr) *ptr++=',';
  if(!zcmp(m->cmd->name,"type"))
   {
   if(!instr) *ptr++='\"', instr=1;
   ptr=unescape(ptr,m->k);
   }
  else
   {
   for(x=0;m->cmd->name[x];++x) *ptr++=m->cmd->name[x];
   if(!zcmp(m->cmd->name,"play") ||
      !zcmp(m->cmd->name,"gomark") ||
      !zcmp(m->cmd->name,"setmark") ||
      !zcmp(m->cmd->name,"record") ||
      !zcmp(m->cmd->name,"uarg"))
    {
    *ptr++=',';
    *ptr++='"';
    ptr=unescape(ptr,m->k);
    *ptr++='"';
    }
   }
  }
 }

char *mtext(s,m)
char *s;
MACRO *m;
 {
 ptr=s;
 first=1;
 instr=0;
 domtext(m);
 if(instr) *ptr++='\"';
 *ptr=0;
 return s;
 }

/* Keyboard macro recorder */

static MACRO *kbdmacro[10];
static int playmode[10];

struct recmac *recmac=0;

static void unmac()
 {
 if(recmac) rmmacro(recmac->m->steps[--recmac->m->n]);
 }

void chmac()
 {
 if(recmac && recmac->m->n) recmac->m->steps[recmac->m->n-1]->k=3;
 }

static void record(m)
MACRO *m;
 {
 if(recmac) addmacro(recmac->m,dupmacro(m));
 }

/* Query for user input */

int uquery(bw)
BW *bw;
 {
 int ret;
 struct recmac *tmp=recmac;
 recmac=0;
 ret=edloop(1);
 recmac=tmp;
 return ret;
 }

/* Macro execution */

MACRO *curmacro=0;		/* Set if we're in a macro */
static int macroptr;
static int arg=0;		/* Repeat argument */
static int argset=0;		/* Set if 'arg' is set */

int exmacro(m,u)
MACRO *m;
 {
 int larg;
 int negarg=0;
 int flg=0;
 CMD *cmd;
 int n;
 int ret=0;

 if(argset)
  {
  larg=arg;
  arg=0;
  argset=0;
  if(larg<0) negarg=1, larg= -larg;
  if(m->steps) negarg=0;
  else
   {
   cmd=m->cmd;
   if(!cmd->arg) larg=0;
   else if(negarg)
    if(cmd->negarg) cmd=findcmd(cmd->negarg);
    else larg=0;
   }
  }
 else
  {
  cmd=m->cmd;
  larg=1;
  }

 if( m->steps ||
     larg!=1 ||
     !(cmd->flag&EMINOR) ||
     maint->curwin->watom->what==TYPEQW		/* Undo work right for s & r */
   ) flg=1;

 if(flg && u) umclear();
 while(larg-- && !leave && !ret)
  if(m->steps)
   {
   MACRO *tmpmac=curmacro;
   int tmpptr=macroptr;
   int x=0;
   int stk=nstack;
   while(m && x!=m->n && !leave && !ret)
    {
    MACRO *d;
    d=m->steps[x++];
    curmacro=m;
    macroptr=x;
    ret=exmacro(d,0);
    m=curmacro;
    x=macroptr;
    }
   curmacro=tmpmac;
   macroptr=tmpptr;
   while(nstack>stk) upop(NULL);
   }
  else ret=execmd(cmd,m->k);
 if(leave) return ret;
 if(flg && u) umclear();

 if(u) undomark();

 return ret;
 }

/* Execute a macro */

int exemac(m)
MACRO *m;
 {
 record(m);
 return exmacro(m,1);
 }

/* Keyboard macro user routines */

static int dorecord(bw,c,object,notify)
BW *bw;
void *object;
int *notify;
 {
 int n;
 struct recmac *r;
 if(notify) *notify=1;
 if(c>'9' || c<'0')
  {
  nungetc(c);
  return -1;
  }
 for(n=0;n!=10;++n) if(playmode[n]) return -1;
 r=(struct recmac *)malloc(sizeof(struct recmac));
 r->m=mkmacro(0,1,0,NULL);
 r->next=recmac;
 r->n=c-'0';
 recmac=r;
 return 0;
 }

int urecord(bw,c)
BW *bw;
 {
 if(c>='0' && c<='9') return dorecord(bw,c,NULL,NULL);
 else
  if(mkqw(bw,sc("Macro to record (0-9 or ^C to abort): "),dorecord,NULL,NULL,NULL)) return 0;
  else return -1;
 }

extern int dostaupd;

int ustop()
 {
 unmac();
 if(recmac)
  {
  struct recmac *r=recmac;
  MACRO *m;
  dostaupd=1;
  recmac=r->next;
  if(kbdmacro[r->n]) rmmacro(kbdmacro[r->n]);
  kbdmacro[r->n]=r->m;
  if(recmac) record(m=mkmacro(r->n+'0',1,0,findcmd("play"))), rmmacro(m);
  free(r);
  }
 return 0;
 }

int doplay(bw,c,object,notify)
BW *bw;
void *object;
int *notify;
 {
 if(notify) *notify=1;
 if(c>='0' && c<='9')
  {
  int ret;
  c-='0';
  if(playmode[c] || !kbdmacro[c]) return -1;
  playmode[c]=1;
  ret=exmacro(kbdmacro[c],0);
  playmode[c]=0;
  return ret;
  }
 else
  {
  nungetc(c);
  return -1;
  }
 }

int umacros(bw)
BW *bw;
 {
 int x;
 char buf[1024];
 peol(bw->cursor);
 for(x=0;x!=10;++x) if(kbdmacro[x])
  {
  mtext(buf,kbdmacro[x]);
  binss(bw->cursor,buf);
  peol(bw->cursor);
  sprintf(buf,"\t^K %c\tMacro %d",x+'0',x);
  binss(bw->cursor,buf);
  peol(bw->cursor);
  binsc(bw->cursor,'\n');
  pgetc(bw->cursor);
  }
 return 0;
 }

int uplay(bw,c)
BW *bw;
 {
 if(c>='0' && c<='9') return doplay(bw,c,NULL,NULL);
 else
  if(mkqwna(bw,sc("Play-"),doplay,NULL,NULL,NULL)) return 0;
  else return -1;
 }

/* Repeat-count setting */

static int doarg(bw,s,object,notify)
BW *bw;
char *s;
void *object;
int *notify;
 {
 long num;
 if(notify) *notify=1;
 num=calc(bw,s);
 if(merr) { msgnw(bw,merr); return -1; }
 arg=num;
 argset=1;
 vsrm(s);
 return 0;
 }

int uarg(bw)
BW *bw;
 {
 if(wmkpw(bw,
          "No. times to repeat next command (^C to abort): ",
          NULL,doarg,NULL,NULL,utypebw,NULL,NULL)) return 0;
 else return -1;
 }

int unaarg;
int negarg;

int douarg(bw,c,object,notify)
BW *bw;
void *object;
int *notify;
 {
 if(c=='-') negarg= !negarg;
 else if(c>='0' && c<='9') unaarg=unaarg*10+c-'0';
 else if(c=='U'-'@')
  if(unaarg) unaarg*=4;
  else unaarg=16;
 else if(c==7 || c==3 || c==32)
  {
  if(notify) *notify=1;
  return -1;
  }
 else
  {
  nungetc(c);
  if(unaarg) arg=unaarg;
  else if(negarg) arg=1;
  else arg=4;
  if(negarg) arg= -arg;
  argset=1;
  if(notify) *notify=1;
  return 0;
  }
 sprintf(msgbuf,"Repeat %s%d",negarg?"-":"",unaarg);
 if(mkqwna(bw,sz(msgbuf),douarg,NULL,NULL,notify)) return 0;
 else return -1;
 }

int uuarg(bw,c)
BW *bw;
 {
 unaarg=0;
 negarg=0;
 if(c>='0' && c<='9' || c=='-') return douarg(bw,c,NULL,NULL);
 else
  if(mkqwna(bw,sc("Repeat"),douarg,NULL,NULL,NULL)) return 0;
  else return -1;
 }
