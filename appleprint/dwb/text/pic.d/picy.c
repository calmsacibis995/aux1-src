#ifndef lint	/* .../appleprint/dwb/text/pic.d/picy.c */
#define _AC_NAME picy_c
#define _AC_NO_MAIN "%Z% Copyright (c) ???, All Rights Reserved.  {Apple version %I% %E% %U%}"
#include <apple_notice.h>

#ifdef _AC_HISTORY
  static char *sccsid = "@(#)Copyright Apple Computer 1987\tVersion 1.1 of picy.y on 87/05/04 13:48:38";    
#endif		/* _AC_HISTORY */
#endif		/* lint */


# line 2 "picy.y"
#ifndef lint	/* .../appleprint/dwb/text/pic.d/picy.y */
#define _AC_NAME picy_y
#define _AC_NO_MAIN "%Z% Copyright (c) 1983-87 AT&T-IS, all rights reserved.  Apple version %I% %E% %U%"
#include <apple_notice.h>

#ifdef _AC_HISTORY
#define _AC_MODS
#endif		/* _AC_HISTORY */
#endif		/* lint */


# line 21 "picy.y"
#define _AC_MODS
#include <stdio.h>
#include "pic.h"
#include <math.h>
YYSTYPE	y;
# define BOX 1
# define LINE 2
# define ARROW 3
# define CIRCLE 4
# define ELLIPSE 5
# define ARC 6
# define SPLINE 7
# define BLOCK 8
# define TEXT 9
# define TROFF 10
# define MOVE 11
# define PLOT 12
# define BLOCKEND 13
# define PLACE 270
# define PRINT 271
# define THRU 272
# define UNTIL 273
# define FOR 274
# define IF 275
# define COPY 276
# define THENSTR 277
# define ELSESTR 278
# define DOSTR 279
# define DEFNAME 280
# define PLACENAME 281
# define VARNAME 282
# define ATTR 283
# define TEXTATTR 284
# define LEFT 285
# define RIGHT 286
# define UP 287
# define DOWN 288
# define FROM 289
# define TO 290
# define AT 291
# define BY 292
# define WITH 293
# define HEAD 294
# define CW 295
# define CCW 296
# define THEN 297
# define HEIGHT 298
# define WIDTH 299
# define RADIUS 300
# define DIAMETER 301
# define LENGTH 302
# define SIZE 303
# define CORNER 304
# define HERE 305
# define LAST 306
# define NTH 307
# define SAME 308
# define BETWEEN 309
# define AND 310
# define EAST 311
# define WEST 312
# define NORTH 313
# define SOUTH 314
# define NE 315
# define NW 316
# define SE 317
# define SW 318
# define START 319
# define END 320
# define DOTX 321
# define DOTY 322
# define DOTHT 323
# define DOTWID 324
# define DOTRAD 325
# define NUMBER 326
# define LOG 327
# define EXP 328
# define SIN 329
# define COS 330
# define ATAN2 331
# define SQRT 332
# define RAND 333
# define MIN 334
# define MAX 335
# define INT 336
# define DIR 337
# define DOT 338
# define DASH 339
# define CHOP 340
# define ST 341
# define OROR 342
# define ANDAND 343
# define GT 344
# define LT 345
# define LE 346
# define GE 347
# define EQ 348
# define NEQ 349
# define UMINUS 350
# define NOT 351
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
typedef int yytabelem;
# define YYERRCODE 256
yytabelem yyexca[] ={
-1, 0,
	0, 2,
	-2, 0,
-1, 1,
	0, -1,
	-2, 0,
-1, 192,
	344, 0,
	345, 0,
	346, 0,
	347, 0,
	348, 0,
	349, 0,
	-2, 138,
-1, 199,
	344, 0,
	345, 0,
	346, 0,
	347, 0,
	348, 0,
	349, 0,
	-2, 137,
-1, 200,
	344, 0,
	345, 0,
	346, 0,
	347, 0,
	348, 0,
	349, 0,
	-2, 139,
-1, 201,
	344, 0,
	345, 0,
	346, 0,
	347, 0,
	348, 0,
	349, 0,
	-2, 140,
-1, 202,
	344, 0,
	345, 0,
	346, 0,
	347, 0,
	348, 0,
	349, 0,
	-2, 141,
-1, 203,
	344, 0,
	345, 0,
	346, 0,
	347, 0,
	348, 0,
	349, 0,
	-2, 142,
-1, 242,
	344, 0,
	345, 0,
	346, 0,
	347, 0,
	348, 0,
	349, 0,
	-2, 138,
	};
# define YYNPROD 156
# define YYLAST 1604
yytabelem yyact[]={

    15,    19,    20,    16,    17,    18,    21,    38,    32,    25,
    22,    23,   186,   187,   118,    35,   141,   282,   140,   211,
   123,   124,   125,   126,   127,   228,    46,   116,   140,   117,
    37,    45,   237,   226,   158,   184,   177,    91,   116,    46,
   117,    65,   143,   116,    45,   117,   116,    87,   117,   297,
   268,    90,   267,   107,    59,   266,   223,   181,   105,   103,
   246,   104,   185,   106,    82,    80,   107,    37,   145,   229,
    62,   105,   103,   100,   104,   278,   106,    60,    61,    65,
    63,   107,    75,   116,    41,   117,   105,    45,    83,   227,
    33,   106,   148,   152,   153,   149,   150,   151,   154,   156,
     4,   300,   298,    34,   299,    64,   301,   173,    24,   140,
    24,   107,   138,   137,    24,    46,   105,   103,   136,   104,
    45,   106,    27,   148,   152,   153,   149,   150,   151,   154,
   146,   142,   155,   135,    24,   134,   133,    34,    93,   132,
    66,   116,   249,   117,    24,    24,   180,   210,    75,   116,
   131,   117,   130,    45,   129,   248,   247,    67,    68,    69,
    70,    71,    72,    73,     2,    77,   144,    78,   182,   159,
   243,    36,    81,     1,     5,    26,     6,    11,    12,    13,
    34,    46,    86,    88,     0,     0,    45,    24,     0,    44,
     8,    79,     8,     0,     0,   188,     8,     0,     0,     0,
     0,     0,    24,     0,     0,     0,     0,     0,     0,     0,
     0,   224,   225,     0,   209,   174,     8,     0,     0,    45,
     0,     0,     0,     0,     0,     0,     8,    97,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,   107,     0,     0,     0,   207,   105,   103,
   100,   104,    45,   106,   107,     0,     0,     0,   212,   105,
   103,   100,   104,   183,   106,     0,     0,    59,    43,     8,
    10,     0,     0,    30,    31,    29,   139,   107,     0,     0,
    96,    43,   105,   103,     8,   104,   139,   106,     0,     0,
   236,    61,    65,    63,   250,   294,   123,   124,   125,   126,
   127,     0,     0,    60,    61,    65,    63,   142,   293,     0,
     0,     0,    42,    49,    50,    51,    52,    53,    54,    55,
    57,    56,    58,    59,    43,    42,    49,    50,    51,    52,
    53,    54,    55,    57,    56,    58,     9,    48,   102,   285,
    94,   189,   144,   302,   115,     0,    60,    61,    65,    63,
    48,    84,    85,     0,     0,     0,    59,    43,   114,   113,
   108,   175,   109,   110,   111,   112,     0,     0,    42,    49,
    50,    51,    52,    53,    54,    55,    57,    56,    58,    60,
    61,    65,    63,     0,   269,     0,   107,     0,     0,    59,
    43,   105,   103,    48,   104,     0,   106,   157,     0,     0,
     0,    42,    49,    50,    51,    52,    53,    54,    55,    57,
    56,    58,    60,    61,    65,    63,     0,     0,     0,   107,
     0,     0,    59,    43,   105,   103,    48,   104,   147,   106,
     0,     0,     0,     0,    42,    49,    50,    51,    52,    53,
    54,    55,    57,    56,    58,    60,    61,    65,    63,     0,
     0,     0,   107,     0,     0,    59,    43,   105,   103,    48,
   104,     0,   106,     0,     0,     0,     0,    42,    49,    50,
    51,    52,    53,    54,    55,    57,    56,    58,    60,    61,
    65,    63,     0,     0,     0,   107,     0,     0,    59,    43,
   105,   103,    48,   104,     0,   106,     0,     0,     0,     0,
    42,    49,    50,    51,    52,    53,    54,    55,    57,    56,
    58,    60,    61,    65,    63,   102,     0,     0,     0,   292,
     0,     0,     0,     0,     0,    48,   102,     0,     0,     0,
     0,     0,   291,    42,    49,    50,    51,    52,    53,    54,
    55,    57,    56,    58,     0,     0,     0,    99,   114,   113,
   108,   101,   109,   110,   111,   112,     0,     0,    48,   114,
   113,   108,   101,   109,   110,   111,   112,   107,     0,     0,
     0,   212,   105,   103,   277,   104,     0,   106,     0,     0,
     0,     0,   114,   113,   108,   175,   109,   110,   111,   112,
   107,     0,     0,     0,   212,   105,   103,   275,   104,   107,
   106,     0,     0,     0,   105,   103,   100,   104,   107,   106,
     0,     0,   296,   105,   103,     0,   104,   107,   106,     0,
     0,   295,   105,   103,     0,   104,   107,   106,   306,     0,
   290,   105,   103,     0,   104,   107,   106,     0,     0,   289,
   105,   103,     0,   104,   107,   106,     0,     0,   288,   105,
   103,     0,   104,     0,   106,     0,     0,     0,   107,     0,
     0,   305,   265,   105,   103,     0,   104,   107,   106,     0,
     0,     0,   105,   103,   264,   104,   107,   106,     0,     0,
     0,   105,   103,   263,   104,     0,   106,     0,     0,     0,
     0,   114,   113,   108,   175,   109,   110,   111,   112,     0,
     0,     0,   107,     0,     0,   271,   262,   105,   103,     0,
   104,   107,   106,     0,     0,   261,   105,   103,     0,   104,
     0,   106,     0,     0,   114,   113,   108,   175,   109,   110,
   111,   112,     0,     0,   107,     0,     0,     0,   270,   105,
   103,   260,   104,   107,   106,     0,     0,   259,   105,   103,
     0,   104,     0,   106,     0,     0,     0,   114,   113,   108,
   175,   109,   110,   111,   112,   107,     0,     0,     0,   258,
   105,   103,     0,   104,   107,   106,     0,     0,   257,   105,
   103,     0,   104,     0,   106,     0,     0,     0,     0,     0,
   114,   113,   108,   175,   109,   110,   111,   112,   107,     0,
     0,     0,   256,   105,   103,     0,   104,   107,   106,     0,
     0,     0,   105,   103,   253,   104,   107,   106,     0,     0,
     0,   105,   103,   251,   104,   107,   106,     0,     0,   212,
   105,   103,   107,   104,     0,   106,     0,   105,   103,     0,
   104,     0,   106,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   107,     0,
     0,     0,     0,   105,   103,     0,   104,     0,   106,     0,
     0,   102,   114,   113,   108,   175,   109,   110,   111,   112,
     0,    15,    19,    20,    16,    17,    18,    21,     0,    32,
    25,    22,    23,     0,     0,   114,   113,   108,   175,   109,
   110,   111,   112,     0,   114,   113,   108,   101,   109,   110,
   111,   112,     0,   114,   113,   108,   175,   109,   110,   111,
   112,    32,   114,   113,   108,   175,   109,   110,   111,   112,
     0,   114,   113,   108,   175,   109,   110,   111,   112,     0,
   114,   113,   108,   175,   109,   110,   111,   112,     0,   114,
   113,   108,   175,   109,   110,   111,   112,     0,     0,     0,
     0,     0,     0,   114,   113,   108,   175,   109,   110,   111,
   112,    33,   114,   113,   108,   175,   109,   110,   111,   112,
     0,   114,   113,   108,   175,   109,   110,   111,   112,     0,
     0,     0,     0,     0,     0,     0,   107,     0,     0,     0,
     0,   105,   103,    27,   104,    92,   106,   114,   113,   108,
   175,   109,   110,   111,   112,     0,   114,   113,   108,   175,
   109,   110,   111,   112,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,   114,
   113,   108,   175,   109,   110,   111,   112,     0,   114,   113,
   108,   175,   109,   110,   111,   112,    15,    19,    20,    16,
    17,    18,    21,     0,    32,    25,    22,    23,     0,     0,
   114,   113,   108,   175,   109,   110,   111,   112,     0,   114,
   113,   108,   175,   109,   110,   111,   112,    15,    19,    20,
    16,    17,    18,    21,     0,    32,    25,    22,    23,     0,
     0,     0,     0,   114,   113,   108,   175,   109,   110,   111,
   112,     0,   114,   113,   108,   175,   109,   110,   111,   112,
     0,   114,   113,   108,   175,   109,   110,   111,   112,     0,
   114,   113,   108,   175,   109,   110,   111,   112,   113,   108,
   175,   109,   110,   111,   112,     0,    33,     0,   178,     0,
     0,    10,     0,     0,    30,    31,    29,     0,     0,     0,
     0,     7,    28,   114,   113,   108,   175,   109,   110,   111,
   112,     0,     0,     0,     0,     0,     0,    33,    27,    15,
    19,    20,    16,    17,    18,    21,     0,    32,    25,    22,
    23,     0,     0,     0,     0,   160,   168,     0,     0,     0,
     0,   162,   163,   164,   165,   166,   169,    76,     0,    27,
     0,     0,     0,     0,     0,     0,     0,     9,    47,    40,
   167,    14,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    47,     0,     0,     0,   161,
   170,   171,   172,     0,   121,     0,     0,    95,     0,     0,
     0,     0,     0,     0,     0,     0,   120,     0,     0,    33,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    98,    27,     0,   108,   175,   109,   110,   111,   112,    47,
    47,    39,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   191,   193,     0,    74,     0,    10,     0,     0,    30,
    31,    29,    89,     0,     0,     0,     7,    28,     0,     0,
     0,     0,     3,     0,     0,     0,   119,   122,     0,   128,
     0,     0,     0,     0,     0,     0,     0,    10,     0,     0,
    30,    31,    29,     0,     0,     0,     0,     7,    28,     0,
    47,    47,    47,    47,    47,     0,   176,     0,     0,     0,
     0,   179,   232,   233,   234,   235,   238,     0,     0,     0,
     0,     0,     9,     0,     0,     0,    14,     0,     0,     0,
     0,   190,   192,     0,   194,   195,   196,   197,   198,   199,
   200,   201,   202,   203,   204,   205,     0,   206,   208,   255,
     0,     0,     0,     9,     0,     0,     0,    14,     0,     0,
   213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    10,
     0,     0,    30,    31,    29,     0,     0,    47,    47,     7,
    28,   230,   231,     0,     0,     0,     0,     0,     0,   272,
   273,   239,   240,   241,     0,     0,   242,     0,     0,     0,
     0,     0,     0,     0,   244,   245,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   252,     0,
   254,     0,     0,     0,     0,     9,     0,     0,     0,    14,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,   274,     0,   276,     0,     0,     0,     0,     0,
     0,   279,     0,     0,   280,   281,     0,     0,     0,     0,
     0,   283,   284,     0,     0,     0,   286,     0,   287,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   303,     0,
     0,     0,     0,   304 };
yytabelem yypact[]={

  1086, -1000,  1178, -1000, -1000,  -326,  1178,     9,  -334, -1000,
    75, -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000,
 -1000, -1000, -1000,   108,   158, -1000,  1178, -1000,     4,    79,
  -235,    42,  -247, -1000, -1000, -1000,   880,    -1, -1000,   206,
     3,  -327, -1000,     4, -1000,   108,   141,  -301,   108,   114,
   112,   110,    99,    96,    95,    93,    78,    73,    72,   -18,
  -265, -1000,   122,    91,  -270, -1000,   912,   912,   912,   912,
   912,   912,   912,   912,   821,   108,  -301,   912,  -248,  1055,
   108,    79, -1000, -1000,  -223,   159,   -26, -1000,  -215,   821,
  -336, -1000, -1000, -1000,  1178,     0,   -28,  -334,   562, -1000,
   108,   141,   141,   108,   108,   108,   108,   108,   108,   108,
   108,   108,   108,   108,   108, -1000,   207,   174, -1000, -1000,
   106,   -25,   217, -1000, -1000, -1000, -1000, -1000, -1000,   108,
   108,   108,   108,   108,   108,   108,   108,   108,   108, -1000,
  -225,    63,   122,    91, -1000,  -271,    43, -1000, -1000, -1000,
 -1000, -1000, -1000, -1000, -1000,  -279,    23, -1000, -1000, -1000,
   108,   108,   141,   141,   141,   141,   -14, -1000, -1000, -1000,
   108,   108,   108,   158,   912,   108,   788, -1000, -1000,   821,
 -1000, -1000, -1000,   108,   108,  -218,   147,   146, -1000, -1000,
   821,    98,    29,   -16,    44,    44, -1000, -1000, -1000,    74,
    74,    74,    74,    74,   959,   795,   779,   108,   770,   108,
 -1000,  -227, -1000,   761,   737,   728,   706,   697,   674,   665,
   639,   630,   621, -1000, -1000, -1000, -1000,  -226, -1000,  -229,
   821,   821,    40,    40,    40,    40,  -265,  -231,    40,   821,
   821,   821,    74, -1000,   448,   415, -1000, -1000, -1000,   141,
   141,   108,   553,   108,   530,    34, -1000, -1000, -1000, -1000,
   108, -1000, -1000,   108,   108, -1000, -1000, -1000,  -287,   912,
   108,   108,    -5,    40,   821,   108,   821,   108, -1000,   607,
   598,   589, -1000,   240,    16, -1000,   580,   571, -1000, -1000,
 -1000,    59, -1000,    59, -1000, -1000, -1000,   108, -1000, -1000,
 -1000, -1000,   108,   382,   349, -1000, -1000 };
yytabelem yypgo[]={

     0,  1300,   183,   189,   182,    49,   179,   178,   177,   176,
   100,   164,  1219,   175,   174,  1207,   105,   107,    70,    68,
   173,   172,    64,   140,   170,   169 };
yytabelem yyr1[]={

     0,    20,    20,    20,    11,    11,    10,    10,    10,    10,
    10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
     3,     8,    21,    21,    22,    22,    22,     7,     7,     7,
     7,     6,     6,     2,     2,     2,     4,     5,     5,     5,
     5,     5,     9,    14,    14,    14,    14,    14,    14,    14,
    14,    14,    14,    14,    24,    14,    13,    23,    23,    25,
    25,    25,    25,    25,    25,    25,    25,    25,    25,    25,
    25,    25,    25,    25,    25,    25,    25,    25,    25,    25,
    25,    17,    17,    17,    17,    12,    12,    12,    12,    12,
    12,    12,    12,    12,    12,    15,    15,    15,    15,    15,
    15,    15,    15,    15,    15,    15,    15,    15,    16,    16,
    16,    18,    18,    18,    19,    19,    19,    19,    19,    19,
    19,    19,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1 };
yytabelem yyr2[]={

     0,     2,     0,     3,     2,     4,     5,     7,     7,     9,
     9,     5,     3,     7,     7,     7,     2,     2,     2,     2,
     7,     5,     2,     4,     3,     5,     5,    21,    15,    21,
    15,     9,     7,     2,     7,     7,     3,     3,     3,     3,
     3,     1,     3,     5,     5,     5,     5,     5,     5,     5,
     5,     7,     5,     3,     1,    11,     3,     4,     0,     5,
     3,     5,     3,     5,     5,     5,     5,     5,     7,     9,
     5,     3,     3,     3,     5,     3,     5,     3,     5,     3,
     2,     3,     5,     5,     7,     2,     7,     7,    11,    11,
    15,    15,    11,    13,    11,     3,     5,     5,     3,     5,
     7,     7,     5,     7,     7,     2,     5,     5,     9,     9,
     7,     5,     5,     3,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     3,     2,     7,     7,     7,     7,     7,
     5,     7,     5,     5,     5,     5,     5,     7,     7,     7,
     7,     7,     7,     7,     7,     5,     9,     9,     9,     9,
    13,     9,     9,    13,    13,     9 };
yytabelem yychk[]={

 -1000,   -20,   -11,   256,   -10,   -14,    -9,   281,    -3,   337,
   271,    -8,    -7,    -6,   341,     1,     4,     5,     6,     2,
     3,     7,    11,    12,   -17,    10,   -13,   123,   282,   276,
   274,   275,     9,    91,   -10,   341,   -11,    58,   341,    -1,
   -12,     9,   326,   282,    -3,    45,    40,   -15,   351,   327,
   328,   329,   330,   331,   332,   333,   335,   334,   336,   281,
   304,   305,   -18,   307,   -16,   306,   -23,   -23,   -23,   -23,
   -23,   -23,   -23,   -23,    -1,    40,   -15,   -23,     9,   -11,
    61,   -21,   -22,     9,   272,   273,    -4,   282,    -2,    -1,
     9,   284,   125,   -10,   341,   -12,   281,    -3,    -1,   341,
    44,   345,   309,    43,    45,    42,    47,    37,   344,   346,
   347,   348,   349,   343,   342,   341,    43,    45,   341,    -1,
   -12,   -15,    -1,   321,   322,   323,   324,   325,    -1,    40,
    40,    40,    40,    40,    40,    40,    40,    40,    40,   304,
    46,   281,   -18,   307,   -16,   -19,     8,   306,     1,     4,
     5,     6,     2,     3,     7,   -19,     8,   306,   304,   -25,
   283,   337,   289,   290,   291,   292,   293,   308,   284,   294,
   338,   339,   340,   -17,   -23,   345,    -1,   284,    93,    -1,
   -22,   280,     9,   289,    61,   277,   348,   349,   -10,   341,
    -1,   -12,    -1,   -12,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    40,    -1,    40,
    41,    44,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,   281,   -19,   -19,   304,    46,   304,    46,
    -1,    -1,   -12,   -12,   -12,   -12,   304,    46,   -12,    -1,
    -1,    -1,    -1,   -24,    -1,    -1,   278,     9,     9,    44,
   310,    44,    -1,    44,    -1,   -15,    41,    41,    41,    41,
    44,    41,    41,    44,    44,    41,   281,   281,   281,   -23,
   290,   290,   -12,   -12,    -1,    44,    -1,    44,    41,    -1,
    -1,    -1,   304,    -1,    -1,   344,    -1,    -1,    41,    41,
    41,   292,   279,   292,   279,    41,    41,    -5,    43,    45,
    42,    47,    -5,    -1,    -1,   279,   279 };
yytabelem yydef[]={

    -2,    -2,     1,     3,     4,     0,     0,     0,     0,    12,
     0,    16,    17,    18,    19,    58,    58,    58,    58,    58,
    58,    58,    58,     0,    58,    53,     0,    42,     0,     0,
     0,     0,    81,    56,     5,     6,     0,     0,    11,     0,
     0,     0,   122,   123,   124,     0,     0,    85,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    95,
     0,    98,     0,     0,   105,   113,    43,    44,    45,    46,
    47,    48,    49,    50,    58,     0,     0,    52,    83,     0,
     0,    21,    22,    24,     0,     0,     0,    36,     0,    33,
     0,    82,     7,     8,    19,     0,    95,   124,     0,    13,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    14,     0,     0,    15,   130,
     0,    85,     0,   132,   133,   134,   135,   136,   145,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    96,
     0,    97,     0,     0,   107,    99,   121,   111,   114,   115,
   116,   117,   118,   119,   120,   102,   121,   112,   106,    57,
    60,    62,     0,     0,     0,     0,     0,    71,    72,    73,
    75,    77,    79,    80,    51,     0,     0,    84,    54,    20,
    23,    25,    26,     0,     0,    32,     0,     0,     9,    10,
    87,     0,    -2,     0,   125,   126,   127,   128,   129,    -2,
    -2,    -2,    -2,    -2,   143,   144,     0,     0,     0,     0,
    86,     0,   131,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,   110,   101,   104,   100,     0,   103,     0,
    59,    61,    63,    64,    65,    66,    67,     0,    70,    74,
    76,    78,    -2,    58,     0,     0,    31,    34,    35,     0,
     0,     0,     0,     0,     0,     0,   146,   147,   148,   149,
     0,   151,   152,     0,     0,   155,   108,   109,    68,    55,
     0,     0,     0,    94,    88,     0,    89,     0,    92,     0,
     0,     0,    69,     0,     0,    93,     0,     0,   150,   153,
   154,    41,    28,    41,    30,    90,    91,     0,    37,    38,
    39,    40,     0,     0,     0,    27,    29 };
typedef struct { char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"BOX",	1,
	"LINE",	2,
	"ARROW",	3,
	"CIRCLE",	4,
	"ELLIPSE",	5,
	"ARC",	6,
	"SPLINE",	7,
	"BLOCK",	8,
	"TEXT",	9,
	"TROFF",	10,
	"MOVE",	11,
	"PLOT",	12,
	"BLOCKEND",	13,
	"PLACE",	270,
	"PRINT",	271,
	"THRU",	272,
	"UNTIL",	273,
	"FOR",	274,
	"IF",	275,
	"COPY",	276,
	"THENSTR",	277,
	"ELSESTR",	278,
	"DOSTR",	279,
	"DEFNAME",	280,
	"PLACENAME",	281,
	"VARNAME",	282,
	"ATTR",	283,
	"TEXTATTR",	284,
	"LEFT",	285,
	"RIGHT",	286,
	"UP",	287,
	"DOWN",	288,
	"FROM",	289,
	"TO",	290,
	"AT",	291,
	"BY",	292,
	"WITH",	293,
	"HEAD",	294,
	"CW",	295,
	"CCW",	296,
	"THEN",	297,
	"HEIGHT",	298,
	"WIDTH",	299,
	"RADIUS",	300,
	"DIAMETER",	301,
	"LENGTH",	302,
	"SIZE",	303,
	"CORNER",	304,
	"HERE",	305,
	"LAST",	306,
	"NTH",	307,
	"SAME",	308,
	"BETWEEN",	309,
	"AND",	310,
	"EAST",	311,
	"WEST",	312,
	"NORTH",	313,
	"SOUTH",	314,
	"NE",	315,
	"NW",	316,
	"SE",	317,
	"SW",	318,
	"START",	319,
	"END",	320,
	"DOTX",	321,
	"DOTY",	322,
	"DOTHT",	323,
	"DOTWID",	324,
	"DOTRAD",	325,
	"NUMBER",	326,
	"LOG",	327,
	"EXP",	328,
	"SIN",	329,
	"COS",	330,
	"ATAN2",	331,
	"SQRT",	332,
	"RAND",	333,
	"MIN",	334,
	"MAX",	335,
	"INT",	336,
	"DIR",	337,
	"DOT",	338,
	"DASH",	339,
	"CHOP",	340,
	"ST",	341,
	"=",	61,
	"OROR",	342,
	"ANDAND",	343,
	"GT",	344,
	"LT",	345,
	"LE",	346,
	"GE",	347,
	"EQ",	348,
	"NEQ",	349,
	"+",	43,
	"-",	45,
	"*",	42,
	"/",	47,
	"%",	37,
	"UMINUS",	350,
	"NOT",	351,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"top : piclist",
	"top : /* empty */",
	"top : error",
	"piclist : picture",
	"piclist : piclist picture",
	"picture : prim ST",
	"picture : leftbrace piclist '}'",
	"picture : PLACENAME ':' picture",
	"picture : PLACENAME ':' ST picture",
	"picture : PLACENAME ':' position ST",
	"picture : asgn ST",
	"picture : DIR",
	"picture : PRINT expr ST",
	"picture : PRINT position ST",
	"picture : PRINT TEXT ST",
	"picture : copy",
	"picture : for",
	"picture : if",
	"picture : ST",
	"asgn : VARNAME '=' expr",
	"copy : COPY copylist",
	"copylist : copyattr",
	"copylist : copylist copyattr",
	"copyattr : TEXT",
	"copyattr : THRU DEFNAME",
	"copyattr : UNTIL TEXT",
	"for : FOR name FROM expr TO expr BY optop expr DOSTR",
	"for : FOR name FROM expr TO expr DOSTR",
	"for : FOR name '=' expr TO expr BY optop expr DOSTR",
	"for : FOR name '=' expr TO expr DOSTR",
	"if : IF if_expr THENSTR ELSESTR",
	"if : IF if_expr THENSTR",
	"if_expr : expr",
	"if_expr : TEXT EQ TEXT",
	"if_expr : TEXT NEQ TEXT",
	"name : VARNAME",
	"optop : '+'",
	"optop : '-'",
	"optop : '*'",
	"optop : '/'",
	"optop : /* empty */",
	"leftbrace : '{'",
	"prim : BOX attrlist",
	"prim : CIRCLE attrlist",
	"prim : ELLIPSE attrlist",
	"prim : ARC attrlist",
	"prim : LINE attrlist",
	"prim : ARROW attrlist",
	"prim : SPLINE attrlist",
	"prim : MOVE attrlist",
	"prim : PLOT expr attrlist",
	"prim : textlist attrlist",
	"prim : TROFF",
	"prim : lbracket piclist ']'",
	"prim : lbracket piclist ']' attrlist",
	"lbracket : '['",
	"attrlist : attrlist attr",
	"attrlist : /* empty */",
	"attr : ATTR expr",
	"attr : ATTR",
	"attr : DIR expr",
	"attr : DIR",
	"attr : FROM position",
	"attr : TO position",
	"attr : AT position",
	"attr : BY position",
	"attr : WITH CORNER",
	"attr : WITH '.' PLACENAME",
	"attr : WITH '.' PLACENAME CORNER",
	"attr : WITH position",
	"attr : SAME",
	"attr : TEXTATTR",
	"attr : HEAD",
	"attr : DOT expr",
	"attr : DOT",
	"attr : DASH expr",
	"attr : DASH",
	"attr : CHOP expr",
	"attr : CHOP",
	"attr : textlist",
	"textlist : TEXT",
	"textlist : TEXT TEXTATTR",
	"textlist : textlist TEXT",
	"textlist : textlist TEXT TEXTATTR",
	"position : place",
	"position : '(' position ')'",
	"position : expr ',' expr",
	"position : position '+' expr ',' expr",
	"position : position '-' expr ',' expr",
	"position : position '+' '(' expr ',' expr ')'",
	"position : position '-' '(' expr ',' expr ')'",
	"position : '(' place ',' place ')'",
	"position : expr LT position ',' position GT",
	"position : expr BETWEEN position AND position",
	"place : PLACENAME",
	"place : PLACENAME CORNER",
	"place : CORNER PLACENAME",
	"place : HERE",
	"place : last type",
	"place : last type CORNER",
	"place : CORNER last type",
	"place : NTH type",
	"place : NTH type CORNER",
	"place : CORNER NTH type",
	"place : blockname",
	"place : blockname CORNER",
	"place : CORNER blockname",
	"blockname : last BLOCK '.' PLACENAME",
	"blockname : NTH BLOCK '.' PLACENAME",
	"blockname : PLACENAME '.' PLACENAME",
	"last : last LAST",
	"last : NTH LAST",
	"last : LAST",
	"type : BOX",
	"type : CIRCLE",
	"type : ELLIPSE",
	"type : ARC",
	"type : LINE",
	"type : ARROW",
	"type : SPLINE",
	"type : BLOCK",
	"expr : NUMBER",
	"expr : VARNAME",
	"expr : asgn",
	"expr : expr '+' expr",
	"expr : expr '-' expr",
	"expr : expr '*' expr",
	"expr : expr '/' expr",
	"expr : expr '%' expr",
	"expr : '-' expr",
	"expr : '(' expr ')'",
	"expr : place DOTX",
	"expr : place DOTY",
	"expr : place DOTHT",
	"expr : place DOTWID",
	"expr : place DOTRAD",
	"expr : expr GT expr",
	"expr : expr LT expr",
	"expr : expr LE expr",
	"expr : expr GE expr",
	"expr : expr EQ expr",
	"expr : expr NEQ expr",
	"expr : expr ANDAND expr",
	"expr : expr OROR expr",
	"expr : NOT expr",
	"expr : LOG '(' expr ')'",
	"expr : EXP '(' expr ')'",
	"expr : SIN '(' expr ')'",
	"expr : COS '(' expr ')'",
	"expr : ATAN2 '(' expr ',' expr ')'",
	"expr : SQRT '(' expr ')'",
	"expr : RAND '(' expr ')'",
	"expr : MAX '(' expr ',' expr ')'",
	"expr : MIN '(' expr ',' expr ')'",
	"expr : INT '(' expr ')'",
};
#endif /* YYDEBUG */
/*	@(#)yaccpar	UniPlus V.2.1.2	(ATT 1.9)	*/

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	return(0)
#define YYABORT		return(1)
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-1000)

/*
** global variables used by the parser
*/
YYSTYPE yyv[ YYMAXDEPTH ];	/* value stack */
int yys[ YYMAXDEPTH ];		/* state stack */

YYSTYPE *yypv;			/* top of value stack */
int *yyps;			/* top of state stack */

int yystate;			/* current state */
int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */
int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */



/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
int
yyparse()
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

	goto yystack;
	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ YYMAXDEPTH ] )	/* room on stack? */
		{
			yyerror( "yacc stack overflow" );
			YYABORT;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			printf( "Received token " );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				printf( "Received token " );
				if ( yychar == 0 )
					printf( "end-of-file\n" );
				else if ( yychar < 0 )
					printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax error" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
				yynerrs++;
			skip_init:
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					printf( "Error recovery discards " );
					if ( yychar == 0 )
						printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 3:
# line 81 "picy.y"
{ yyerror("syntax error"); } break;
case 6:
# line 90 "picy.y"
{ codegen = 1; makeiattr(0, 0); } break;
case 7:
# line 91 "picy.y"
{ rightthing(yypvt[-2].o, '}'); yyval.o = yypvt[-1].o; } break;
case 8:
# line 92 "picy.y"
{ y.o=yypvt[-0].o; makevar(yypvt[-2].p,PLACENAME,y); yyval.o = yypvt[-0].o; } break;
case 9:
# line 93 "picy.y"
{ y.o=yypvt[-0].o; makevar(yypvt[-3].p,PLACENAME,y); yyval.o = yypvt[-0].o; } break;
case 10:
# line 94 "picy.y"
{ y.o=yypvt[-1].o; makevar(yypvt[-3].p,PLACENAME,y); yyval.o = yypvt[-1].o; } break;
case 11:
# line 95 "picy.y"
{ y.f = yypvt[-1].f; yyval.o = y.o; } break;
case 12:
# line 96 "picy.y"
{ setdir(yypvt[-0].i); } break;
case 13:
# line 97 "picy.y"
{ printexpr(yypvt[-1].f); } break;
case 14:
# line 98 "picy.y"
{ printpos(yypvt[-1].o); } break;
case 15:
# line 99 "picy.y"
{ fprintf(stderr, "%s\n", yypvt[-1].p); free(yypvt[-1].p); } break;
case 20:
# line 107 "picy.y"
{ yyval.f=y.f=yypvt[-0].f; makevar(yypvt[-2].p,VARNAME,y); checkscale(yypvt[-2].p); } break;
case 21:
# line 111 "picy.y"
{ copy(); } break;
case 24:
# line 118 "picy.y"
{ copyfile(yypvt[-0].p); } break;
case 25:
# line 119 "picy.y"
{ copydef(yypvt[-0].p); } break;
case 26:
# line 120 "picy.y"
{ copyuntil(yypvt[-0].p); } break;
case 27:
# line 125 "picy.y"
{ forloop(yypvt[-8].p, yypvt[-6].f, yypvt[-4].f, yypvt[-2].i, yypvt[-1].f, yypvt[-0].p); } break;
case 28:
# line 127 "picy.y"
{ forloop(yypvt[-5].p, yypvt[-3].f, yypvt[-1].f, '+', 1.0, yypvt[-0].p); } break;
case 29:
# line 129 "picy.y"
{ forloop(yypvt[-8].p, yypvt[-6].f, yypvt[-4].f, yypvt[-2].i, yypvt[-1].f, yypvt[-0].p); } break;
case 30:
# line 131 "picy.y"
{ forloop(yypvt[-5].p, yypvt[-3].f, yypvt[-1].f, '+', 1.0, yypvt[-0].p); } break;
case 31:
# line 135 "picy.y"
{ ifstat(yypvt[-2].f, yypvt[-1].p, yypvt[-0].p); } break;
case 32:
# line 136 "picy.y"
{ ifstat(yypvt[-1].f, yypvt[-0].p, (char *) 0); } break;
case 34:
# line 140 "picy.y"
{ yyval.f = strcmp(yypvt[-2].p,yypvt[-0].p) == 0; free(yypvt[-2].p); free(yypvt[-0].p); } break;
case 35:
# line 141 "picy.y"
{ yyval.f = strcmp(yypvt[-2].p,yypvt[-0].p) != 0; free(yypvt[-2].p); free(yypvt[-0].p); } break;
case 36:
# line 145 "picy.y"
{ y.f = 0; makevar(yypvt[-0].p, VARNAME, y); } break;
case 37:
# line 148 "picy.y"
{ yyval.i = '+'; } break;
case 38:
# line 149 "picy.y"
{ yyval.i = '-'; } break;
case 39:
# line 150 "picy.y"
{ yyval.i = '*'; } break;
case 40:
# line 151 "picy.y"
{ yyval.i = '/'; } break;
case 41:
# line 152 "picy.y"
{ yyval.i = ' '; } break;
case 42:
# line 157 "picy.y"
{ yyval.o = leftthing('{'); } break;
case 43:
# line 161 "picy.y"
{ yyval.o = boxgen(yypvt[-1].i); } break;
case 44:
# line 162 "picy.y"
{ yyval.o = circgen(yypvt[-1].i); } break;
case 45:
# line 163 "picy.y"
{ yyval.o = circgen(yypvt[-1].i); } break;
case 46:
# line 164 "picy.y"
{ yyval.o = arcgen(yypvt[-1].i); } break;
case 47:
# line 165 "picy.y"
{ yyval.o = linegen(yypvt[-1].i); } break;
case 48:
# line 166 "picy.y"
{ yyval.o = linegen(yypvt[-1].i); } break;
case 49:
# line 167 "picy.y"
{ yyval.o = linegen(yypvt[-1].i); } break;
case 50:
# line 168 "picy.y"
{ yyval.o = movegen(yypvt[-1].i); } break;
case 51:
# line 169 "picy.y"
{ yyval.o = plotgen(yypvt[-2].i, yypvt[-1].f); } break;
case 52:
# line 170 "picy.y"
{ yyval.o = textgen(yypvt[-1].i); } break;
case 53:
# line 171 "picy.y"
{ yyval.o = troffgen(yypvt[-0].i); } break;
case 54:
# line 172 "picy.y"
{ yyval.o=rightthing(yypvt[-2].o,']'); } break;
case 55:
# line 173 "picy.y"
{ yyval.o = blockgen(yypvt[-4].o, yypvt[-3].o, yypvt[-1].o); } break;
case 56:
# line 177 "picy.y"
{ yyval.o = leftthing('['); } break;
case 59:
# line 186 "picy.y"
{ makefattr(yypvt[-1].i, !DEFAULT, yypvt[-0].f); } break;
case 60:
# line 187 "picy.y"
{ makefattr(yypvt[-0].i, DEFAULT, 0.0); } break;
case 61:
# line 188 "picy.y"
{ makefattr(yypvt[-1].i, !DEFAULT, yypvt[-0].f); } break;
case 62:
# line 189 "picy.y"
{ makefattr(yypvt[-0].i, DEFAULT, 0.0); } break;
case 63:
# line 190 "picy.y"
{ makeoattr(yypvt[-1].i, yypvt[-0].o); } break;
case 64:
# line 191 "picy.y"
{ makeoattr(yypvt[-1].i, yypvt[-0].o); } break;
case 65:
# line 192 "picy.y"
{ makeoattr(yypvt[-1].i, yypvt[-0].o); } break;
case 66:
# line 193 "picy.y"
{ makeoattr(yypvt[-1].i, yypvt[-0].o); } break;
case 67:
# line 194 "picy.y"
{ makeiattr(WITH, yypvt[-0].i); } break;
case 68:
# line 195 "picy.y"
{ makeoattr(PLACE, getblock(getlast(1,BLOCK), yypvt[-0].p)); } break;
case 69:
# line 197 "picy.y"
{ makeoattr(PLACE, getpos(getblock(getlast(1,BLOCK), yypvt[-1].p), yypvt[-0].i)); } break;
case 70:
# line 198 "picy.y"
{ makeoattr(PLACE, yypvt[-0].o); } break;
case 71:
# line 199 "picy.y"
{ makeiattr(SAME, yypvt[-0].i); } break;
case 72:
# line 200 "picy.y"
{ maketattr(yypvt[-0].i, 0); } break;
case 73:
# line 201 "picy.y"
{ makeiattr(HEAD, yypvt[-0].i); } break;
case 74:
# line 202 "picy.y"
{ makefattr(DOT, !DEFAULT, yypvt[-0].f); } break;
case 75:
# line 203 "picy.y"
{ makefattr(DOT, DEFAULT, 0.0); } break;
case 76:
# line 204 "picy.y"
{ makefattr(DASH, !DEFAULT, yypvt[-0].f); } break;
case 77:
# line 205 "picy.y"
{ makefattr(DASH, DEFAULT, 0.0); } break;
case 78:
# line 206 "picy.y"
{ makefattr(CHOP, !DEFAULT, yypvt[-0].f); } break;
case 79:
# line 207 "picy.y"
{ makefattr(CHOP, DEFAULT, 0.0); } break;
case 81:
# line 212 "picy.y"
{ maketattr(CENTER, yypvt[-0].p); } break;
case 82:
# line 213 "picy.y"
{ maketattr(yypvt[-0].i, yypvt[-1].p); } break;
case 83:
# line 214 "picy.y"
{ maketattr(CENTER, yypvt[-0].p); } break;
case 84:
# line 215 "picy.y"
{ maketattr(yypvt[-0].i, yypvt[-1].p); } break;
case 86:
# line 220 "picy.y"
{ yyval.o = yypvt[-1].o; } break;
case 87:
# line 221 "picy.y"
{ yyval.o = makepos(yypvt[-2].f, yypvt[-0].f); } break;
case 88:
# line 222 "picy.y"
{ yyval.o = fixpos(yypvt[-4].o, yypvt[-2].f, yypvt[-0].f); } break;
case 89:
# line 223 "picy.y"
{ yyval.o = fixpos(yypvt[-4].o, -yypvt[-2].f, -yypvt[-0].f); } break;
case 90:
# line 224 "picy.y"
{ yyval.o = fixpos(yypvt[-6].o, yypvt[-3].f, yypvt[-1].f); } break;
case 91:
# line 225 "picy.y"
{ yyval.o = fixpos(yypvt[-6].o, -yypvt[-3].f, -yypvt[-1].f); } break;
case 92:
# line 226 "picy.y"
{ yyval.o = makepos(getcomp(yypvt[-3].o,DOTX), getcomp(yypvt[-1].o,DOTY)); } break;
case 93:
# line 227 "picy.y"
{ yyval.o = makebetween(yypvt[-5].f, yypvt[-3].o, yypvt[-1].o); } break;
case 94:
# line 228 "picy.y"
{ yyval.o = makebetween(yypvt[-4].f, yypvt[-2].o, yypvt[-0].o); } break;
case 95:
# line 232 "picy.y"
{ y = getvar(yypvt[-0].p); yyval.o = y.o; } break;
case 96:
# line 233 "picy.y"
{ y = getvar(yypvt[-1].p); yyval.o = getpos(y.o, yypvt[-0].i); } break;
case 97:
# line 234 "picy.y"
{ y = getvar(yypvt[-0].p); yyval.o = getpos(y.o, yypvt[-1].i); } break;
case 98:
# line 235 "picy.y"
{ yyval.o = gethere(yypvt[-0].i); } break;
case 99:
# line 236 "picy.y"
{ yyval.o = getlast(yypvt[-1].i, yypvt[-0].i); } break;
case 100:
# line 237 "picy.y"
{ yyval.o = getpos(getlast(yypvt[-2].i, yypvt[-1].i), yypvt[-0].i); } break;
case 101:
# line 238 "picy.y"
{ yyval.o = getpos(getlast(yypvt[-1].i, yypvt[-0].i), yypvt[-2].i); } break;
case 102:
# line 239 "picy.y"
{ yyval.o = getfirst(yypvt[-1].i, yypvt[-0].i); } break;
case 103:
# line 240 "picy.y"
{ yyval.o = getpos(getfirst(yypvt[-2].i, yypvt[-1].i), yypvt[-0].i); } break;
case 104:
# line 241 "picy.y"
{ yyval.o = getpos(getfirst(yypvt[-1].i, yypvt[-0].i), yypvt[-2].i); } break;
case 106:
# line 243 "picy.y"
{ yyval.o = getpos(yypvt[-1].o, yypvt[-0].i); } break;
case 107:
# line 244 "picy.y"
{ yyval.o = getpos(yypvt[-0].o, yypvt[-1].i); } break;
case 108:
# line 248 "picy.y"
{ yyval.o = getblock(getlast(yypvt[-3].i,yypvt[-2].i), yypvt[-0].p); } break;
case 109:
# line 249 "picy.y"
{ yyval.o = getblock(getfirst(yypvt[-3].i,yypvt[-2].i), yypvt[-0].p); } break;
case 110:
# line 250 "picy.y"
{ y = getvar(yypvt[-2].p); yyval.o = getblock(y.o, yypvt[-0].p); } break;
case 111:
# line 254 "picy.y"
{ yyval.i = yypvt[-1].i + 1; } break;
case 112:
# line 255 "picy.y"
{ yyval.i = yypvt[-1].i; } break;
case 113:
# line 256 "picy.y"
{ yyval.i = 1; } break;
case 123:
# line 272 "picy.y"
{ yyval.f = getfval(yypvt[-0].p); } break;
case 125:
# line 274 "picy.y"
{ yyval.f = yypvt[-2].f + yypvt[-0].f; } break;
case 126:
# line 275 "picy.y"
{ yyval.f = yypvt[-2].f - yypvt[-0].f; } break;
case 127:
# line 276 "picy.y"
{ yyval.f = yypvt[-2].f * yypvt[-0].f; } break;
case 128:
# line 277 "picy.y"
{ if (yypvt[-0].f == 0.0) {
					yyerror("division by 0"); yypvt[-0].f = 1; }
				  yyval.f = yypvt[-2].f / yypvt[-0].f; } break;
case 129:
# line 280 "picy.y"
{ if ((long)yypvt[-0].f == 0) {
					yyerror("mod does division by 0"); yypvt[-0].f = 1; }
				  yyval.f = (long)yypvt[-2].f % (long)yypvt[-0].f; } break;
case 130:
# line 283 "picy.y"
{ yyval.f = -yypvt[-0].f; } break;
case 131:
# line 284 "picy.y"
{ yyval.f = yypvt[-1].f; } break;
case 132:
# line 285 "picy.y"
{ yyval.f = getcomp(yypvt[-1].o, yypvt[-0].i); } break;
case 133:
# line 286 "picy.y"
{ yyval.f = getcomp(yypvt[-1].o, yypvt[-0].i); } break;
case 134:
# line 287 "picy.y"
{ yyval.f = getcomp(yypvt[-1].o, yypvt[-0].i); } break;
case 135:
# line 288 "picy.y"
{ yyval.f = getcomp(yypvt[-1].o, yypvt[-0].i); } break;
case 136:
# line 289 "picy.y"
{ yyval.f = getcomp(yypvt[-1].o, yypvt[-0].i); } break;
case 137:
# line 290 "picy.y"
{ yyval.f = yypvt[-2].f > yypvt[-0].f; } break;
case 138:
# line 291 "picy.y"
{ yyval.f = yypvt[-2].f < yypvt[-0].f; } break;
case 139:
# line 292 "picy.y"
{ yyval.f = yypvt[-2].f <= yypvt[-0].f; } break;
case 140:
# line 293 "picy.y"
{ yyval.f = yypvt[-2].f >= yypvt[-0].f; } break;
case 141:
# line 294 "picy.y"
{ yyval.f = yypvt[-2].f == yypvt[-0].f; } break;
case 142:
# line 295 "picy.y"
{ yyval.f = yypvt[-2].f != yypvt[-0].f; } break;
case 143:
# line 296 "picy.y"
{ yyval.f = yypvt[-2].f && yypvt[-0].f; } break;
case 144:
# line 297 "picy.y"
{ yyval.f = yypvt[-2].f || yypvt[-0].f; } break;
case 145:
# line 298 "picy.y"
{ yyval.f = !(yypvt[-0].f); } break;
case 146:
# line 299 "picy.y"
{ yyval.f = Log10(yypvt[-1].f); } break;
case 147:
# line 300 "picy.y"
{ yyval.f = Exp(yypvt[-1].f * log(10.0)); } break;
case 148:
# line 301 "picy.y"
{ yyval.f = sin(yypvt[-1].f); } break;
case 149:
# line 302 "picy.y"
{ yyval.f = cos(yypvt[-1].f); } break;
case 150:
# line 303 "picy.y"
{ yyval.f = atan2(yypvt[-3].f, yypvt[-1].f); } break;
case 151:
# line 304 "picy.y"
{ yyval.f = Sqrt(yypvt[-1].f); } break;
case 152:
# line 305 "picy.y"
{ yyval.f = rand() % (int) yypvt[-1].f + 1; } break;
case 153:
# line 306 "picy.y"
{ yyval.f = yypvt[-3].f >= yypvt[-1].f ? yypvt[-3].f : yypvt[-1].f; } break;
case 154:
# line 307 "picy.y"
{ yyval.f = yypvt[-3].f <= yypvt[-1].f ? yypvt[-3].f : yypvt[-1].f; } break;
case 155:
# line 308 "picy.y"
{ yyval.f = (long) yypvt[-1].f; } break;
	}
	goto yystack;		/* reset registers in driver code */
}
