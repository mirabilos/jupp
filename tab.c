/* File selection menu
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
#include <sys/types.h>
#include <sys/stat.h>
#include "config.h"
#include "scrn.h"
#include "kbd.h"
#include "vs.h"
#include "w.h"
#include "bw.h"
#include "zstr.h"
#include "path.h"
#include "va.h"
#include "menu.h"
#include "tty.h"
#include "blocks.h"
#include "tab.h"

typedef struct tab TAB;

extern int smode;

struct tab
 {
 char *path;	/* Current directory */
 char *pattern;	/* Search pattern */
 int len;	/* No. entries in files */
 char **files;	/* Array of file names */
 char **list;
 char *type;	/* File type array */
 int prv;
 char *orgpath;
 char *orgnam;
 };

/* Type codes for file type array */

#define F_DIR		1
#define F_NORMAL	2
#define F_EXEC		4

/* Read matching files from a directory
 *  Directory is given in tab.path
 *  Pattern is given in tab.pattern
 *
 * Returns with -1 if there was an error
 * Otherwise returns index to file with inode given in prv
 * len and files are set with the file names
 * type is set with the file types
 */

static int get_entries(tab,prv)
TAB *tab;
 {
 int a;
 int which=0;
 char *oldpwd=pwd();
 char **files;
 if(chpwd(tab->path)) return -1;
 files=(char **)rexpnd(tab->pattern);
 if(!files)
  {
  chpwd(oldpwd);
  return -1;
  }
 if(!aLEN(files))
  {
  chpwd(oldpwd);
  return -1;
  }
 tab->len=aLEN(files);
 varm(tab->files); tab->files=files;
 vasort(files,tab->len);
 if(tab->type) free(tab->type);
 tab->type=(char *)malloc(tab->len);
 for(a=0;a!=tab->len;a++)
  {
  struct stat buf;
  mset(&buf,0,sizeof(struct stat));
  stat(files[a],&buf);
  if(buf.st_ino==prv) which=a;
  if((buf.st_mode&S_IFMT)==S_IFDIR) tab->type[a]=F_DIR;
  else if(buf.st_mode&(0100|0010|0001)) tab->type[a]=F_EXEC;
  else tab->type[a]=F_NORMAL;
  }
 chpwd(oldpwd);
 return which;
 }

void insnam(bw,path,nam)
BW *bw;
char *path, *nam;
 {
 P *p=pdup(bw->cursor); pbol(p);
 peol(bw->cursor);
 bdel(p,bw->cursor);
 if(sLEN(path))
  {
  binsm(bw->cursor,sv(path)), peol(bw->cursor);
  if(path[sLEN(path)-1]!='/')
   binsm(bw->cursor,sc("/")), peol(bw->cursor);
  }
 binsm(bw->cursor,sv(nam)); peol(bw->cursor);
 prm(p);
 bw->cursor->xcol=piscol(bw->cursor);
 }

/* Given a menu structure with a tab structure as its object,
 * a pattern and path set in the tab structure:
 *
 * Load the menu with a list of file names and set the file name in
 * the prompt window to the directory the menu was read in from.
 * If flg is set, treload attempts to position to the previous directory
 * which was visited.
 *
 * Returns with -1 if there was an error
 * Returns with 0 for success
 */

int treload(m,flg)
MENU *m;
 {
 TAB *tab=(TAB *)m->object;	/* The menu */
 W *w=m->parent;		/* Window menu is in */
 BW *bw=(BW *)w->win->object;	/* The prompt window */
 int x;
 int which;
 struct stat buf;

 if((which=get_entries(tab,tab->prv))<0) return -1;
 if(tab->path && tab->path[0]) stat(tab->path,&buf);
 else stat(".",&buf);
 tab->prv=buf.st_ino;
 if(!flg) which=0;

 tab->list=vatrunc(tab->list,aLEN(tab->files));

 for(x=0;tab->files[x];++x)
  {
  char *s=vsncpy(NULL,0,sv(tab->files[x]));
  tab->list=vaset(tab->list,x,s);
  if(tab->type[x]==F_DIR)
   tab->list[x]=vsadd(tab->list[x],'/');
  else if(tab->type[x]==F_EXEC)
   tab->list[x]=vsadd(tab->list[x],'*');
  }
 ldmenu(m,tab->list,which);
 insnam(bw,tab->path,tab->pattern);
 return 0;
 }
 
void rmtab(tab)
TAB *tab;
 {
 vsrm(tab->orgpath);
 vsrm(tab->orgnam);
 varm(tab->list);
 vsrm(tab->path);
 vsrm(tab->pattern);
 varm(tab->files);
 if(tab->type) free(tab->type);
 free(tab);
 }

/* The user hit return */

int tabrtn(m,cursor,tab)
MENU *m;
TAB *tab;
 {
 if(tab->type[cursor]==F_DIR)
  { /* Switch directories */
  char *orgpath=tab->path;
  char *orgpattern=tab->pattern;
  char *e=endprt(tab->path);
  if(!zcmp(tab->files[cursor],"..") && sLEN(e) &&
     !(e[0]=='.' && e[1]=='.' && (!e[2] || e[2]=='/')))
   tab->path=begprt(tab->path);
  else
   {
   tab->path=vsncpy(NULL,0,sv(tab->path));
   tab->path=vsncpy(sv(tab->path),sv(m->list[cursor]));
   }
  vsrm(e);
  tab->pattern=vsncpy(NULL,0,sc("*"));
  if(treload(m,0))
   {
   msgnw(m,"Couldn't read directory ");
   vsrm(tab->pattern); tab->pattern=orgpattern;
   vsrm(tab->path); tab->path=orgpath;
   return -1;
   }
  else
   {
   vsrm(orgpattern);
   vsrm(orgpath);
   return 0;
   }
  }
 else
  { /* Select name */
  BW *bw=m->parent->win->object;
  insnam(bw,tab->path,tab->files[cursor]);
  rmtab(tab);
  m->object=0; m->abrt=0;
  wabort(m->parent);
  return 0;
  }
 }

/* The user hit backspace */

int tabbacks(m,cursor,tab)
MENU *m;
TAB *tab;
 {
 char *orgpath=tab->path;
 char *orgpattern=tab->pattern;
 char *e=endprt(tab->path);
 if(sLEN(e)) tab->path=begprt(tab->path);
 else
  {
  wabort(m->parent);
  return 0;
  }
 vsrm(e);
 tab->pattern=vsncpy(NULL,0,sc("*"));

 if(treload(m,1))
  {
  msgnw(m,"Couldn't read directory ");
  vsrm(tab->pattern); tab->pattern=orgpattern;
  vsrm(tab->path); tab->path=orgpath;
  return -1;
  }
 else
  {
  vsrm(orgpattern);
  vsrm(orgpath);
  return 0;
  }
 }

int tababrt(bw,cursor,tab)
BW *bw;
TAB *tab;
 {
 insnam(bw,tab->orgpath,tab->orgnam);
 rmtab(tab);
 return -1;
 }

/* Create a tab window */

int cmplt(bw)
BW *bw;
 {
 MENU *new;
 TAB *tab;
 P *p, *q;
 char *cline, *tmp;
 long a,b;

 tab=(TAB *)malloc(sizeof(TAB));
 new=mkmenu(bw,NULL,tabrtn,tababrt,tabbacks,0,tab,NULL);
 if(!new)
  {
  free(tab);
  return -1;
  }

 tab->files=0;
 tab->type=0;
 tab->list=0;
 tab->prv=0;
 tab->len=0;

 p=pdup(bw->cursor); pbol(p);
 q=pdup(bw->cursor); peol(q);
 tmp=brvs(p,(int)(q->byte-p->byte));
 cline=parsens(tmp,&a,&b);
 vsrm(tmp);
 prm(p); prm(q);

 tab->pattern=namprt(cline);
 tab->path=dirprt(cline);
 tab->orgnam=vsncpy(NULL,0,sv(tab->pattern));
 tab->orgpath=vsncpy(NULL,0,sv(tab->path));
 tab->pattern=vsadd(tab->pattern,'*');
 vsrm(cline);

 if(treload(new,0))
  {
  wabort(new->parent);
  ttputc(7);
  return -1;
  }
 else
  if(sLEN(tab->files)==1) return tabrtn(new,0,tab);
  else if(smode || isreg(tab->orgnam)) return 0;
  else
   {
   char *com=mcomplete(new);
   vsrm(tab->orgnam);
   tab->orgnam=com;
   wabort(new->parent);
   smode=2;
   ttputc(7);
   return 0;
   }
 }
