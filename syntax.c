/*
 *	Syntax highlighting DFA interpreter
 *	Copyright
 *		(C) 2004 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

#include "config.h"
#include "types.h"

__RCSID("$MirOS: contrib/code/jupp/syntax.c,v 1.25 2018/11/11 18:15:38 tg Exp $");

#include <stdlib.h>
#include <string.h>
#include "b.h"
#include "scrn.h"
#include "utils.h"
#include "hash.h"
#include "path.h"
#include "charmap.h"
#include "syntax.h"

static struct {
	unsigned char buf[7];
	unsigned char start;
	unsigned char limit;
	unsigned eaten : 1;
	unsigned ebbed : 1;
	unsigned unget : 1;
	unsigned first : 1;
} utfstate;

static int
utfoctet(P *p)
{
	int c;

	utfstate.first = 0;
	if (utfstate.eaten) {
 ate:
		if (utfstate.start < utfstate.limit)
			return (utfstate.buf[utfstate.start++]);
		if (utfstate.ebbed)
			return (NO_MORE_DATA);
		utfstate.eaten = utfstate.limit = 0;
	}
	if (!utfstate.limit) {
		utfstate.first = 1;
		if (utfstate.unget) {
			c = utfstate.buf[utfstate.start];
			utfstate.unget = 0;
		} else
			c = pgetb(p);
		if ((c == NO_MORE_DATA) || (c < 0x80))
			return (c);
		if ((c < 0xC2) || (c >= 0xFE))
			return (0xFF);
		utfstate.start = 0;
		utfstate.buf[utfstate.start++] = (unsigned char)c;
		utfstate.limit = (c < 0xE0) ? 2 : (c < 0xF0) ? 3 :
		    (c < 0xF8) ? 4 : (c < 0xFC) ? 5 : 6;
	}
	while (utfstate.start < utfstate.limit) {
		if (((c = pgetb(p)) == NO_MORE_DATA) || ((c ^ 0x80) > 0x3F)) {
			/* invalid follow byte, invalidate all previous ones */
			utfstate.limit = 0;
			while (utfstate.limit < utfstate.start)
				utfstate.buf[utfstate.limit++] = 0xFF;
			/* append this as ungetch unless the well is dry */
			if (c == NO_MORE_DATA)
				utfstate.ebbed = 1;
			else {
				utfstate.buf[utfstate.limit] = (unsigned char)c;
				utfstate.unget = 1;
			}
			/* now return those bytes */
			break;
		}
		utfstate.buf[utfstate.start++] = (unsigned char)c;
	}
	utfstate.start = 0;
	utfstate.eaten = 1;
	goto ate;
}

static int
octetutf(P *p)
{
	int c;

	utfstate.first = 0;
	if (!(utfstate.start < utfstate.limit)) {
		if ((c = pgetb(p)) == NO_MORE_DATA)
			return (NO_MORE_DATA);

		utfstate.limit = utf8_encode(utfstate.buf,
		    to_uni(p->b->o.charmap, c));
		utfstate.start = 0;
		utfstate.first = 1;
	}
	return (utfstate.buf[utfstate.start++]);
}

/* Parse one line.  Returns new state.
   'syntax' is the loaded syntax definition for this buffer.
   'line' is advanced to start of next line.
   Global array 'attr_buf' end up with coloring for each character of line.
   'state' is initial parser state for the line (0 is initial state).
*/

int *attr_buf = 0;
int attr_size = 0;

int parse(struct high_syntax *syntax, P *line, int state)
{
	struct high_state *h = syntax->states[state];
			/* Current state */
	unsigned char buf[20];	/* Name buffer (trunc after 19 characters) */
	int buf_idx = 0;	/* Index into buffer */
	int buf_len = 0;	/* counts only starting characters */
	int buf_en = 0;		/* Set for name buffering */
	int *attr_end = attr_buf+attr_size;
	int *attr = attr_buf;
	int c;			/* Current character */
	int ofst = 0;	/* record length after we've stopped buffering */
	int (*getoctet)(P *) = line->b->o.charmap->type ? utfoctet : octetutf;

	memset(&utfstate, 0, sizeof(utfstate));
	buf[0] = 0;

	/* Get next character */
	while((c = getoctet(line)) != NO_MORE_DATA) {
		struct high_cmd *cmd, *kw_cmd;
		int x;

		/* Expand attribute array if necessary */
		if(attr==attr_end) {
			attr_buf = realloc(attr_buf,
			    sizeof(int) * (attr_size * 2));
			attr = attr_buf + attr_size;
			attr_size *= 2;
			attr_end = attr_buf + attr_size;
		}

		/* Advance to next attribute position (note attr[-1] below) */
		if (utfstate.first)
			attr++;

		/* Loop while noeat */
		do {
			/* Color with current state */
			attr[-1] = h->color;
			/* Get command for this character */
			cmd = h->cmd[c];
			/* Determine new state */
			if (cmd->keywords && (cmd->ignore ?
			    (kw_cmd = htfind(cmd->keywords, joe_strtolower(buf))) :
			    (kw_cmd = htfind(cmd->keywords, buf)))) {
				cmd = kw_cmd;
				h = cmd->new_state;
				/* Recolor keyword */
				for (x = -(buf_len + 1); x < -1; ++x)
					attr[x - ofst] = h->color;
			} else {
				h = cmd->new_state;
			}
			/* Recolor if necessary */
			x = cmd->recolor;
			while (&attr[x] < attr_buf)
				++x;
			while (x < 0)
				attr[x++] = h->color;

			/* Start buffering? */
			if (cmd->start_buffering) {
				buf_idx = 0;
				buf_len = 0;
				buf_en = 1;
				ofst = 0;
			}

			/* Stop buffering? */
			if (cmd->stop_buffering)
				buf_en = 0;
		} while(cmd->noeat);

		/* Save character in buffer */
		if (!buf_en)
			ofst += utfstate.first;
		else if (buf_idx < 19) {
			buf[buf_idx++] = c;
			buf[buf_idx] = 0;
			buf_len += utfstate.first;
		}

		if (c == '\n')
			break;
	}
	/* Return new state number */
	return h->no;
}

/* Subroutines for load_dfa() */

static struct high_state *find_state(struct high_syntax *syntax, const unsigned char *name)
{
	int x;
	struct high_state *state;

	/* Find state */
	for(x=0;x!=syntax->nstates;++x)
		if(!strcmp(syntax->states[x]->name,name))
			break;

	/* It doesn't exist, so create it */
	if(x==syntax->nstates) {
		int y;
		state = malloc(sizeof(struct high_state));
		state->name=(const unsigned char *)strdup((const char *)name);
		state->no=syntax->nstates;
		state->color=FG_WHITE;
		if(!syntax->nstates)
			/* We're the first state */
			syntax->default_cmd.new_state = state;
		if(syntax->nstates==syntax->szstates)
			syntax->states = realloc(syntax->states,
			   sizeof(struct high_state *) * (syntax->szstates *= 2));
		syntax->states[syntax->nstates++]=state;
		for(y=0; y!=256; ++y)
			state->cmd[y] = &syntax->default_cmd;
	} else
		state = syntax->states[x];
	return state;
}

/* Load syntax file */

struct high_syntax *syntax_list;

struct high_syntax *load_dfa(const unsigned char *name)
{
	unsigned char buf[1024];
	unsigned char bf[256];
	unsigned char bf1[256];
	int clist[256];
	unsigned char *p;
	int c;
	FILE *f = NULL;
	struct high_state *state=0;	/* Current state */
	struct high_syntax *syntax;	/* New syntax table */
	int line = 0;
	void *np;

	if (!name)
		return NULL;

	if (!attr_buf)
		attr_buf = ralloc((size_t)(attr_size = 1024), sizeof(int));

	/* Find syntax table */

	/* Already loaded? */
	for(syntax=syntax_list;syntax;syntax=syntax->next)
		if(!strcmp(syntax->name,name))
			return syntax;

	/* Load it */
	p = (unsigned char *)getenv("HOME");
	if (p) {
		joe_snprintf_2((char *)buf,sizeof(buf),"%s/.jupp/syntax/%s.jsf",p,name);
		f = fopen((char *)buf,"r");
	}

	if (!f && has_JOERC) {
		joe_snprintf_2((char *)buf,sizeof(buf),"%ssyntax/%s.jsf",get_JOERC,name);
		f = fopen((char *)buf,"r");
	}
	if(!f)
		return 0;

	/* Create new one */
	syntax = calloc(1, sizeof(struct high_syntax));
	syntax->name = (const unsigned char *)strdup((const char *)name);
	syntax->next = syntax_list;
	syntax_list = syntax;
	syntax->states = ralloc((size_t)(syntax->szstates = 64),
	    sizeof(struct high_state *));
	syntax->sync_lines = 120;

	memset(clist, 0, sizeof(clist));

	/* Parse file */
	while(fgets((char *)buf,1023,f)) {
		++line;
		p = buf;
		parse_ws(&p,'#');
		if(!parse_char(&p, ':')) {
			if(!parse_ident(&p, bf, 255)) {

				state = find_state(syntax,bf);

				parse_ws(&p,'#');
				if(!parse_ident(&p,bf,255)) {
					struct high_color *color;
					for(color=syntax->color;color;color=color->next)
						if(!strcmp(color->name,bf))
							break;
					if(color)
						state->color=color->color;
					else {
						state->color=0;
						fprintf(stderr,"%s:%d: Unknown class '%s'\n", name, line, bf);
					}
				} else
					fprintf(stderr,"%s:%d: Missing color for state definition\n", name, line);
			} else
				fprintf(stderr,"%s:%d: Missing state name\n", name, line);
		} else if(!parse_char(&p, '=')) {
			if(!parse_ident(&p, bf, 255)) {
				struct high_color *color;

				/* Find color */
				for(color=syntax->color;color;color=color->next)
					if(!strcmp(color->name,bf))
						break;
				/* If it doesn't exist, create it */
				if(!color) {
					color = calloc(1, sizeof(struct high_color));
					color->name = (unsigned char *)strdup((char *)bf);
					color->next = syntax->color;
					syntax->color = color;
				} else {
					fprintf(stderr,"%s:%d: Class '%s' already defined\n", name, line, bf);
				}

				/* Parse color definition */
				while(parse_ws(&p,'#'), !parse_ident(&p,bf,255)) {
					color->color |= meta_color(bf);
				}
			}
		} else if(!parse_char(&p, '-')) { /* No. sync lines */
			syntax->sync_lines = (int)ustolb(p, &np,
			    INT_MIN, INT_MAX, USTOL_TRIM);
			if (!np)
				syntax->sync_lines = -1;
			else
				p = np;
		} else {
			c = parse_ws(&p,'#');

			if (!c) {
			} else if (c=='"' || c=='*') {
				if (state) {
					if (!parse_field(&p, UC "*")) {
						int z;
						for(z=0;z!=256;++z)
							clist[z] = 1;
					} else {
						c = parse_string(&p, bf, 255);
						if(c)
							fprintf(stderr,"%s:%d: Bad string\n", name, line);
						else {
							int z;
							int first, second;
							unsigned char *t = bf;
							for(z=0;z!=256;++z)
								clist[z] = 0;
							while(!parse_range(&t, &first, &second)) {
								if(first>second)
									second = first;
								while(first<=second)
									clist[first++] = 1;
							}
						}
					}
					/* Create command */
					parse_ws(&p,'#');
					if(!parse_ident(&p,bf,255)) {
						struct high_cmd *cmd = calloc(1, sizeof(struct high_cmd));
						int z;

						cmd->new_state = find_state(syntax,bf);

						/* Parse options */
						while (parse_ws(&p,'#'), !parse_ident(&p,bf,255))
							if(!strcmp(bf,"buffer")) {
								cmd->start_buffering = 1;
							} else if(!strcmp(bf,"hold")) {
								cmd->stop_buffering = 1;
							} else if(!strcmp(bf,"recolor")) {
								parse_ws(&p,'#');
								if(!parse_char(&p,'=')) {
									parse_ws(&p,'#');
									cmd->recolor = (int)ustolb(p, &np,
									    INT_MIN, INT_MAX, USTOL_TRIM);
									if (!np)
										fprintf(stderr,"%s:%d: Missing value for option %s\n", name, line, bf);
									else
										p = np;
								} else
									fprintf(stderr,"%s:%d: Missing value for option %s\n", name, line, bf);
							} else if(!strcmp(bf,"strings") || !strcmp(bf,"istrings")) {
								if (bf[0]=='i')
									cmd->ignore = 1;
								while(fgets((char *)buf,1023,f)) {
									++line;
									p = buf;
									parse_ws(&p,'#');
									if (*p) {
										if (!parse_field(&p, UC "done"))
											break;
										if(!parse_string(&p,bf,255)) {
											parse_ws(&p,'#');
											if (cmd->ignore)
												joe_strtolower(bf);
											if(!parse_ident(&p,bf1,255)) {
												struct high_cmd *kw_cmd = calloc(1, sizeof(struct high_cmd));
												kw_cmd->noeat=1;
												kw_cmd->new_state = find_state(syntax,bf1);
												if(!cmd->keywords)
													cmd->keywords = htmk(64);
												htadd(cmd->keywords,(unsigned char *)strdup((char *)bf),kw_cmd);
												while (parse_ws(&p,'#'), !parse_ident(&p,bf,255))
													if(!strcmp(bf,"buffer")) {
														kw_cmd->start_buffering = 1;
													} else if(!strcmp(bf,"hold")) {
														kw_cmd->stop_buffering = 1;
													} else if(!strcmp(bf,"recolor")) {
														parse_ws(&p,'#');
														if(!parse_char(&p,'=')) {
															parse_ws(&p,'#');
															kw_cmd->recolor = (int)ustolb(p, &np,
															    INT_MIN, INT_MAX, USTOL_TRIM);
															if (!np)
																fprintf(stderr,"%s:%d: Missing value for option %s\n", name, line, bf);
															else
																p = np;
														} else
															fprintf(stderr,"%s:%d: Missing value for option %s\n", name, line, bf);
													} else
														fprintf(stderr,"%s:%d: Unknown option '%s'\n", name, line, bf);
											} else
												fprintf(stderr,"%s:%d: Missing state name\n", name, line);
										} else
											fprintf(stderr,"%s:%d: Missing string\n", name, line);
									}
								}
							} else if(!strcmp(bf,"noeat")) {
								cmd->noeat = 1;
							} else if(!strcmp(bf,"mark")) {
								/* not implemented yet */ ;
							} else if(!strcmp(bf,"markend")) {
								/* not implemented yet */ ;
							} else if(!strcmp(bf,"recolormark")) {
								/* not implemented yet */ ;
							} else
								fprintf(stderr,"%s:%d: Unknown option '%s'\n", name, line, bf);

						/* Install command */
						for(z=0;z!=256;++z)
							if(clist[z])
								state->cmd[z]=cmd;
					} else
						fprintf(stderr,"%s:%d: Missing jump\n", name, line);
				} else
					fprintf(stderr,"%s:%d: No state\n", name, line);
			} else
				fprintf(stderr,"%s:%d: Unknown character\n", name, line);
		}
	}

	fclose(f);

	return syntax;
}
