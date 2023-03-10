#ifndef lint	/* .../appleprint/dwb/text/pic.d/input.c */
#define _AC_NAME input_c
#define _AC_NO_MAIN "@(#) Copyright (c) 1984-85 AT&T-IS, All Rights Reserved.  {Apple version 1.2 87/11/11 21:56:41}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.2 of input.c on 87/11/11 21:56:41";
#endif		/* _AC_HISTORY */
#endif		/* lint */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _AC_MODS
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include "pic.h"
#include "y.tab.h"

Infile	infile[10];
Infile	*curfile = infile;

#define	MAXSRC	50
Src	src[MAXSRC];	/* input source stack */
Src	*srcp	= src;

pushsrc(type, ptr)	/* new input source */
	int type;
	char *ptr;
{
	if (++srcp >= src + MAXSRC)
		fatal("inputs nested too deep");
	srcp->type = type;
	srcp->sp = ptr;
	if (dbg > 1) {
		printf("\n%3d ", srcp - src);
		switch (srcp->type) {
		case File:
			printf("push file %s\n", ((Infile *)ptr)->fname);
			break;
		case Macro:
			printf("push macro <%s>\n", ptr);
			break;
		case Char:
			printf("push char <%c>\n", *ptr);
			break;
		case Thru:
			printf("push thru\n");
			break;
		case String:
			printf("push string <%s>\n", ptr);
			break;
		case Free:
			printf("push free <%s>\n", ptr);
			break;
		default:
			fatal("pushed bad type %d\n", srcp->type);
		}
	}
}

popsrc()	/* restore an old one */
{
	if (srcp <= src)
		fatal("too many inputs popped");
	if (dbg > 1) {
		printf("%3d ", srcp - src);
		switch (srcp->type) {
		case File:
			printf("pop file\n");
			break;
		case Macro:
			printf("pop macro\n");
			break;
		case Char:
			printf("pop char <%c>\n", *srcp->sp);
			break;
		case Thru:
			printf("pop thru\n");
			break;
		case String:
			printf("pop string\n");
			break;
		case Free:
			printf("pop free\n");
			break;
		default:
			fatal("pop weird input %d\n", srcp->type);
		}
	}
	srcp--;
}

definition(s)	/* collect definition for s and install */
	char *s;	/* definitions picked up lexically */
{
	char *p;
	struct symtab *stp;

	p = delimstr("definition");
	stp = lookup(s);
	if (stp != NULL) {	/* it's there before */
		if (stp->s_type != DEFNAME) {
			yyerror("%s used as variable and definition\n", s);
			return;
		}
		free(stp->s_val.p);
		stp->s_val.p = p;
	} else {
		YYSTYPE u;
		u.p = p;
		makevar(tostring(s), DEFNAME, u);
	}
	dprintf("installing %s as `%s'\n", s, p);
}

char *delimstr(s)	/* get body of X ... X */
	char *s;		/* message if too big */
{
	int c, delim, n;
	static char *buf = NULL;
	static int nbuf = 0;
	char *p;

	if (buf == NULL)
		buf = grow(buf, "buf", nbuf += 1000, sizeof(buf[0]));
	while ((delim = input()) == ' ' || delim == '\t' || delim == '\n')
		;
	for (p = buf; (c = input()) != delim; ) {
		if (p >= buf + nbuf) {
			n = p - buf;
			buf = grow(buf, "buf", nbuf += 1000, sizeof(buf[0]));
			p = buf + n;
		}
		if (c == EOF)
			fatal("end of file in %s %c %.20s... %c", s, delim, buf, delim);
		*p++ = c;
	}
	*p = '\0';
	dprintf("delimstr %s %c <%s> %c\n", s, delim, buf, delim);
	return tostring(buf);
}

Arg	args[10];	/* argument frames */
Arg	*argfp = args;	/* frame pointer */
int	argcnt;		/* number of arguments seen so far */

dodef(stp)	/* collect args and switch input to defn */
	struct symtab *stp;
{
	int i, len;
	char *p;
	Arg *ap;

	ap = argfp+1;
	if (ap >= args+10)
		fatal("arguments too deep");
	argcnt = 0;
	if (input() != '(')
		fatal("disaster in dodef\n");
	if (ap->argval == 0)
		ap->argval = malloc(1000);
	for (p = ap->argval; (len = getarg(p)) != -1; p += len) {
		ap->argstk[argcnt++] = p;
		if (input() == ')')
			break;
	}
	for (i = argcnt; i < MAXARGS; i++)
		ap->argstk[i] = "";
	if (dbg)
		for (i = 0; i < argcnt; i++)
			printf("arg %d.%d = <%s>\n", ap-args, i+1, ap->argstk[i]);
	argfp = ap;
	pushsrc(Macro, stp->s_val.p);
}

getarg(p)	/* pick up single argument, store in p, return length */
	char *p;
{
	int n, c, npar;

	n = npar = 0;
	for ( ;; ) {
		c = input();
		if (c == EOF)
			fatal("end of file in getarg!\n");
		if (npar == 0 && (c == ',' || c == ')'))
			break;
		if (c == '"')	/* copy quoted stuff intact */
			do {
				*p++ = c;
				n++;
			} while ((c = input()) != '"' && c != EOF);
		else if (c == '(')
			npar++;
		else if (c == ')')
			npar--;
		n++;
		*p++ = c;
	}
	*p = 0;
	unput(c);
	return(n + 1);
}

#define	PBSIZE	2000
char	pbuf[PBSIZE];		/* pushback buffer */
char	*pb	= pbuf-1;	/* next pushed back character */

char	ebuf[200];		/* collect input here for error reporting */
char	*ep	= ebuf;

int	begin	= 0;
extern	int	thru;
extern	struct symtab	*thrudef;
extern	char	*untilstr;

input()
{
	register int c;

	if (thru && begin) {
		do_thru();
		begin = 0;
	}
	c = nextchar();
	if (dbg > 1)
		printf(" <%c>", c);
	if (ep >= ebuf + sizeof ebuf)
		ep = ebuf;
	*ep++ = c;
	return c;
}

nextchar()
{
	register int c;

  loop:
	switch (srcp->type) {
	case Free:	/* free string */
		free(srcp->sp);
		popsrc();
		goto loop;
	case Thru:	/* end of pushed back line */
		begin = 1;
		popsrc();
		c = '\n';
		break;
	case Char:
		if (pb >= pbuf) {
			c = *pb--;
			popsrc();
			break;
		} else {	/* can't happen? */
			popsrc();
			goto loop;
		}
	case String:
		c = *srcp->sp++;
		if (c == '\0') {
			popsrc();
			goto loop;
		} else {
			if (*srcp->sp == '\0')	/* empty, so pop */
				popsrc();
			break;
		}
	case Macro:
		c = *srcp->sp++;
		if (c == '\0') {
			if (--argfp < args)
				fatal("argfp underflow");
			popsrc();
			goto loop;
		} else if (c == '$' && isdigit(*srcp->sp)) {
			int n = 0;
			while (isdigit(*srcp->sp))
				n = 10 * n + *srcp->sp++ - '0';
			if (n > 0 && n <= MAXARGS)
				pushsrc(String, argfp->argstk[n-1]);
			goto loop;
		}
		break;
	case File:
		c = getc(curfile->fin);
		if (c == EOF) {
			if (curfile == infile)
				fatal("end of file inside .PS/.PE");
			if (curfile->fin != stdin) {
				fclose(curfile->fin);
				free(curfile->fname);	/* assumes allocated */
			}
			curfile--;
			popsrc();
			thru = 0;	/* chicken out */
			thrudef = 0;
			if (untilstr) {
				free(untilstr);
				untilstr = 0;
			}
			goto loop;
		}
		if (c == '\n')
			curfile->lineno++;
		break;
	}
	return c;
}

do_thru()	/* read one line, make into a macro expansion */
{
	int c, i, n;
	char *p;
	Arg *ap;

	ap = argfp+1;
	if (ap >= args+10)
		fatal("arguments too deep");
	if (ap->argval == NULL)
		ap->argval = malloc(1000);
	p = ap->argval;
	argcnt = 0;
	c = nextchar();
	if (thru == 0) {	/* end of file was seen, so thru is done */
		unput(c);
		return;
	}
	for ( ; c != '\n' && c != EOF; ) {
		if (c == ' ' || c == '\t') {
			c = nextchar();
			continue;
		}
		ap->argstk[argcnt++] = p;
		if (c == '"') {
			do {
				*p++ = c;
				if ((c = nextchar()) == '\\') {
					*p++ = c;
					*p++ = nextchar();
					c = nextchar();
				}
			} while (c != '"' && c != '\n' && c != EOF);
			*p++ = '"';
			if (c == '"')
				c = nextchar();
		} else {
			while (c!=' ' && c!='\t' && c!='\n' && c!=',' && c!=EOF) {
				*p++ = c;
				c = nextchar ();
			}
		}
		if (c == ',')
			c = nextchar();
		*p++ = '\0';
	}
	if (c == EOF)
		fatal("unexpected end of file in do_thru");
	if (argcnt == 0) {	/* ignore blank line */
		pushsrc(Thru, (char *) 0);
		return;
	}
	for (i = argcnt; i < MAXARGS; i++)
		ap->argstk[i] = "";
	if (dbg)
		for (i = 0; i < argcnt; i++)
			printf("arg %d.%d = <%s>\n", ap-args, i+1, ap->argstk[i]);
	if (strcmp(ap->argstk[0], ".PE") == 0) {
		thru = 0;
		thrudef = 0;
		pushsrc(String, "\n.PE\n");
		return;
	}
	if (untilstr && strcmp(ap->argstk[0], untilstr) == 0) {
		thru = 0;
		thrudef = 0;
		free(untilstr);
		untilstr = 0;
		return;
	}
	pushsrc(Thru, (char *) 0);
	dprintf("do_thru pushing back <%s>\n", thrudef->s_val.p);
	argfp = ap;
	pushsrc(Macro, thrudef->s_val.p);
}

unput(c)
{
	if (++pb >= pbuf + sizeof pbuf)
		fatal("pushback overflow\n");
	if (--ep < ebuf)
		ep = ebuf + sizeof(ebuf) - 1;
	*pb = c;
	pushsrc(Char, pb);
	return c;
}

pbstr(s)
	char *s;
{
	int n;

	pushsrc(String, s);
}

double errcheck(x, s)
	double x;
	char *s;
{
	extern int errno;

	if (errno == EDOM) {
		errno = 0;
		yyerror("%s argument out of domain", s);
	} else if (errno == ERANGE) {
		errno = 0;
		yyerror("%s result out of range", s);
	}
	return x;
}

fatal(s, s1, s2, s3, s4)	/* should be a flag on yyerror */
	char *s, *s1, *s2, *s3, *s4;
{
	yyerror(s, s1, s2, s3, s4);
	if (dbg)
		abort();
	else
		exit(1);
}

yyerror(s, s1, s2, s3, s4)
	char *s, *s1, *s2, *s3, *s4;
{
	extern char *cmdname, *sys_errlist[];
	extern int errno, sys_nerr;

	if (synerr)
		return;
	fprintf(stderr, "%s: ", cmdname);
	fprintf(stderr, s, s1, s2, s3, s4);
	if (errno > 0 && errno < sys_nerr)
		fprintf(stderr, " (%s)", sys_errlist[errno]);
	fprintf(stderr, " near line %d, file %s\n",
		curfile->lineno, curfile->fname);
	eprint();
	synerr = 1;
	errno = 0;
}

eprint()	/* try to print context around error */
{
	char *p, *q;
	int c;

	p = ep - 1;
	if (p > ebuf && *p == '\n')
		p--;
	for ( ; p >= ebuf && *p != '\n'; p--)
		;
	while (*p == '\n')
		p++;
	fprintf(stderr, " context is\n\t");
	for (q=ep-1; q>=p && *q!=' ' && *q!='\t' && *q!='\n'; q--)
		;
	while (p < q)
		putc(*p++, stderr);
	fprintf(stderr, " >>> ");
	while (p < ep)
		putc(*p++, stderr);
	fprintf(stderr, " <<< ");
	while (pb >= pbuf)
		putc(*pb--, stderr);
	fgets(ebuf, sizeof ebuf, curfile->fin);
	fprintf(stderr, "%s", ebuf);
	pbstr("\n.PE\n");	/* safety first */
	ep = ebuf;
}

yywrap() {;}

char	*newfile = 0;		/* filename for file copy */
char	*untilstr = 0;		/* string that terminates a thru */
int	thru	= 0;		/* 1 if copying thru macro */
struct symtab	*thrudef = 0;		/* macro being used */

copyfile(s)	/* remember file to start reading from */
	char *s;
{
	newfile = s;
}

copydef(p)	/* remember macro symtab ptr */
	struct symtab *p;
{
	thrudef = p;
}

struct symtab *copythru(s)	/* collect the macro name or body for thru */
	char *s;
{
	struct symtab *p;
	char *q;

	p = lookup(s);
	if (p != NULL) {
		if (p->s_type == DEFNAME)
			return p;
		else
			fatal("%s used as define and name", s);
	}
	/* have to collect the definition */
	pbstr(s);	/* first char is the delimiter */
	q = delimstr("thru body");
	p = lookup("nameless");
	if (p != NULL) {
		if (p->s_val.p)
			free(p->s_val.p);
		p->s_val.p = q;
	} else {
		YYSTYPE u;
		u.p = q;
		p = makevar(tostring(s), DEFNAME, u);
	}
	dprintf("installing nameless as `%s'\n", p->s_val.p);
	return p;
}

copyuntil(s)	/* string that terminates a thru */
	char *s;
{
	untilstr = s;
}

copy()	/* begin input from file, etc. */
{
	FILE *fin;

	if (newfile) {
		if ((fin = fopen(newfile, "r")) == NULL)
			fatal("can't open file %s", newfile);
		curfile++;
		curfile->fin = fin;
		curfile->fname = newfile;
		curfile->lineno = 0;
		pushsrc(File, curfile);
		newfile = 0;
	}
	if (thrudef) {
		thru = 1;
		begin = 1;	/* wrong place */
	}
}

char	shellbuf[1000], *shellp;

shell_init()	/* set up to interpret a shell command */
{
	shellp = shellbuf;
	*shellp = '\0';
}

shell_text(s)	/* add string to command being collected */
	char *s;
{
	while (*shellp++ = *s++)
		;
	shellp--;
}

shell_exec()	/* do it */
{
	system(shellbuf);
}
