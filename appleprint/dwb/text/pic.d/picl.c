#ifndef lint	/* .../appleprint/dwb/text/pic.d/picl.c */
#define _AC_NAME picl_c
#define _AC_NO_MAIN "%Z% Copyright (c) ???, All Rights Reserved.  {Apple version %I% %E% %U%}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.1 of picl.l on 87/05/04 13:48:31";/*	Copyright (c) 1984 AT&T	*/
#endif		/* _AC_HISTORY */
#endif		/* lint */

# include "stdio.h"
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX 200
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin = {stdin}, *yyout = {stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
#ifndef lint	/* .../appleprint/dwb/text/pic.d/picl.l */
#define _AC_NAME picl_l
#define _AC_NO_MAIN "%Z% Copyright (c) 1983-87 AT&T-IS, all rights reserved.  Apple version %I% %E% %U%"
#include <apple_notice.h>

#ifdef _AC_HISTORY
#define _AC_MODS
#endif		/* _AC_HISTORY */
#endif		/* lint */

#define _AC_MODS
/*	  All Rights Reserved  	*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/
# define A 2
# define str 4
# define comment 6
# define def 8
# define sc 10
# define br 12
# define thru 14
# define sh 16
#undef	input
#undef	unput
#include <stdio.h>
#include <ctype.h>
#include "pic.h"
#include "y.tab.h"

extern	float	atof();
extern	struct	symtab	symtab[];
extern	char	*filename;
extern	int	synerr;

#define	CADD	cbuf[clen++]=yytext[0]; if(clen>=CBUFLEN-1) {yyerror("string too long", cbuf); BEGIN A;}
#define	CBUFLEN	500
char	cbuf[CBUFLEN];
int	c, clen, cflag, delim;
int	ifsw	= 0;	/* 1 if if statement in progress */
# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
switch (yybgin-yysvec-1) {	/* witchcraft */
	case 0:
		BEGIN A;
		break;
	case sc:
		BEGIN A;
		return('}');
	case br:
		BEGIN A;
		return(']');
	}
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
	;
break;
case 2:
;
break;
case 3:
	{ return(ST); }
break;
case 4:
	{ return(ST); }
break;
case 5:
	{ BEGIN sc; return(ST); }
break;
case 6:
	{ BEGIN br; return(ST); }
break;
case 7:
{ if (curfile == infile) yyerror(".PS found inside .PS/.PE"); }
break;
case 8:
{ if (curfile == infile) {
			yylval.i = yytext[2];
			return(EOF);
		  }
		}
break;
case 9:
{ yylval.p = tostring(yytext); return(TROFF); }
break;
case 10:
return(yylval.i = PRINT);
break;
case 11:
	return(yylval.i = BOX);
break;
case 12:
return(yylval.i = CIRCLE);
break;
case 13:
	return(yylval.i = ARC);
break;
case 14:
return(yylval.i = ELLIPSE);
break;
case 15:
return(yylval.i = ARROW);
break;
case 16:
return(yylval.i = SPLINE);
break;
case 17:
	return(yylval.i = LINE);
break;
case 18:
	return(yylval.i = MOVE);
break;
case 19:
	return(yylval.i = PLOT);
break;
case 20:
	return(yylval.i = BLOCK);
break;
case 21:
	return(SAME);
break;
case 22:
return(BETWEEN);
break;
case 23:
	return(AND);
break;
case 24:
	;
break;
case 25:
	;
break;
case 26:
	;
break;
case 27:
	{ yylval.i = EAST; return(CORNER); }
break;
case 28:
{ yylval.i = EAST; return(CORNER); }
break;
case 29:
	{ yylval.i = EAST; return(CORNER); }
break;
case 30:
{ yylval.i = EAST; return(CORNER); }
break;
case 31:
	{ yylval.i = WEST; return(CORNER); }
break;
case 32:
{ yylval.i = WEST; return(CORNER); }
break;
case 33:
	{ yylval.i = WEST; return(CORNER); }
break;
case 34:
{ yylval.i = WEST; return(CORNER); }
break;
case 35:
	{ yylval.i = NORTH; return(CORNER); }
break;
case 36:
{ yylval.i = NORTH; return(CORNER); }
break;
case 37:
	{ yylval.i = NORTH; return(CORNER); }
break;
case 38:
{ yylval.i = NORTH; return(CORNER); }
break;
case 39:
	{ yylval.i = SOUTH; return(CORNER); }
break;
case 40:
{ yylval.i = SOUTH; return(CORNER); }
break;
case 41:
	{ yylval.i = SOUTH; return(CORNER); }
break;
case 42:
{ yylval.i = SOUTH; return(CORNER); }
break;
case 43:
{ yylval.i = SOUTH; return(CORNER); }
break;
case 44:
	{ yylval.i = CENTER; return(CORNER); }
break;
case 45:
{ yylval.i = CENTER; return(CORNER); }
break;
case 46:
{ yylval.i = START; return(CORNER); }
break;
case 47:
{ yylval.i = END; return(CORNER); }
break;
case 48:
	{ yylval.i = NE; return(CORNER); }
break;
case 49:
{ yylval.i = NE; return(CORNER); }
break;
case 50:
	{ yylval.i = SE; return(CORNER); }
break;
case 51:
{ yylval.i = SE; return(CORNER); }
break;
case 52:
	{ yylval.i = NW; return(CORNER); }
break;
case 53:
{ yylval.i = NW; return(CORNER); }
break;
case 54:
	{ yylval.i = SW; return(CORNER); }
break;
case 55:
{ yylval.i = SW; return(CORNER); }
break;
case 56:
	{ yylval.i = NORTH; return(CORNER); }
break;
case 57:
	{ yylval.i = NORTH; return(CORNER); }
break;
case 58:
	{ yylval.i = SOUTH; return(CORNER); }
break;
case 59:
	{ yylval.i = SOUTH; return(CORNER); }
break;
case 60:
	{ yylval.i = WEST; return(CORNER); }
break;
case 61:
	{ yylval.i = WEST; return(CORNER); }
break;
case 62:
	{ yylval.i = EAST; return(CORNER); }
break;
case 63:
	{ yylval.i = EAST; return(CORNER); }
break;
case 64:
	{ yylval.i = CENTER; return(CORNER); }
break;
case 65:
	{ yylval.i = START; return(CORNER); }
break;
case 66:
	{ yylval.i = END; return(CORNER); }
break;
case 67:
{ yylval.i = NE; return(CORNER); }
break;
case 68:
{ yylval.i = NW; return(CORNER); }
break;
case 69:
{ yylval.i = SE; return(CORNER); }
break;
case 70:
{ yylval.i = SW; return(CORNER); }
break;
case 71:
{ yylval.i = HEIGHT; return(ATTR); }
break;
case 72:
	{ yylval.i = HEIGHT; return(ATTR); }
break;
case 73:
	{ yylval.i = WIDTH; return(ATTR); }
break;
case 74:
{ yylval.i = WIDTH; return(ATTR); }
break;
case 75:
	{ yylval.i = RADIUS; return(ATTR); }
break;
case 76:
{ yylval.i = RADIUS; return(ATTR); }
break;
case 77:
	{ yylval.i = DIAMETER; return(ATTR); }
break;
case 78:
{ yylval.i = DIAMETER; return(ATTR); }
break;
case 79:
	{ yylval.i = SIZE; return(ATTR); }
break;
case 80:
	{ yylval.i = LEFT; return(DIR); }
break;
case 81:
{ yylval.i = RIGHT; return(DIR); }
break;
case 82:
	{ yylval.i = UP; return(DIR); }
break;
case 83:
	{ yylval.i = DOWN; return(DIR); }
break;
case 84:
	{ yylval.i = CW; return(ATTR); }
break;
case 85:
{ yylval.i = CW; return(ATTR); }
break;
case 86:
	{ yylval.i = CCW; return(ATTR); }
break;
case 87:
{ yylval.i = INVIS; return(ATTR); }
break;
case 88:
{ yylval.i = INVIS; return(ATTR); }
break;
case 89:
	return(yylval.i = DOT);
break;
case 90:
return(yylval.i = DOT);
break;
case 91:
	return(yylval.i = DASH);
break;
case 92:
return(yylval.i = DASH);
break;
case 93:
	return(yylval.i = CHOP);
break;
case 94:
{ yylval.i = SPREAD; return TEXTATTR; }
break;
case 95:
	{ yylval.i = FILL; return TEXTATTR; }
break;
case 96:
{ yylval.i = LJUST; return TEXTATTR; }
break;
case 97:
{ yylval.i = RJUST; return TEXTATTR; }
break;
case 98:
{ yylval.i = ABOVE; return TEXTATTR; }
break;
case 99:
{ yylval.i = BELOW; return TEXTATTR; }
break;
case 100:
	{ yylval.i = HEAD1; return(HEAD); }
break;
case 101:
	{ yylval.i = HEAD2; return(HEAD); }
break;
case 102:
{ yylval.i = HEAD12; return(HEAD); }
break;
case 103:
	return(yylval.i = DOTX);
break;
case 104:
	return(yylval.i = DOTY);
break;
case 105:
return(yylval.i = DOTHT);
break;
case 106:
return(yylval.i = DOTHT);
break;
case 107:
return(yylval.i = DOTWID);
break;
case 108:
return(yylval.i = DOTWID);
break;
case 109:
return(yylval.i = DOTRAD);
break;
case 110:
return(yylval.i = DOTRAD);
break;
case 111:
	return(yylval.i = FROM);
break;
case 112:
	return(yylval.i = TO);
break;
case 113:
	return(yylval.i = AT);
break;
case 114:
	return(yylval.i = BY);
break;
case 115:
	return(yylval.i = WITH);
break;
case 116:
	return(yylval.i = LAST);
break;
case 117:
	return(LOG);
break;
case 118:
	return(EXP);
break;
case 119:
	return(SIN);
break;
case 120:
	return(COS);
break;
case 121:
return(ATAN2);
break;
case 122:
	return(SQRT);
break;
case 123:
	return(RAND);
break;
case 124:
	return(MAX);
break;
case 125:
	return(MIN);
break;
case 126:
	return(INT);
break;
case 127:
	return(EQ);
break;
case 128:
	return(GE);
break;
case 129:
	return(LE);
break;
case 130:
	return(NEQ);
break;
case 131:
	return(GT);
break;
case 132:
	return(LT);
break;
case 133:
	return(ANDAND);
break;
case 134:
	return(OROR);
break;
case 135:
	return(NOT);
break;
case 136:
	return(yylval.i = HERE);
break;
case 137:
	return(FOR);
break;
case 138:
{ endfor(); }
break;
case 139:
	{ yylval.p = delimstr("loop body"); BEGIN A; return(DOSTR); }
break;
case 140:
	return(COPY);
break;
case 141:
{ BEGIN thru; return(THRU); }
break;
case 142:
;
break;
case 143:
{ yylval.p = (char *) copythru(yytext); BEGIN A; return(DEFNAME); }
break;
case 144:
return(UNTIL);
break;
case 145:
	{ ifsw = 1; return(IF); }
break;
case 146:
	{ if (!ifsw) { yylval.i = THEN; return(ATTR); }
		  yylval.p = delimstr("then part"); ifsw = 0; BEGIN A;
		  return(THENSTR); }
break;
case 147:
	{ yylval.p = delimstr("else part"); BEGIN A; return(ELSESTR); }
break;
case 148:
{ BEGIN sh; delim = input(); shell_init(); }
break;
case 149:
{ struct symtab *p;
		  if (yytext[0] == delim) {
			shell_exec();
			BEGIN A;
		  } else {
			p = lookup(yytext, 0);
			if (p != NULL && p->s_type == DEFNAME) {
				c = input();
				unput(c);
				if (c == '(')
					dodef(p);
				else
					pbstr(p->s_val.p);
			} else
				shell_text(yytext);
		  }
		}
break;
case 150:
	{ if (yytext[0] == delim) {
			shell_exec();
			BEGIN A;
		  } else
			shell_text(yytext);
		}
break;
case 151:
{ BEGIN def; }
break;
case 152:
{ definition(yytext); BEGIN A; }
break;
case 153:
	{ yylval.i = 1; return(NTH); }
break;
case 154:
{ yylval.i = atoi(yytext); return(NTH); }
break;
case 155:
{
		  yylval.f = atof(yytext); return(NUMBER); }
break;
case 156:
{
		struct symtab *p;
		p = lookup(yytext);
		if (p != NULL && p->s_type == DEFNAME) {
			c = input();
			unput(c);
			if (c == '(')	/* it's name(...) */
				dodef(p);
			else {	/* no argument list */
				pbstr(p->s_val);
				dprintf("pushing back `%s'\n", p->s_val);
			}
		} else if (islower(yytext[0])) {
			yylval.p = tostring(yytext);
			return(VARNAME);
		} else {
			yylval.p = tostring(yytext);
			return(PLACENAME);
		}
	}
break;
case 157:
	{ BEGIN str; clen=0; }
break;
case 158:
	{ BEGIN comment; }
break;
case 159:
{ BEGIN A; return(ST); }
break;
case 160:
;
break;
case 161:
	{ yylval.i = yytext[0]; return(yytext[0]); }
break;
case 162:
	{ BEGIN A; cbuf[clen]=0; yylval.p = tostring(cbuf); return(TEXT); }
break;
case 163:
	{ yyerror("newline in string"); BEGIN A; return(ST); }
break;
case 164:
{ cbuf[clen++]='"'; }
break;
case 165:
{ cbuf[clen++]='\t'; }
break;
case 166:
{ cbuf[clen++]='\\'; }
break;
case 167:
	{ CADD; }
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */
int yyvstop[] = {
0,

161,
0,

1,
161,
0,

3,
0,

135,
161,
0,

157,
161,
0,

158,
161,
0,

161,
0,

161,
0,

161,
0,

155,
161,
0,

4,
161,
0,

132,
161,
0,

161,
0,

131,
161,
0,

156,
161,
0,

156,
161,
0,

161,
0,

161,
0,

6,
161,
0,

156,
161,
0,

156,
161,
0,

156,
161,
0,

156,
161,
0,

156,
161,
0,

156,
161,
0,

156,
161,
0,

156,
161,
0,

156,
161,
0,

156,
161,
0,

156,
161,
0,

156,
161,
0,

156,
161,
0,

156,
161,
0,

156,
161,
0,

156,
161,
0,

156,
161,
0,

156,
161,
0,

161,
0,

5,
161,
0,

9,
161,
0,

156,
161,
0,

167,
0,

163,
0,

162,
167,
0,

167,
0,

160,
0,

159,
0,

152,
0,

143,
0,

142,
143,
0,

143,
0,

150,
0,

149,
150,
0,

130,
0,

133,
0,

101,
0,

155,
0,

41,
0,

44,
0,

27,
0,

33,
0,

35,
0,

29,
0,

39,
0,

37,
0,

31,
0,

103,
0,

104,
0,

155,
0,

155,
0,

155,
0,

100,
0,

129,
0,

127,
0,

128,
0,

156,
0,

156,
0,

20,
0,

2,
0,

156,
0,

156,
0,

156,
0,

113,
156,
0,

156,
0,

156,
0,

114,
156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

84,
156,
0,

156,
0,

156,
0,

156,
0,

139,
156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

72,
156,
0,

145,
156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

24,
156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

112,
156,
0,

156,
0,

82,
156,
0,

156,
0,

156,
0,

156,
0,

134,
0,

9,
0,

9,
155,
0,

9,
0,

9,
41,
0,

9,
44,
0,

9,
27,
0,

9,
0,

9,
33,
0,

9,
35,
0,

9,
29,
0,

9,
39,
0,

9,
37,
0,

9,
0,

9,
31,
0,

9,
103,
0,

9,
104,
0,

156,
0,

164,
0,

166,
0,

165,
0,

142,
0,

149,
0,

105,
0,

48,
0,

52,
0,

50,
0,

54,
0,

155,
0,

154,
0,

102,
0,

156,
0,

156,
0,

23,
156,
0,

13,
156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

11,
156,
0,

86,
156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

120,
156,
0,

156,
0,

156,
0,

156,
0,

89,
156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

118,
156,
0,

156,
0,

156,
0,

137,
156,
0,

156,
0,

156,
0,

126,
156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

117,
156,
0,

156,
0,

124,
156,
0,

125,
156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

75,
156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

148,
0,

119,
156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

25,
156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

26,
156,
0,

156,
0,

73,
156,
0,

156,
0,

9,
0,

9,
155,
0,

8,
9,
0,

7,
9,
0,

9,
0,

9,
0,

9,
0,

9,
0,

9,
0,

9,
105,
0,

9,
0,

9,
0,

9,
48,
0,

9,
0,

9,
52,
0,

9,
0,

9,
0,

9,
50,
0,

9,
0,

9,
0,

9,
54,
0,

9,
0,

9,
0,

9,
0,

9,
0,

156,
0,

42,
0,

47,
0,

109,
0,

38,
0,

107,
0,

136,
156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

93,
156,
0,

156,
0,

156,
0,

140,
156,
0,

91,
156,
0,

156,
0,

77,
156,
0,

156,
0,

83,
156,
0,

156,
0,

156,
0,

147,
156,
0,

95,
156,
0,

156,
0,

111,
156,
0,

156,
0,

156,
0,

116,
156,
0,

80,
156,
0,

17,
156,
0,

156,
0,

156,
0,

18,
156,
0,

156,
0,

19,
156,
0,

156,
0,

156,
0,

123,
156,
0,

156,
0,

156,
0,

21,
156,
0,

79,
156,
0,

156,
0,

156,
0,

156,
0,

122,
156,
0,

156,
0,

146,
156,
0,

156,
0,

141,
156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

115,
156,
0,

9,
0,

9,
155,
0,

9,
42,
0,

9,
0,

9,
0,

9,
47,
0,

9,
0,

9,
0,

9,
0,

9,
0,

9,
109,
0,

9,
0,

9,
0,

9,
0,

9,
38,
0,

9,
0,

9,
0,

9,
107,
0,

156,
0,

28,
0,

34,
0,

32,
0,

98,
156,
0,

15,
156,
0,

121,
156,
0,

99,
156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

153,
156,
0,

156,
0,

87,
156,
0,

96,
156,
0,

156,
0,

156,
0,

10,
156,
0,

156,
0,

81,
156,
0,

97,
156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

156,
0,

144,
156,
0,

156,
0,

74,
156,
0,

9,
0,

9,
0,

9,
28,
0,

9,
0,

9,
34,
0,

9,
0,

9,
0,

9,
0,

9,
0,

9,
0,

9,
0,

9,
0,

9,
32,
0,

9,
0,

156,
0,

36,
0,

30,
0,

40,
0,

46,
0,

108,
0,

156,
0,

156,
0,

156,
0,

12,
156,
0,

156,
0,

92,
156,
0,

156,
0,

156,
0,

90,
156,
0,

156,
0,

66,
0,

71,
156,
0,

156,
0,

76,
156,
0,

16,
156,
0,

94,
156,
0,

156,
0,

56,
0,

9,
0,

9,
0,

9,
0,

9,
0,

9,
36,
0,

9,
0,

9,
30,
0,

9,
40,
0,

9,
46,
0,

9,
0,

9,
108,
0,

156,
0,

43,
0,

45,
0,

106,
0,

110,
0,

22,
156,
0,

156,
0,

151,
0,

156,
0,

63,
0,

14,
156,
0,

156,
0,

60,
0,

61,
0,

9,
43,
0,

9,
45,
0,

9,
106,
0,

9,
0,

9,
0,

9,
110,
0,

9,
0,

9,
0,

138,
0,

156,
0,

78,
156,
0,

156,
0,

57,
0,

62,
0,

59,
0,

65,
0,

9,
0,

9,
0,

9,
0,

9,
0,

58,
0,

64,
0,

85,
156,
0,

88,
156,
0,

9,
0,

9,
0,

9,
0,

9,
0,

55,
0,

53,
0,

70,
0,

68,
0,

9,
55,
0,

9,
0,

9,
53,
0,

9,
0,

51,
0,

49,
0,

69,
0,

67,
0,

9,
51,
0,

9,
49,
0,
0};
# define YYTYPE int
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,	0,0,	3,19,	0,0,	
5,60,	0,0,	0,0,	7,64,	
0,0,	0,0,	3,20,	3,21,	
5,60,	5,61,	16,0,	7,64,	
7,65,	15,67,	18,0,	36,104,	
17,70,	0,0,	0,0,	0,0,	
0,0,	15,68,	15,0,	0,0,	
17,70,	17,0,	0,0,	0,0,	
0,0,	0,0,	3,22,	3,23,	
3,24,	5,62,	6,62,	3,25,	
25,73,	0,0,	0,0,	0,0,	
0,0,	0,0,	3,26,	3,27,	
0,0,	3,28,	0,0,	5,60,	
0,0,	0,0,	7,64,	68,184,	
0,0,	0,0,	30,97,	0,0,	
3,29,	3,30,	3,31,	3,32,	
15,67,	22,72,	3,33,	17,70,	
5,60,	26,74,	3,33,	7,64,	
5,60,	3,34,	30,98,	7,64,	
31,99,	32,100,	68,184,	0,0,	
0,0,	15,69,	0,0,	0,0,	
17,71,	15,69,	0,0,	0,0,	
17,71,	0,0,	0,0,	0,0,	
3,35,	3,36,	3,37,	5,63,	
6,63,	35,103,	3,38,	3,39,	
3,40,	3,41,	3,42,	3,43,	
34,102,	3,44,	3,45,	49,143,	
0,0,	3,46,	3,47,	3,48,	
3,49,	3,50,	44,130,	3,51,	
3,52,	3,53,	3,54,	48,142,	
3,55,	38,105,	4,22,	4,23,	
4,24,	3,56,	3,57,	4,25,	
45,132,	44,131,	42,123,	56,163,	
39,109,	38,106,	4,26,	4,58,	
45,133,	38,107,	43,127,	38,108,	
50,144,	42,124,	39,110,	42,125,	
43,128,	59,180,	50,145,	43,129,	
4,29,	4,30,	4,31,	4,32,	
39,111,	42,126,	76,186,	53,156,	
41,119,	77,187,	4,59,	4,33,	
41,120,	4,34,	53,157,	40,112,	
41,121,	40,113,	84,203,	85,204,	
40,114,	40,115,	41,122,	47,139,	
40,116,	93,209,	54,158,	40,117,	
54,159,	79,190,	95,209,	47,140,	
4,35,	4,36,	4,37,	40,118,	
96,209,	47,141,	4,38,	4,39,	
4,40,	4,41,	4,42,	4,43,	
79,191,	4,44,	4,45,	97,210,	
78,188,	4,46,	4,47,	4,48,	
4,49,	4,50,	51,146,	4,51,	
4,52,	4,53,	4,54,	102,211,	
4,55,	78,189,	51,147,	51,148,	
105,212,	4,56,	4,57,	9,66,	
9,66,	9,66,	9,66,	9,66,	
9,66,	9,66,	9,66,	9,66,	
9,66,	9,66,	9,66,	9,66,	
9,66,	9,66,	9,66,	9,66,	
9,66,	9,66,	9,66,	9,66,	
9,66,	9,66,	9,66,	9,66,	
9,66,	106,213,	86,205,	63,181,	
108,216,	9,66,	86,206,	9,66,	
9,66,	9,66,	9,66,	9,66,	
9,66,	9,66,	9,66,	9,66,	
9,66,	9,66,	9,66,	9,66,	
9,66,	9,66,	9,66,	9,66,	
9,66,	9,66,	9,66,	9,66,	
9,66,	9,66,	9,66,	9,66,	
9,66,	27,75,	27,75,	27,75,	
27,75,	27,75,	27,75,	27,75,	
27,75,	27,75,	27,75,	110,219,	
112,221,	113,222,	114,223,	110,220,	
28,89,	115,224,	28,90,	28,90,	
28,90,	28,90,	28,90,	28,90,	
28,90,	28,90,	28,90,	28,90,	
46,134,	63,182,	55,160,	80,192,	
46,135,	107,214,	55,161,	82,197,	
46,136,	46,137,	55,162,	28,91,	
81,194,	80,193,	46,138,	82,198,	
116,225,	119,228,	117,226,	109,217,	
107,215,	117,227,	81,195,	27,76,	
27,77,	63,183,	27,78,	109,218,	
120,229,	27,79,	81,196,	121,230,	
123,233,	27,80,	122,231,	27,81,	
125,236,	122,232,	126,237,	27,82,	
27,83,	27,84,	27,85,	28,91,	
27,86,	27,87,	27,88,	28,92,	
124,234,	128,240,	129,241,	130,242,	
28,93,	134,245,	135,246,	124,235,	
28,94,	28,95,	28,96,	33,101,	
33,101,	33,101,	33,101,	33,101,	
33,101,	33,101,	33,101,	33,101,	
33,101,	127,238,	133,243,	136,247,	
133,244,	137,248,	139,251,	127,239,	
33,101,	33,101,	33,101,	33,101,	
33,101,	33,101,	33,101,	33,101,	
33,101,	33,101,	33,101,	33,101,	
33,101,	33,101,	33,101,	33,101,	
33,101,	33,101,	33,101,	33,101,	
33,101,	33,101,	33,101,	33,101,	
33,101,	33,101,	140,252,	141,253,	
142,254,	144,255,	33,101,	145,256,	
33,101,	33,101,	33,101,	33,101,	
33,101,	33,101,	33,101,	33,101,	
33,101,	33,101,	33,101,	33,101,	
33,101,	33,101,	33,101,	33,101,	
33,101,	33,101,	33,101,	33,101,	
33,101,	33,101,	33,101,	33,101,	
33,101,	33,101,	52,149,	58,164,	
138,249,	146,257,	147,259,	148,260,	
149,261,	52,150,	52,151,	58,164,	
58,0,	152,265,	154,268,	146,258,	
52,152,	52,153,	52,154,	155,269,	
138,250,	52,155,	66,66,	66,66,	
66,66,	66,66,	66,66,	66,66,	
66,66,	66,66,	66,66,	66,66,	
75,75,	75,75,	75,75,	75,75,	
75,75,	75,75,	75,75,	75,75,	
75,75,	75,75,	83,199,	150,262,	
151,263,	153,266,	156,270,	157,272,	
158,273,	159,274,	58,165,	153,267,	
83,200,	75,91,	160,275,	161,276,	
151,264,	83,201,	178,0,	156,271,	
83,202,	166,0,	179,0,	180,304,	
186,305,	176,0,	150,262,	58,164,	
162,277,	167,0,	187,306,	58,164,	
89,89,	89,89,	89,89,	89,89,	
89,89,	89,89,	89,89,	89,89,	
89,89,	89,89,	58,166,	188,307,	
162,278,	75,91,	189,308,	190,309,	
192,310,	75,92,	193,311,	195,312,	
197,313,	198,314,	200,315,	201,316,	
203,317,	204,318,	205,319,	206,320,	
58,167,	58,168,	208,92,	58,169,	
211,321,	212,322,	58,170,	215,323,	
216,324,	217,325,	58,171,	218,326,	
58,172,	219,327,	175,0,	222,328,	
58,173,	58,174,	58,175,	58,176,	
166,281,	58,177,	58,178,	58,179,	
69,69,	69,69,	69,69,	69,69,	
69,69,	69,69,	69,69,	69,69,	
69,69,	69,69,	166,282,	223,329,	
224,330,	225,331,	226,332,	228,333,	
229,334,	69,69,	69,69,	69,69,	
69,69,	69,69,	69,69,	69,69,	
69,69,	69,69,	69,69,	69,69,	
69,69,	69,69,	69,69,	69,69,	
69,69,	69,69,	69,69,	69,69,	
69,69,	69,69,	69,69,	69,69,	
69,69,	69,69,	69,69,	176,301,	
230,335,	231,336,	167,283,	69,69,	
232,337,	69,69,	69,69,	69,69,	
69,69,	69,69,	69,69,	69,69,	
69,69,	69,69,	69,69,	69,69,	
69,69,	69,69,	69,69,	69,69,	
69,69,	69,69,	69,69,	69,69,	
69,69,	69,69,	69,69,	69,69,	
69,69,	69,69,	69,69,	71,185,	
71,185,	71,185,	71,185,	71,185,	
71,185,	71,185,	71,185,	71,185,	
71,185,	169,0,	168,0,	170,0,	
171,0,	172,0,	174,0,	175,300,	
71,185,	71,185,	71,185,	71,185,	
71,185,	71,185,	71,185,	71,185,	
71,185,	71,185,	71,185,	71,185,	
71,185,	71,185,	71,185,	71,185,	
71,185,	71,185,	71,185,	71,185,	
71,185,	71,185,	71,185,	71,185,	
71,185,	71,185,	173,0,	233,338,	
177,0,	234,339,	71,185,	235,340,	
71,185,	71,185,	71,185,	71,185,	
71,185,	71,185,	71,185,	71,185,	
71,185,	71,185,	71,185,	71,185,	
71,185,	71,185,	71,185,	71,185,	
71,185,	71,185,	71,185,	71,185,	
71,185,	71,185,	71,185,	71,185,	
71,185,	71,185,	91,207,	236,341,	
91,207,	238,342,	239,343,	91,208,	
91,208,	91,208,	91,208,	91,208,	
91,208,	91,208,	91,208,	91,208,	
91,208,	164,164,	241,344,	165,164,	
242,345,	244,346,	245,347,	246,348,	
169,285,	164,164,	164,0,	165,164,	
165,0,	168,284,	170,287,	171,289,	
172,291,	174,296,	247,349,	248,350,	
250,351,	169,286,	253,352,	254,353,	
255,354,	171,290,	172,292,	174,297,	
256,355,	170,288,	257,356,	258,357,	
174,298,	259,358,	172,293,	174,299,	
260,359,	261,360,	264,361,	262,262,	
265,362,	173,294,	266,363,	267,364,	
268,365,	269,366,	270,367,	177,302,	
164,164,	173,295,	165,165,	177,303,	
207,208,	207,208,	207,208,	207,208,	
207,208,	207,208,	207,208,	207,208,	
207,208,	207,208,	262,262,	272,370,	
271,368,	164,164,	273,371,	165,164,	
274,372,	164,164,	271,369,	165,279,	
165,164,	276,373,	277,374,	278,375,	
279,0,	280,0,	281,281,	283,0,	
282,282,	284,0,	285,0,	286,0,	
287,0,	288,0,	281,281,	281,0,	
282,282,	282,0,	289,0,	290,0,	
291,0,	292,0,	293,0,	295,0,	
294,0,	296,0,	297,0,	299,0,	
300,0,	301,0,	298,0,	165,279,	
302,0,	303,0,	304,394,	165,280,	
305,395,	279,376,	306,396,	279,376,	
307,397,	309,398,	279,377,	310,399,	
311,400,	312,401,	313,402,	314,403,	
315,404,	316,405,	318,406,	319,407,	
320,408,	281,281,	322,409,	282,282,	
323,410,	324,411,	325,412,	326,413,	
327,414,	328,415,	330,416,	331,417,	
333,418,	334,419,	335,420,	336,421,	
338,422,	339,423,	281,281,	341,341,	
282,282,	343,425,	281,281,	345,426,	
282,282,	346,427,	348,428,	350,429,	
351,430,	353,431,	355,432,	356,433,	
358,434,	359,435,	362,436,	363,437,	
364,438,	366,439,	368,440,	370,370,	
371,442,	372,443,	373,444,	374,445,	
376,0,	377,0,	378,0,	379,0,	
380,0,	286,381,	381,0,	382,0,	
383,0,	385,0,	384,0,	287,382,	
386,0,	284,379,	289,383,	388,0,	
389,0,	283,378,	294,386,	285,380,	
295,387,	298,389,	387,0,	390,0,	
392,0,	393,0,	391,0,	303,393,	
394,460,	292,385,	395,461,	396,462,	
290,384,	398,463,	300,390,	301,391,	
400,464,	297,388,	376,377,	377,377,	
401,465,	302,392,	402,466,	403,467,	
404,468,	405,469,	406,470,	408,471,	
413,472,	414,473,	415,474,	416,475,	
417,476,	418,477,	341,424,	419,478,	
420,479,	421,480,	422,422,	423,482,	
424,483,	426,484,	427,485,	428,428,	
430,487,	431,488,	433,489,	434,490,	
436,491,	437,492,	438,493,	439,494,	
440,495,	441,496,	370,441,	443,497,	
444,444,	446,0,	447,0,	448,0,	
449,0,	450,0,	451,0,	452,0,	
453,0,	455,0,	454,0,	456,0,	
458,0,	459,0,	457,0,	460,510,	
461,511,	462,512,	463,513,	466,516,	
377,280,	472,519,	473,520,	464,464,	
382,449,	384,451,	474,521,	476,522,	
478,523,	479,524,	470,470,	386,453,	
378,446,	379,447,	380,448,	481,525,	
482,526,	485,527,	383,450,	385,452,	
387,454,	391,457,	486,528,	487,487,	
389,456,	388,455,	495,369,	488,488,	
490,490,	491,491,	494,494,	478,523,	
498,537,	499,0,	392,458,	393,459,	
497,497,	500,0,	501,0,	502,0,	
503,0,	422,481,	504,0,	505,0,	
506,0,	507,0,	428,486,	508,0,	
509,0,	510,546,	514,547,	515,548,	
517,549,	518,550,	520,520,	521,521,	
522,553,	523,523,	524,554,	444,498,	
527,555,	502,502,	529,556,	530,557,	
531,558,	532,559,	533,560,	534,561,	
535,562,	508,508,	536,563,	538,0,	
539,0,	447,500,	540,0,	541,0,	
542,0,	543,0,	449,501,	464,514,	
523,523,	452,503,	446,499,	455,506,	
544,0,	464,515,	470,517,	459,509,	
545,0,	547,568,	451,502,	548,569,	
470,518,	549,570,	550,571,	453,504,	
454,505,	456,507,	457,508,	487,529,	
551,572,	552,573,	553,574,	555,575,	
556,576,	487,530,	488,531,	490,532,	
491,533,	494,534,	557,577,	562,578,	
497,535,	563,579,	564,0,	565,0,	
568,584,	566,0,	497,536,	567,0,	
569,585,	570,586,	571,587,	576,588,	
577,589,	578,590,	579,591,	580,0,	
581,0,	582,0,	583,0,	585,596,	
499,538,	520,551,	521,552,	587,597,	
589,598,	502,541,	591,599,	592,0,	
593,0,	500,539,	594,0,	502,542,	
501,540,	508,544,	595,0,	504,543,	
600,0,	601,0,	0,0,	508,545,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	541,564,	0,0,	
0,0,	0,0,	0,0,	542,565,	
0,0,	0,0,	0,0,	544,566,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	545,567,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	564,580,	0,0,	
565,581,	566,582,	0,0,	0,0,	
567,583,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	581,593,	0,0,	
583,595,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	580,592,	0,0,	582,594,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	593,600,	0,0,	
0,0,	0,0,	0,0,	0,0,	
595,601,	0,0,	0,0,	0,0,	
0,0};
struct yysvf yysvec[] = {
0,	0,	0,
yycrank+0,	0,		0,	
yycrank+0,	0,		0,	
yycrank+-1,	0,		0,	
yycrank+-89,	yysvec+3,	0,	
yycrank+-3,	0,		0,	
yycrank+-4,	yysvec+5,	0,	
yycrank+-6,	0,		0,	
yycrank+0,	yysvec+7,	0,	
yycrank+150,	0,		0,	
yycrank+0,	yysvec+9,	0,	
yycrank+0,	0,		0,	
yycrank+0,	0,		0,	
yycrank+0,	0,		0,	
yycrank+0,	0,		0,	
yycrank+-16,	0,		0,	
yycrank+-4,	yysvec+15,	0,	
yycrank+-19,	0,		0,	
yycrank+-8,	yysvec+17,	0,	
yycrank+0,	0,		yyvstop+1,
yycrank+0,	0,		yyvstop+3,
yycrank+0,	0,		yyvstop+6,
yycrank+4,	0,		yyvstop+8,
yycrank+0,	0,		yyvstop+11,
yycrank+0,	0,		yyvstop+14,
yycrank+2,	0,		yyvstop+17,
yycrank+7,	0,		yyvstop+19,
yycrank+225,	0,		yyvstop+21,
yycrank+242,	0,		yyvstop+23,
yycrank+0,	0,		yyvstop+26,
yycrank+13,	0,		yyvstop+29,
yycrank+15,	0,		yyvstop+32,
yycrank+16,	0,		yyvstop+34,
yycrank+311,	0,		yyvstop+37,
yycrank+3,	yysvec+33,	yyvstop+40,
yycrank+4,	0,		yyvstop+43,
yycrank+9,	0,		yyvstop+45,
yycrank+0,	0,		yyvstop+47,
yycrank+23,	yysvec+33,	yyvstop+50,
yycrank+31,	yysvec+33,	yyvstop+53,
yycrank+64,	yysvec+33,	yyvstop+56,
yycrank+59,	yysvec+33,	yyvstop+59,
yycrank+33,	yysvec+33,	yyvstop+62,
yycrank+33,	yysvec+33,	yyvstop+65,
yycrank+13,	yysvec+33,	yyvstop+68,
yycrank+26,	yysvec+33,	yyvstop+71,
yycrank+203,	yysvec+33,	yyvstop+74,
yycrank+74,	yysvec+33,	yyvstop+77,
yycrank+8,	yysvec+33,	yyvstop+80,
yycrank+5,	yysvec+33,	yyvstop+83,
yycrank+32,	yysvec+33,	yyvstop+86,
yycrank+105,	yysvec+33,	yyvstop+89,
yycrank+337,	yysvec+33,	yyvstop+92,
yycrank+51,	yysvec+33,	yyvstop+95,
yycrank+64,	yysvec+33,	yyvstop+98,
yycrank+205,	yysvec+33,	yyvstop+101,
yycrank+7,	0,		yyvstop+104,
yycrank+0,	0,		yyvstop+106,
yycrank+-434,	0,		yyvstop+109,
yycrank+35,	yysvec+33,	yyvstop+112,
yycrank+0,	0,		yyvstop+115,
yycrank+0,	0,		yyvstop+117,
yycrank+0,	0,		yyvstop+119,
yycrank+209,	0,		yyvstop+122,
yycrank+0,	0,		yyvstop+124,
yycrank+0,	0,		yyvstop+126,
yycrank+406,	yysvec+9,	yyvstop+128,
yycrank+0,	0,		yyvstop+130,
yycrank+46,	0,		yyvstop+132,
yycrank+508,	0,		yyvstop+135,
yycrank+0,	0,		yyvstop+137,
yycrank+583,	0,		yyvstop+139,
yycrank+0,	0,		yyvstop+142,
yycrank+0,	0,		yyvstop+144,
yycrank+0,	0,		yyvstop+146,
yycrank+416,	0,		yyvstop+148,
yycrank+43,	0,		yyvstop+150,
yycrank+56,	0,		yyvstop+152,
yycrank+99,	0,		yyvstop+154,
yycrank+76,	0,		0,	
yycrank+202,	0,		yyvstop+156,
yycrank+211,	0,		yyvstop+158,
yycrank+210,	0,		yyvstop+160,
yycrank+373,	0,		yyvstop+162,
yycrank+55,	0,		yyvstop+164,
yycrank+55,	0,		0,	
yycrank+141,	0,		yyvstop+166,
yycrank+0,	0,		yyvstop+168,
yycrank+0,	0,		yyvstop+170,
yycrank+456,	yysvec+75,	yyvstop+172,
yycrank+0,	yysvec+28,	yyvstop+174,
yycrank+663,	0,		0,	
yycrank+0,	0,		yyvstop+176,
yycrank+73,	0,		0,	
yycrank+0,	yysvec+93,	0,	
yycrank+62,	0,		0,	
yycrank+80,	0,		0,	
yycrank+133,	0,		yyvstop+178,
yycrank+0,	0,		yyvstop+180,
yycrank+0,	0,		yyvstop+182,
yycrank+0,	0,		yyvstop+184,
yycrank+0,	yysvec+33,	yyvstop+186,
yycrank+93,	yysvec+33,	yyvstop+188,
yycrank+0,	0,		yyvstop+190,
yycrank+0,	0,		yyvstop+192,
yycrank+101,	yysvec+33,	yyvstop+194,
yycrank+141,	yysvec+33,	yyvstop+196,
yycrank+206,	yysvec+33,	yyvstop+198,
yycrank+147,	yysvec+33,	yyvstop+200,
yycrank+211,	yysvec+33,	yyvstop+203,
yycrank+167,	yysvec+33,	yyvstop+205,
yycrank+0,	yysvec+33,	yyvstop+207,
yycrank+165,	yysvec+33,	yyvstop+210,
yycrank+175,	yysvec+33,	yyvstop+212,
yycrank+175,	yysvec+33,	yyvstop+214,
yycrank+175,	yysvec+33,	yyvstop+216,
yycrank+205,	yysvec+33,	yyvstop+218,
yycrank+206,	yysvec+33,	yyvstop+220,
yycrank+0,	yysvec+33,	yyvstop+222,
yycrank+202,	yysvec+33,	yyvstop+225,
yycrank+226,	yysvec+33,	yyvstop+227,
yycrank+234,	yysvec+33,	yyvstop+229,
yycrank+218,	yysvec+33,	yyvstop+231,
yycrank+217,	yysvec+33,	yyvstop+234,
yycrank+240,	yysvec+33,	yyvstop+236,
yycrank+236,	yysvec+33,	yyvstop+238,
yycrank+226,	yysvec+33,	yyvstop+240,
yycrank+261,	yysvec+33,	yyvstop+242,
yycrank+235,	yysvec+33,	yyvstop+244,
yycrank+239,	yysvec+33,	yyvstop+246,
yycrank+246,	yysvec+33,	yyvstop+248,
yycrank+0,	yysvec+33,	yyvstop+250,
yycrank+0,	yysvec+33,	yyvstop+253,
yycrank+254,	yysvec+33,	yyvstop+256,
yycrank+238,	yysvec+33,	yyvstop+258,
yycrank+252,	yysvec+33,	yyvstop+260,
yycrank+261,	yysvec+33,	yyvstop+262,
yycrank+256,	yysvec+33,	yyvstop+264,
yycrank+333,	yysvec+33,	yyvstop+266,
yycrank+254,	yysvec+33,	yyvstop+268,
yycrank+292,	yysvec+33,	yyvstop+270,
yycrank+285,	yysvec+33,	yyvstop+272,
yycrank+290,	yysvec+33,	yyvstop+274,
yycrank+0,	yysvec+33,	yyvstop+276,
yycrank+294,	yysvec+33,	yyvstop+279,
yycrank+302,	yysvec+33,	yyvstop+281,
yycrank+337,	yysvec+33,	yyvstop+283,
yycrank+335,	yysvec+33,	yyvstop+285,
yycrank+322,	yysvec+33,	yyvstop+287,
yycrank+331,	yysvec+33,	yyvstop+289,
yycrank+466,	yysvec+33,	yyvstop+291,
yycrank+366,	yysvec+33,	yyvstop+293,
yycrank+328,	yysvec+33,	yyvstop+295,
yycrank+369,	yysvec+33,	yyvstop+297,
yycrank+332,	yysvec+33,	yyvstop+299,
yycrank+354,	yysvec+33,	yyvstop+301,
yycrank+377,	yysvec+33,	yyvstop+303,
yycrank+367,	yysvec+33,	yyvstop+305,
yycrank+364,	yysvec+33,	yyvstop+308,
yycrank+369,	yysvec+33,	yyvstop+310,
yycrank+365,	yysvec+33,	yyvstop+313,
yycrank+372,	yysvec+33,	yyvstop+315,
yycrank+400,	yysvec+33,	yyvstop+317,
yycrank+0,	0,		yyvstop+319,
yycrank+-720,	0,		yyvstop+321,
yycrank+-722,	0,		yyvstop+323,
yycrank+-483,	yysvec+164,	yyvstop+326,
yycrank+-491,	yysvec+164,	yyvstop+328,
yycrank+-632,	yysvec+164,	yyvstop+331,
yycrank+-631,	yysvec+164,	yyvstop+334,
yycrank+-633,	yysvec+164,	yyvstop+337,
yycrank+-634,	yysvec+164,	yyvstop+339,
yycrank+-635,	yysvec+164,	yyvstop+342,
yycrank+-664,	yysvec+164,	yyvstop+345,
yycrank+-636,	yysvec+164,	yyvstop+348,
yycrank+-536,	yysvec+164,	yyvstop+351,
yycrank+-487,	yysvec+164,	yyvstop+354,
yycrank+-666,	yysvec+164,	yyvstop+356,
yycrank+-480,	yysvec+164,	yyvstop+359,
yycrank+-484,	yysvec+164,	yyvstop+362,
yycrank+395,	yysvec+33,	yyvstop+365,
yycrank+0,	0,		yyvstop+367,
yycrank+0,	0,		yyvstop+369,
yycrank+0,	0,		yyvstop+371,
yycrank+0,	yysvec+68,	yyvstop+373,
yycrank+0,	yysvec+71,	yyvstop+375,
yycrank+380,	0,		0,	
yycrank+392,	0,		0,	
yycrank+400,	0,		0,	
yycrank+418,	0,		0,	
yycrank+414,	0,		0,	
yycrank+0,	0,		yyvstop+377,
yycrank+418,	0,		0,	
yycrank+403,	0,		0,	
yycrank+0,	0,		yyvstop+379,
yycrank+409,	0,		0,	
yycrank+0,	0,		yyvstop+381,
yycrank+424,	0,		0,	
yycrank+422,	0,		0,	
yycrank+0,	0,		yyvstop+383,
yycrank+409,	0,		0,	
yycrank+430,	0,		0,	
yycrank+0,	0,		yyvstop+385,
yycrank+416,	0,		0,	
yycrank+417,	0,		0,	
yycrank+415,	0,		0,	
yycrank+431,	0,		0,	
yycrank+724,	0,		0,	
yycrank+429,	yysvec+207,	yyvstop+387,
yycrank+0,	0,		yyvstop+389,
yycrank+0,	0,		yyvstop+391,
yycrank+435,	yysvec+33,	yyvstop+393,
yycrank+419,	yysvec+33,	yyvstop+395,
yycrank+0,	yysvec+33,	yyvstop+397,
yycrank+0,	yysvec+33,	yyvstop+400,
yycrank+428,	yysvec+33,	yyvstop+403,
yycrank+430,	yysvec+33,	yyvstop+405,
yycrank+430,	yysvec+33,	yyvstop+407,
yycrank+424,	yysvec+33,	yyvstop+409,
yycrank+429,	yysvec+33,	yyvstop+411,
yycrank+0,	yysvec+33,	yyvstop+413,
yycrank+0,	yysvec+33,	yyvstop+416,
yycrank+431,	yysvec+33,	yyvstop+419,
yycrank+455,	yysvec+33,	yyvstop+421,
yycrank+469,	yysvec+33,	yyvstop+423,
yycrank+470,	yysvec+33,	yyvstop+425,
yycrank+449,	yysvec+33,	yyvstop+427,
yycrank+0,	yysvec+33,	yyvstop+429,
yycrank+467,	yysvec+33,	yyvstop+432,
yycrank+467,	yysvec+33,	yyvstop+434,
yycrank+491,	yysvec+33,	yyvstop+436,
yycrank+485,	yysvec+33,	yyvstop+438,
yycrank+494,	yysvec+33,	yyvstop+441,
yycrank+559,	yysvec+33,	yyvstop+443,
yycrank+572,	yysvec+33,	yyvstop+445,
yycrank+578,	yysvec+33,	yyvstop+447,
yycrank+675,	yysvec+33,	yyvstop+449,
yycrank+0,	yysvec+33,	yyvstop+451,
yycrank+601,	yysvec+33,	yyvstop+454,
yycrank+595,	yysvec+33,	yyvstop+456,
yycrank+0,	yysvec+33,	yyvstop+458,
yycrank+613,	yysvec+33,	yyvstop+461,
yycrank+621,	yysvec+33,	yyvstop+463,
yycrank+0,	yysvec+33,	yyvstop+465,
yycrank+620,	yysvec+33,	yyvstop+468,
yycrank+610,	yysvec+33,	yyvstop+470,
yycrank+611,	yysvec+33,	yyvstop+472,
yycrank+637,	yysvec+33,	yyvstop+474,
yycrank+624,	yysvec+33,	yyvstop+476,
yycrank+0,	yysvec+33,	yyvstop+478,
yycrank+639,	yysvec+33,	yyvstop+481,
yycrank+0,	yysvec+33,	yyvstop+483,
yycrank+0,	yysvec+33,	yyvstop+486,
yycrank+641,	yysvec+33,	yyvstop+489,
yycrank+627,	yysvec+33,	yyvstop+491,
yycrank+628,	yysvec+33,	yyvstop+493,
yycrank+638,	yysvec+33,	yyvstop+495,
yycrank+645,	yysvec+33,	yyvstop+497,
yycrank+651,	yysvec+33,	yyvstop+500,
yycrank+649,	yysvec+33,	yyvstop+502,
yycrank+641,	yysvec+33,	yyvstop+504,
yycrank+656,	yysvec+33,	yyvstop+506,
yycrank+750,	0,		yyvstop+508,
yycrank+0,	yysvec+33,	yyvstop+510,
yycrank+657,	yysvec+33,	yyvstop+513,
yycrank+644,	yysvec+33,	yyvstop+515,
yycrank+657,	yysvec+33,	yyvstop+517,
yycrank+662,	yysvec+33,	yyvstop+519,
yycrank+648,	yysvec+33,	yyvstop+521,
yycrank+651,	yysvec+33,	yyvstop+523,
yycrank+656,	yysvec+33,	yyvstop+525,
yycrank+673,	yysvec+33,	yyvstop+528,
yycrank+751,	yysvec+33,	yyvstop+530,
yycrank+681,	yysvec+33,	yyvstop+532,
yycrank+687,	yysvec+33,	yyvstop+534,
yycrank+0,	yysvec+33,	yyvstop+536,
yycrank+677,	yysvec+33,	yyvstop+539,
yycrank+678,	yysvec+33,	yyvstop+541,
yycrank+691,	yysvec+33,	yyvstop+544,
yycrank+-786,	yysvec+164,	yyvstop+546,
yycrank+-787,	yysvec+164,	yyvstop+548,
yycrank+-797,	0,		yyvstop+551,
yycrank+-799,	0,		yyvstop+554,
yycrank+-789,	yysvec+164,	yyvstop+557,
yycrank+-791,	yysvec+164,	yyvstop+559,
yycrank+-792,	yysvec+164,	yyvstop+561,
yycrank+-793,	yysvec+164,	yyvstop+563,
yycrank+-794,	yysvec+164,	yyvstop+565,
yycrank+-795,	yysvec+164,	yyvstop+567,
yycrank+-800,	yysvec+164,	yyvstop+570,
yycrank+-801,	yysvec+164,	yyvstop+572,
yycrank+-802,	yysvec+164,	yyvstop+574,
yycrank+-803,	yysvec+164,	yyvstop+577,
yycrank+-804,	yysvec+164,	yyvstop+579,
yycrank+-806,	yysvec+164,	yyvstop+582,
yycrank+-805,	yysvec+164,	yyvstop+584,
yycrank+-807,	yysvec+164,	yyvstop+586,
yycrank+-808,	yysvec+164,	yyvstop+589,
yycrank+-812,	yysvec+164,	yyvstop+591,
yycrank+-809,	yysvec+164,	yyvstop+593,
yycrank+-810,	yysvec+164,	yyvstop+596,
yycrank+-811,	yysvec+164,	yyvstop+598,
yycrank+-814,	yysvec+164,	yyvstop+600,
yycrank+-815,	yysvec+164,	yyvstop+602,
yycrank+724,	yysvec+33,	yyvstop+604,
yycrank+712,	0,		yyvstop+606,
yycrank+714,	0,		0,	
yycrank+716,	0,		0,	
yycrank+0,	0,		yyvstop+608,
yycrank+730,	0,		0,	
yycrank+719,	0,		0,	
yycrank+735,	0,		0,	
yycrank+721,	0,		0,	
yycrank+733,	0,		yyvstop+610,
yycrank+735,	0,		0,	
yycrank+724,	0,		0,	
yycrank+727,	0,		0,	
yycrank+0,	0,		yyvstop+612,
yycrank+741,	0,		0,	
yycrank+727,	0,		0,	
yycrank+728,	0,		yyvstop+614,
yycrank+0,	yysvec+33,	yyvstop+616,
yycrank+745,	yysvec+33,	yyvstop+619,
yycrank+729,	yysvec+33,	yyvstop+621,
yycrank+799,	yysvec+33,	yyvstop+623,
yycrank+731,	yysvec+33,	yyvstop+625,
yycrank+750,	yysvec+33,	yyvstop+627,
yycrank+741,	yysvec+33,	yyvstop+629,
yycrank+752,	yysvec+33,	yyvstop+631,
yycrank+0,	yysvec+33,	yyvstop+633,
yycrank+746,	yysvec+33,	yyvstop+636,
yycrank+748,	yysvec+33,	yyvstop+638,
yycrank+0,	yysvec+33,	yyvstop+640,
yycrank+755,	yysvec+33,	yyvstop+643,
yycrank+747,	yysvec+33,	yyvstop+646,
yycrank+757,	yysvec+33,	yyvstop+648,
yycrank+758,	yysvec+33,	yyvstop+651,
yycrank+0,	yysvec+33,	yyvstop+653,
yycrank+828,	yysvec+33,	yyvstop+656,
yycrank+749,	yysvec+33,	yyvstop+658,
yycrank+0,	yysvec+33,	yyvstop+660,
yycrank+831,	0,		0,	
yycrank+0,	yysvec+33,	yyvstop+663,
yycrank+749,	yysvec+33,	yyvstop+666,
yycrank+0,	yysvec+33,	yyvstop+668,
yycrank+763,	yysvec+33,	yyvstop+671,
yycrank+754,	yysvec+33,	yyvstop+673,
yycrank+0,	yysvec+33,	yyvstop+675,
yycrank+838,	yysvec+33,	yyvstop+678,
yycrank+0,	yysvec+33,	yyvstop+681,
yycrank+755,	yysvec+33,	yyvstop+684,
yycrank+758,	yysvec+33,	yyvstop+686,
yycrank+0,	yysvec+33,	yyvstop+688,
yycrank+769,	yysvec+33,	yyvstop+691,
yycrank+0,	yysvec+33,	yyvstop+693,
yycrank+758,	yysvec+33,	yyvstop+696,
yycrank+758,	yysvec+33,	yyvstop+698,
yycrank+0,	yysvec+33,	yyvstop+700,
yycrank+760,	yysvec+33,	yyvstop+703,
yycrank+761,	yysvec+33,	yyvstop+705,
yycrank+0,	yysvec+33,	yyvstop+707,
yycrank+0,	yysvec+33,	yyvstop+710,
yycrank+774,	yysvec+33,	yyvstop+713,
yycrank+769,	yysvec+33,	yyvstop+715,
yycrank+783,	yysvec+33,	yyvstop+717,
yycrank+0,	yysvec+33,	yyvstop+719,
yycrank+765,	yysvec+33,	yyvstop+722,
yycrank+0,	yysvec+33,	yyvstop+724,
yycrank+765,	yysvec+33,	yyvstop+727,
yycrank+0,	yysvec+33,	yyvstop+729,
yycrank+851,	0,		0,	
yycrank+776,	yysvec+33,	yyvstop+732,
yycrank+771,	yysvec+33,	yyvstop+734,
yycrank+854,	yysvec+33,	yyvstop+736,
yycrank+783,	yysvec+33,	yyvstop+738,
yycrank+0,	yysvec+33,	yyvstop+740,
yycrank+-878,	yysvec+164,	yyvstop+743,
yycrank+-879,	yysvec+164,	yyvstop+745,
yycrank+-880,	yysvec+164,	yyvstop+748,
yycrank+-881,	yysvec+164,	yyvstop+751,
yycrank+-882,	yysvec+164,	yyvstop+753,
yycrank+-884,	yysvec+164,	yyvstop+755,
yycrank+-885,	yysvec+164,	yyvstop+758,
yycrank+-886,	yysvec+164,	yyvstop+760,
yycrank+-888,	yysvec+164,	yyvstop+762,
yycrank+-887,	yysvec+164,	yyvstop+764,
yycrank+-890,	yysvec+164,	yyvstop+766,
yycrank+-900,	yysvec+164,	yyvstop+769,
yycrank+-893,	yysvec+164,	yyvstop+771,
yycrank+-894,	yysvec+164,	yyvstop+773,
yycrank+-901,	yysvec+164,	yyvstop+775,
yycrank+-904,	yysvec+164,	yyvstop+778,
yycrank+-902,	yysvec+164,	yyvstop+780,
yycrank+-903,	yysvec+164,	yyvstop+782,
yycrank+805,	yysvec+33,	yyvstop+785,
yycrank+807,	0,		0,	
yycrank+818,	0,		0,	
yycrank+0,	0,		yyvstop+787,
yycrank+817,	0,		0,	
yycrank+0,	0,		yyvstop+789,
yycrank+810,	0,		0,	
yycrank+824,	0,		0,	
yycrank+813,	0,		0,	
yycrank+815,	0,		0,	
yycrank+828,	0,		0,	
yycrank+817,	0,		0,	
yycrank+820,	0,		0,	
yycrank+0,	0,		yyvstop+791,
yycrank+831,	0,		0,	
yycrank+0,	yysvec+33,	yyvstop+793,
yycrank+0,	yysvec+33,	yyvstop+796,
yycrank+0,	yysvec+33,	yyvstop+799,
yycrank+0,	yysvec+33,	yyvstop+802,
yycrank+835,	yysvec+33,	yyvstop+805,
yycrank+828,	yysvec+33,	yyvstop+807,
yycrank+824,	yysvec+33,	yyvstop+809,
yycrank+838,	yysvec+33,	yyvstop+811,
yycrank+821,	yysvec+33,	yyvstop+813,
yycrank+841,	yysvec+33,	yyvstop+815,
yycrank+842,	yysvec+33,	yyvstop+817,
yycrank+828,	yysvec+33,	yyvstop+819,
yycrank+845,	yysvec+33,	yyvstop+821,
yycrank+914,	0,		0,	
yycrank+832,	yysvec+33,	yyvstop+823,
yycrank+846,	0,		0,	
yycrank+0,	yysvec+33,	yyvstop+825,
yycrank+833,	yysvec+33,	yyvstop+828,
yycrank+845,	yysvec+33,	yyvstop+830,
yycrank+919,	0,		0,	
yycrank+0,	yysvec+33,	yyvstop+833,
yycrank+920,	yysvec+33,	yyvstop+836,
yycrank+921,	yysvec+33,	yyvstop+838,
yycrank+0,	yysvec+33,	yyvstop+840,
yycrank+839,	yysvec+33,	yyvstop+843,
yycrank+923,	yysvec+33,	yyvstop+845,
yycrank+0,	yysvec+33,	yyvstop+848,
yycrank+924,	yysvec+33,	yyvstop+851,
yycrank+856,	yysvec+33,	yyvstop+853,
yycrank+858,	yysvec+33,	yyvstop+855,
yycrank+927,	yysvec+33,	yyvstop+857,
yycrank+857,	yysvec+33,	yyvstop+859,
yycrank+859,	0,		0,	
yycrank+0,	yysvec+33,	yyvstop+861,
yycrank+931,	yysvec+33,	yyvstop+864,
yycrank+932,	0,		0,	
yycrank+0,	yysvec+33,	yyvstop+866,
yycrank+-955,	yysvec+164,	yyvstop+869,
yycrank+-956,	yysvec+164,	yyvstop+871,
yycrank+-957,	yysvec+164,	yyvstop+873,
yycrank+-958,	yysvec+164,	yyvstop+876,
yycrank+-959,	yysvec+164,	yyvstop+878,
yycrank+-960,	yysvec+164,	yyvstop+881,
yycrank+-961,	yysvec+164,	yyvstop+883,
yycrank+-962,	yysvec+164,	yyvstop+885,
yycrank+-964,	yysvec+164,	yyvstop+887,
yycrank+-963,	yysvec+164,	yyvstop+889,
yycrank+-965,	yysvec+164,	yyvstop+891,
yycrank+-968,	yysvec+164,	yyvstop+893,
yycrank+-966,	yysvec+164,	yyvstop+895,
yycrank+-967,	yysvec+164,	yyvstop+898,
yycrank+865,	yysvec+33,	yyvstop+900,
yycrank+871,	0,		0,	
yycrank+867,	0,		0,	
yycrank+866,	0,		0,	
yycrank+955,	0,		0,	
yycrank+0,	0,		yyvstop+902,
yycrank+868,	0,		0,	
yycrank+0,	0,		yyvstop+904,
yycrank+0,	0,		yyvstop+906,
yycrank+0,	0,		yyvstop+908,
yycrank+962,	0,		0,	
yycrank+0,	0,		yyvstop+910,
yycrank+875,	yysvec+33,	yyvstop+912,
yycrank+954,	yysvec+33,	yyvstop+914,
yycrank+958,	yysvec+33,	yyvstop+916,
yycrank+0,	yysvec+33,	yyvstop+918,
yycrank+886,	yysvec+33,	yyvstop+921,
yycrank+0,	yysvec+33,	yyvstop+923,
yycrank+983,	yysvec+33,	yyvstop+926,
yycrank+892,	yysvec+33,	yyvstop+928,
yycrank+0,	yysvec+33,	yyvstop+930,
yycrank+897,	0,		0,	
yycrank+899,	yysvec+33,	yyvstop+933,
yycrank+0,	0,		yyvstop+935,
yycrank+0,	yysvec+33,	yyvstop+937,
yycrank+903,	yysvec+33,	yyvstop+940,
yycrank+904,	0,		0,	
yycrank+975,	0,		0,	
yycrank+979,	0,		0,	
yycrank+0,	yysvec+33,	yyvstop+942,
yycrank+980,	0,		0,	
yycrank+981,	0,		0,	
yycrank+0,	yysvec+33,	yyvstop+945,
yycrank+0,	yysvec+33,	yyvstop+948,
yycrank+982,	0,		0,	
yycrank+906,	yysvec+33,	yyvstop+951,
yycrank+0,	0,		yyvstop+953,
yycrank+988,	0,		0,	
yycrank+914,	0,		0,	
yycrank+-1007,	yysvec+164,	yyvstop+955,
yycrank+-1011,	yysvec+164,	yyvstop+957,
yycrank+-1012,	yysvec+164,	yyvstop+959,
yycrank+-1013,	yysvec+164,	yyvstop+961,
yycrank+-1014,	yysvec+164,	yyvstop+963,
yycrank+-1016,	yysvec+164,	yyvstop+966,
yycrank+-1017,	yysvec+164,	yyvstop+968,
yycrank+-1018,	yysvec+164,	yyvstop+971,
yycrank+-1019,	yysvec+164,	yyvstop+974,
yycrank+-1021,	yysvec+164,	yyvstop+977,
yycrank+-1022,	yysvec+164,	yyvstop+979,
yycrank+1023,	yysvec+33,	yyvstop+982,
yycrank+0,	0,		yyvstop+984,
yycrank+0,	0,		yyvstop+986,
yycrank+0,	0,		yyvstop+988,
yycrank+933,	0,		0,	
yycrank+930,	0,		0,	
yycrank+0,	0,		yyvstop+990,
yycrank+935,	0,		0,	
yycrank+932,	0,		0,	
yycrank+0,	yysvec+33,	yyvstop+992,
yycrank+1006,	0,		0,	
yycrank+1007,	0,		0,	
yycrank+925,	yysvec+33,	yyvstop+995,
yycrank+1032,	0,		yyvstop+997,
yycrank+928,	yysvec+33,	yyvstop+999,
yycrank+0,	0,		yyvstop+1001,
yycrank+0,	yysvec+33,	yyvstop+1003,
yycrank+936,	yysvec+33,	yyvstop+1006,
yycrank+0,	0,		yyvstop+1008,
yycrank+945,	0,		0,	
yycrank+942,	0,		0,	
yycrank+946,	0,		0,	
yycrank+947,	0,		0,	
yycrank+948,	0,		0,	
yycrank+949,	0,		0,	
yycrank+951,	0,		0,	
yycrank+949,	0,		0,	
yycrank+0,	0,		yyvstop+1010,
yycrank+-1045,	yysvec+164,	yyvstop+1012,
yycrank+-1046,	yysvec+164,	yyvstop+1015,
yycrank+-1048,	yysvec+164,	yyvstop+1018,
yycrank+-1049,	yysvec+164,	yyvstop+1021,
yycrank+-1050,	yysvec+164,	yyvstop+1023,
yycrank+-1051,	yysvec+164,	yyvstop+1025,
yycrank+-1058,	yysvec+164,	yyvstop+1028,
yycrank+-1062,	yysvec+164,	yyvstop+1030,
yycrank+0,	0,		yyvstop+1032,
yycrank+971,	0,		0,	
yycrank+972,	0,		0,	
yycrank+975,	0,		0,	
yycrank+975,	0,		0,	
yycrank+982,	0,		0,	
yycrank+983,	0,		0,	
yycrank+985,	yysvec+33,	yyvstop+1034,
yycrank+0,	yysvec+33,	yyvstop+1036,
yycrank+986,	yysvec+33,	yyvstop+1039,
yycrank+986,	0,		0,	
yycrank+991,	0,		0,	
yycrank+0,	0,		yyvstop+1041,
yycrank+0,	0,		yyvstop+1043,
yycrank+0,	0,		yyvstop+1045,
yycrank+0,	0,		yyvstop+1047,
yycrank+993,	0,		0,	
yycrank+994,	0,		0,	
yycrank+-1088,	yysvec+164,	yyvstop+1049,
yycrank+-1089,	yysvec+164,	yyvstop+1051,
yycrank+-1091,	yysvec+164,	yyvstop+1053,
yycrank+-1093,	yysvec+164,	yyvstop+1055,
yycrank+984,	0,		0,	
yycrank+1000,	0,		0,	
yycrank+989,	0,		0,	
yycrank+1002,	0,		0,	
yycrank+0,	0,		yyvstop+1057,
yycrank+0,	0,		yyvstop+1059,
yycrank+0,	yysvec+33,	yyvstop+1061,
yycrank+0,	yysvec+33,	yyvstop+1064,
yycrank+991,	0,		0,	
yycrank+1004,	0,		0,	
yycrank+993,	0,		0,	
yycrank+1006,	0,		0,	
yycrank+-1101,	yysvec+164,	yyvstop+1067,
yycrank+-1102,	yysvec+164,	yyvstop+1069,
yycrank+-1103,	yysvec+164,	yyvstop+1071,
yycrank+-1104,	yysvec+164,	yyvstop+1073,
yycrank+0,	0,		yyvstop+1075,
yycrank+999,	0,		0,	
yycrank+0,	0,		yyvstop+1077,
yycrank+1003,	0,		0,	
yycrank+0,	0,		yyvstop+1079,
yycrank+1004,	0,		0,	
yycrank+0,	0,		yyvstop+1081,
yycrank+1006,	0,		0,	
yycrank+-1113,	yysvec+164,	yyvstop+1083,
yycrank+-1114,	yysvec+164,	yyvstop+1086,
yycrank+-1116,	yysvec+164,	yyvstop+1088,
yycrank+-1120,	yysvec+164,	yyvstop+1091,
yycrank+0,	0,		yyvstop+1093,
yycrank+0,	0,		yyvstop+1095,
yycrank+0,	0,		yyvstop+1097,
yycrank+0,	0,		yyvstop+1099,
yycrank+-1122,	yysvec+164,	yyvstop+1101,
yycrank+-1123,	yysvec+164,	yyvstop+1104,
0,	0,	0};
struct yywork *yytop = yycrank+1236;
struct yysvf *yybgin = yysvec+1;
char yymatch[] = {
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,011 ,012 ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
011 ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'0' ,'0' ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,'A' ,'A' ,'A' ,'A' ,'E' ,'E' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,01  ,01  ,01  ,01  ,'A' ,
01  ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,01  ,01  ,01  ,01  ,01  ,
0};
char yyextra[] = {
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
/*	@(#)ncform	UniPlus V.2.1.2	(Motorola 2.1)*/
int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
yylook(){
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych, yyfirst;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	yyfirst=1;
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank && !yyfirst){  /* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
			*yylastch++ = yych = input();
			yyfirst=0;
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (int)yyt > (int)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((int)yyt < (int)yycrank) {		/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
yyback(p, m)
	int *p;
{
if (p==0) return(0);
while (*p)
	{
	if (*p++ == m)
		return(1);
	}
return(0);
}
	/* the following are only used in the lex library */
yyinput(){
	return(input());
	}
yyoutput(c)
  int c; {
	output(c);
	}
yyunput(c)
   int c; {
	unput(c);
	}
