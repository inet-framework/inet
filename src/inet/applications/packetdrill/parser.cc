/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 1



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ELLIPSIS = 258,
     UDP = 259,
     _HTONS_ = 260,
     _HTONL_ = 261,
     BACK_QUOTED = 262,
     SA_FAMILY = 263,
     SIN_PORT = 264,
     SIN_ADDR = 265,
     ACK = 266,
     WIN = 267,
     WSCALE = 268,
     MSS = 269,
     NOP = 270,
     TIMESTAMP = 271,
     ECR = 272,
     EOL = 273,
     TCPSACK = 274,
     VAL = 275,
     SACKOK = 276,
     OPTION = 277,
     IPV4_TYPE = 278,
     IPV6_TYPE = 279,
     INET_ADDR = 280,
     SPP_ASSOC_ID = 281,
     SPP_ADDRESS = 282,
     SPP_HBINTERVAL = 283,
     SPP_PATHMAXRXT = 284,
     SPP_PATHMTU = 285,
     SPP_FLAGS = 286,
     SPP_IPV6_FLOWLABEL_ = 287,
     SPP_DSCP_ = 288,
     SINFO_STREAM = 289,
     SINFO_SSN = 290,
     SINFO_FLAGS = 291,
     SINFO_PPID = 292,
     SINFO_CONTEXT = 293,
     SINFO_ASSOC_ID = 294,
     SINFO_TIMETOLIVE = 295,
     SINFO_TSN = 296,
     SINFO_CUMTSN = 297,
     SINFO_PR_VALUE = 298,
     CHUNK = 299,
     MYDATA = 300,
     MYINIT = 301,
     MYINIT_ACK = 302,
     MYHEARTBEAT = 303,
     MYHEARTBEAT_ACK = 304,
     MYABORT = 305,
     MYSHUTDOWN = 306,
     MYSHUTDOWN_ACK = 307,
     MYERROR = 308,
     MYCOOKIE_ECHO = 309,
     MYCOOKIE_ACK = 310,
     MYSHUTDOWN_COMPLETE = 311,
     PAD = 312,
     ERROR = 313,
     HEARTBEAT_INFORMATION = 314,
     CAUSE_INFO = 315,
     MYSACK = 316,
     STATE_COOKIE = 317,
     PARAMETER = 318,
     MYSCTP = 319,
     TYPE = 320,
     FLAGS = 321,
     LEN = 322,
     MYSUPPORTED_EXTENSIONS = 323,
     MYSUPPORTED_ADDRESS_TYPES = 324,
     TYPES = 325,
     CWR = 326,
     ECNE = 327,
     TAG = 328,
     A_RWND = 329,
     OS = 330,
     IS = 331,
     TSN = 332,
     MYSID = 333,
     SSN = 334,
     PPID = 335,
     CUM_TSN = 336,
     GAPS = 337,
     DUPS = 338,
     MID = 339,
     FSN = 340,
     SRTO_ASSOC_ID = 341,
     SRTO_INITIAL = 342,
     SRTO_MAX = 343,
     SRTO_MIN = 344,
     SINIT_NUM_OSTREAMS = 345,
     SINIT_MAX_INSTREAMS = 346,
     SINIT_MAX_ATTEMPTS = 347,
     SINIT_MAX_INIT_TIMEO = 348,
     MYSACK_DELAY = 349,
     SACK_FREQ = 350,
     ASSOC_VALUE = 351,
     ASSOC_ID = 352,
     SACK_ASSOC_ID = 353,
     RECONFIG = 354,
     OUTGOING_SSN_RESET = 355,
     REQ_SN = 356,
     RESP_SN = 357,
     LAST_TSN = 358,
     SIDS = 359,
     INCOMING_SSN_RESET = 360,
     RECONFIG_RESPONSE = 361,
     RESULT = 362,
     SENDER_NEXT_TSN = 363,
     RECEIVER_NEXT_TSN = 364,
     SSN_TSN_RESET = 365,
     ADD_INCOMING_STREAMS = 366,
     NUMBER_OF_NEW_STREAMS = 367,
     ADD_OUTGOING_STREAMS = 368,
     RECONFIG_REQUEST_GENERIC = 369,
     SRS_ASSOC_ID = 370,
     SRS_FLAGS = 371,
     SRS_NUMBER_STREAMS = 372,
     SRS_STREAM_LIST = 373,
     SSTAT_ASSOC_ID = 374,
     SSTAT_STATE = 375,
     SSTAT_RWND = 376,
     SSTAT_UNACKDATA = 377,
     SSTAT_PENDDATA = 378,
     SSTAT_INSTRMS = 379,
     SSTAT_OUTSTRMS = 380,
     SSTAT_FRAGMENTATION_POINT = 381,
     SSTAT_PRIMARY = 382,
     SASOC_ASOCMAXRXT = 383,
     SASOC_ASSOC_ID = 384,
     SASOC_NUMBER_PEER_DESTINATIONS = 385,
     SASOC_PEER_RWND = 386,
     SASOC_LOCAL_RWND = 387,
     SASOC_COOKIE_LIFE = 388,
     SAS_ASSOC_ID = 389,
     SAS_INSTRMS = 390,
     SAS_OUTSTRMS = 391,
     MYINVALID_STREAM_IDENTIFIER = 392,
     SID = 393,
     MYFLOAT = 394,
     INTEGER = 395,
     HEX_INTEGER = 396,
     MYWORD = 397,
     MYSTRING = 398
   };
#endif
/* Tokens.  */
#define ELLIPSIS 258
#define UDP 259
#define _HTONS_ 260
#define _HTONL_ 261
#define BACK_QUOTED 262
#define SA_FAMILY 263
#define SIN_PORT 264
#define SIN_ADDR 265
#define ACK 266
#define WIN 267
#define WSCALE 268
#define MSS 269
#define NOP 270
#define TIMESTAMP 271
#define ECR 272
#define EOL 273
#define TCPSACK 274
#define VAL 275
#define SACKOK 276
#define OPTION 277
#define IPV4_TYPE 278
#define IPV6_TYPE 279
#define INET_ADDR 280
#define SPP_ASSOC_ID 281
#define SPP_ADDRESS 282
#define SPP_HBINTERVAL 283
#define SPP_PATHMAXRXT 284
#define SPP_PATHMTU 285
#define SPP_FLAGS 286
#define SPP_IPV6_FLOWLABEL_ 287
#define SPP_DSCP_ 288
#define SINFO_STREAM 289
#define SINFO_SSN 290
#define SINFO_FLAGS 291
#define SINFO_PPID 292
#define SINFO_CONTEXT 293
#define SINFO_ASSOC_ID 294
#define SINFO_TIMETOLIVE 295
#define SINFO_TSN 296
#define SINFO_CUMTSN 297
#define SINFO_PR_VALUE 298
#define CHUNK 299
#define MYDATA 300
#define MYINIT 301
#define MYINIT_ACK 302
#define MYHEARTBEAT 303
#define MYHEARTBEAT_ACK 304
#define MYABORT 305
#define MYSHUTDOWN 306
#define MYSHUTDOWN_ACK 307
#define MYERROR 308
#define MYCOOKIE_ECHO 309
#define MYCOOKIE_ACK 310
#define MYSHUTDOWN_COMPLETE 311
#define PAD 312
#define ERROR 313
#define HEARTBEAT_INFORMATION 314
#define CAUSE_INFO 315
#define MYSACK 316
#define STATE_COOKIE 317
#define PARAMETER 318
#define MYSCTP 319
#define TYPE 320
#define FLAGS 321
#define LEN 322
#define MYSUPPORTED_EXTENSIONS 323
#define MYSUPPORTED_ADDRESS_TYPES 324
#define TYPES 325
#define CWR 326
#define ECNE 327
#define TAG 328
#define A_RWND 329
#define OS 330
#define IS 331
#define TSN 332
#define MYSID 333
#define SSN 334
#define PPID 335
#define CUM_TSN 336
#define GAPS 337
#define DUPS 338
#define MID 339
#define FSN 340
#define SRTO_ASSOC_ID 341
#define SRTO_INITIAL 342
#define SRTO_MAX 343
#define SRTO_MIN 344
#define SINIT_NUM_OSTREAMS 345
#define SINIT_MAX_INSTREAMS 346
#define SINIT_MAX_ATTEMPTS 347
#define SINIT_MAX_INIT_TIMEO 348
#define MYSACK_DELAY 349
#define SACK_FREQ 350
#define ASSOC_VALUE 351
#define ASSOC_ID 352
#define SACK_ASSOC_ID 353
#define RECONFIG 354
#define OUTGOING_SSN_RESET 355
#define REQ_SN 356
#define RESP_SN 357
#define LAST_TSN 358
#define SIDS 359
#define INCOMING_SSN_RESET 360
#define RECONFIG_RESPONSE 361
#define RESULT 362
#define SENDER_NEXT_TSN 363
#define RECEIVER_NEXT_TSN 364
#define SSN_TSN_RESET 365
#define ADD_INCOMING_STREAMS 366
#define NUMBER_OF_NEW_STREAMS 367
#define ADD_OUTGOING_STREAMS 368
#define RECONFIG_REQUEST_GENERIC 369
#define SRS_ASSOC_ID 370
#define SRS_FLAGS 371
#define SRS_NUMBER_STREAMS 372
#define SRS_STREAM_LIST 373
#define SSTAT_ASSOC_ID 374
#define SSTAT_STATE 375
#define SSTAT_RWND 376
#define SSTAT_UNACKDATA 377
#define SSTAT_PENDDATA 378
#define SSTAT_INSTRMS 379
#define SSTAT_OUTSTRMS 380
#define SSTAT_FRAGMENTATION_POINT 381
#define SSTAT_PRIMARY 382
#define SASOC_ASOCMAXRXT 383
#define SASOC_ASSOC_ID 384
#define SASOC_NUMBER_PEER_DESTINATIONS 385
#define SASOC_PEER_RWND 386
#define SASOC_LOCAL_RWND 387
#define SASOC_COOKIE_LIFE 388
#define SAS_ASSOC_ID 389
#define SAS_INSTRMS 390
#define SAS_OUTSTRMS 391
#define MYINVALID_STREAM_IDENTIFIER 392
#define SID 393
#define MYFLOAT 394
#define INTEGER 395
#define HEX_INTEGER 396
#define MYWORD 397
#define MYSTRING 398




/* Copy the first part of user declarations.  */
#line 1 "parser.y"

/*
 * Copyright 2013 Google Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
/*
 * Author: Author: ncardwell@google.com (Neal Cardwell)
 *
 * This is the parser for the packetdrill script language. It is
 * processed by the bison parser generator.
 *
 * For full documentation see: http://www.gnu.org/software/bison/manual/
 *
 * Here is a quick and dirty tutorial on bison:
 *
 * A bison parser specification is basically a BNF grammar for the
 * language you are parsing. Each rule specifies a nonterminal symbol
 * on the left-hand side and a sequence of terminal symbols (lexical
 * tokens) and or nonterminal symbols on the right-hand side that can
 * "reduce" to the symbol on the left hand side. When the parser sees
 * the sequence of symbols on the right where it "wants" to see a
 * nonterminal on the left, the rule fires, executing the semantic
 * action code in curly {} braces as it reduces the right hand side to
 * the left hand side.
 *
 * The semantic action code for a rule produces an output, which it
 * can reference using the $$ token. The set of possible types
 * returned in output expressions is given in the %union section of
 * the .y file. The specific type of the output for a terminal or
 * nonterminal symbol (corresponding to a field in the %union) is
 * given by the %type directive in the .y file. The action code can
 * access the outputs of the symbols on the right hand side by using
 * the notation $1 for the first symbol, $2 for the second symbol, and
 * so on.
 *
 * The lexer (generated by flex from lexer.l) feeds a stream of
 * terminal symbols up to this parser. Parser semantic actions can
 * access the lexer output for a terminal symbol with the same
 * notation they use for nonterminals.
 *
 */

/* The first part of the .y file consists of C code that bison copies
 * directly into the top of the .c file it generates.
 */

#include "inet/common/INETDefs.h"

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <arpa/inet.h>
#include <netinet/in.h>
#else
#include "winsock2.h"
#endif
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "PacketDrillUtils.h"
#include "PacketDrill.h"


/* This include of the bison-generated .h file must go last so that we
 * can first include all of the declarations on which it depends.
 */
#include "parser.h"

/* Change this YYDEBUG to 1 to get verbose debug output for parsing: */
#define YYDEBUG 0
#if YYDEBUG
extern int yydebug;
#endif

extern FILE *yyin;
extern int yylineno;
extern int yywrap(void);
extern char *yytext;
extern int yylex(void);
extern int yyparse(void);

/* The input to the parser: the path name of the script file to parse. */
static const char* current_script_path = NULL;

/* The starting line number of the input script statement that we're
 * currently parsing. This may be different than yylineno if bison had
 * to look ahead and lexically scan a token on the following line to
 * decide that the current statement is done.
 */
static int current_script_line = -1;

/*
 * We use this object to look up configuration info needed during
 * parsing.
 */
static PacketDrillConfig *in_config = NULL;

/* The output of the parser: an output script containing
 * 1) a linked list of options
 * 2) a linked list of events
 */
static PacketDrillScript *out_script = NULL;


/* The test invocation to pass back to parse_and_finalize_config(). */
struct invocation *invocation;

/* This standard callback is invoked by flex when it encounters
 * the end of a file. We return 1 to tell flex to return EOF.
 */
int yywrap(void)
{
    return 1;
}


/* The public entry point for the script parser. Parses the
 * text script file with the given path name and fills in the script
 * object with the parsed representation.
 */
int parse_script(PacketDrillConfig *config, PacketDrillScript *script, struct invocation *callback_invocation){
    /* This bison-generated parser is not multi-thread safe, so we
     * have a lock to prevent more than one thread using the
     * parser at the same time. This is useful in the wire server
     * context, where in general we may have more than one test
     * thread running at the same time.
     */

#if YYDEBUG
    yydebug = 1;
#endif

    /* Now parse the script from our buffer. */
    yyin = fopen(script->getScriptPath(), "r");
    if (!yyin)
        printf("fopen: parse error opening script buffer");
    current_script_path = config->getScriptPath();
    in_config = config;
    out_script = script;
    invocation = callback_invocation;

    /* We have to reset the line number here since the wire server
     * can do more than one yyparse().
     */
    yylineno = 1;
    int result = yyparse(); /* invoke bison-generated parser */
    current_script_path = NULL;
    if (fclose(yyin))
        printf("fclose: error closing script buffer");

    /* Unlock parser. */

    return result ? -1 : 0;
}

void parse_and_finalize_config(struct invocation *invocation)
{
    invocation->config->parseScriptOptions(invocation->script->getOptionList());
}

/* Bison emits code to call this method when there's a parse-time error.
 * We print the line number and the error message.
 */
static void yyerror(const char *message) {
    fprintf(stderr, "%s:%d: parse error at '%s': %s\n",
        current_script_path, yylineno, yytext, message);
}

static void semantic_error(const char* message)
{
    printf("%s\n", message);
    throw cTerminationException("Packetdrill error: Script error");
}

/* Create and initalize a new integer expression with the given
 * literal value and format string.
 */
static PacketDrillExpression *new_integer_expression(int64 num, const char *format) {
    PacketDrillExpression *expression = new PacketDrillExpression(EXPR_INTEGER);
    expression->setNum(num);
    expression->setFormat(format);
    return expression;
}


/* Create and initialize a new option. */
/*static struct option_list *new_option(char *name, char *value)
{
    return NULL;
}*/



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 219 "parser.y"
{
    int64 integer;
    double floating;
    char *string;
    char *reserved;
    int64 time_usecs;
    enum direction_t direction;
    uint16 port;
    int32 window;
    uint32 sequence_number;
    struct {
        int protocol;    /* IPPROTO_TCP or IPPROTO_UDP */
        uint32 start_sequence;
        uint16 payload_bytes;
    } tcp_sequence_info;
    PacketDrillEvent *event;
    PacketDrillPacket *packet;
    struct syscall_spec *syscall;
    struct command_spec *command;
    PacketDrillStruct *sack_block;
    PacketDrillStruct *cause_item;
    PacketDrillExpression *expression;
    cQueue *expression_list;
    PacketDrillTcpOption *tcp_option;
    PacketDrillSctpParameter *sctp_parameter;
    PacketDrillOption *option;
    cQueue *tcp_options;
    struct errno_spec *errno_info;
    cQueue *sctp_chunk_list;
    cQueue *sctp_parameter_list;
    cQueue *address_types_list;
    cQueue *sack_block_list;
    cQueue *stream_list;
    cQueue *cause_list;
    PacketDrillBytes *byte_list;
    uint8 byte;
    PacketDrillSctpChunk *sctp_chunk;
}
/* Line 193 of yacc.c.  */
#line 634 "parser.cc"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 659 "parser.cc"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
    YYLTYPE yyls;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  7
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1206

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  162
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  161
/* YYNRULES -- Number of rules.  */
#define YYNRULES  370
/* YYNRULES -- Number of states.  */
#define YYNSTATES  957

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   398

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     148,   149,   146,   145,   154,   158,   157,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   150,   151,
     155,   144,   156,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   152,     2,   153,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   160,   159,   161,   147,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,     7,     9,    11,    14,    18,    20,
      22,    24,    26,    28,    31,    34,    37,    39,    41,    45,
      51,    53,    55,    57,    59,    61,    63,    65,    67,    69,
      76,    82,    87,    89,    93,    95,    97,    99,   101,   103,
     105,   107,   109,   111,   113,   115,   117,   119,   121,   125,
     129,   133,   137,   141,   145,   151,   157,   159,   163,   164,
     166,   170,   172,   174,   176,   178,   180,   182,   184,   186,
     188,   190,   192,   194,   196,   198,   200,   202,   204,   206,
     208,   212,   216,   220,   224,   228,   232,   236,   240,   244,
     248,   252,   256,   260,   264,   268,   272,   276,   280,   284,
     288,   292,   296,   300,   304,   308,   312,   316,   320,   324,
     328,   332,   336,   342,   348,   352,   358,   364,   379,   395,
     411,   424,   431,   438,   443,   450,   455,   464,   469,   472,
     473,   476,   478,   482,   489,   496,   498,   504,   509,   513,
     517,   521,   525,   529,   533,   537,   541,   545,   549,   553,
     557,   561,   565,   569,   573,   575,   579,   580,   582,   591,
     606,   611,   622,   627,   634,   645,   652,   659,   665,   668,
     669,   672,   674,   678,   680,   682,   684,   686,   688,   690,
     692,   694,   696,   698,   703,   710,   717,   726,   727,   729,
     733,   735,   737,   739,   746,   755,   760,   771,   782,   784,
     786,   788,   790,   792,   795,   797,   804,   805,   808,   809,
     812,   813,   817,   821,   823,   827,   829,   831,   834,   837,
     839,   842,   848,   850,   853,   854,   856,   860,   864,   865,
     867,   871,   875,   879,   887,   888,   891,   893,   896,   900,
     902,   906,   908,   910,   912,   917,   922,   927,   929,   931,
     934,   936,   938,   940,   942,   944,   946,   948,   950,   952,
     954,   956,   958,   960,   962,   964,   968,   971,   975,   979,
     983,   987,   991,   995,   999,  1001,  1003,  1005,  1017,  1025,
    1029,  1033,  1037,  1041,  1045,  1049,  1053,  1057,  1061,  1065,
    1081,  1093,  1097,  1101,  1105,  1109,  1113,  1117,  1121,  1125,
    1135,  1155,  1159,  1163,  1167,  1171,  1175,  1179,  1183,  1187,
    1191,  1195,  1199,  1203,  1207,  1227,  1243,  1247,  1251,  1255,
    1259,  1263,  1267,  1271,  1275,  1279,  1283,  1287,  1291,  1295,
    1299,  1321,  1339,  1343,  1347,  1351,  1355,  1359,  1366,  1370,
    1374,  1378,  1382,  1386,  1390,  1394,  1398,  1402,  1424,  1442,
    1446,  1450,  1454,  1470,  1482,  1496,  1506,  1516,  1522,  1526,
    1530,  1534,  1538,  1548,  1554,  1555,  1558,  1559,  1561,  1565,
    1567
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     163,     0,    -1,   164,   169,    -1,    -1,   165,    -1,   166,
      -1,   165,   166,    -1,   167,   144,   168,    -1,    22,    -1,
     140,    -1,   142,    -1,   143,    -1,   170,    -1,   169,   170,
      -1,   171,   173,    -1,   145,   172,    -1,   172,    -1,   146,
      -1,   172,   147,   172,    -1,   145,   172,   147,   145,   172,
      -1,   139,    -1,   140,    -1,   175,    -1,   259,    -1,   174,
      -1,     7,    -1,   176,    -1,   177,    -1,   178,    -1,   244,
     246,   247,   248,   249,   250,    -1,   244,     4,   148,   140,
     149,    -1,   244,    64,   150,   179,    -1,   180,    -1,   179,
     151,   180,    -1,   202,    -1,   203,    -1,   204,    -1,   205,
      -1,   206,    -1,   207,    -1,   208,    -1,   209,    -1,   210,
      -1,   211,    -1,   212,    -1,   218,    -1,   234,    -1,   217,
      -1,    66,   144,     3,    -1,    66,   144,   141,    -1,    66,
     144,   140,    -1,    67,   144,     3,    -1,    67,   144,   140,
      -1,    20,   144,     3,    -1,    20,   144,   152,     3,   153,
      -1,    20,   144,   152,   184,   153,    -1,   186,    -1,   184,
     154,   186,    -1,    -1,   187,    -1,   185,   154,   187,    -1,
     141,    -1,   140,    -1,   141,    -1,   140,    -1,    45,    -1,
      46,    -1,    47,    -1,    61,    -1,    48,    -1,    49,    -1,
      50,    -1,    51,    -1,    52,    -1,    53,    -1,    54,    -1,
      55,    -1,    56,    -1,    57,    -1,    99,    -1,    66,   144,
       3,    -1,    66,   144,   141,    -1,    66,   144,   140,    -1,
      66,   144,   142,    -1,    66,   144,     3,    -1,    66,   144,
     141,    -1,    66,   144,   140,    -1,    66,   144,   142,    -1,
      66,   144,     3,    -1,    66,   144,   141,    -1,    66,   144,
     140,    -1,    66,   144,   142,    -1,    73,   144,     3,    -1,
      73,   144,   140,    -1,    74,   144,     3,    -1,    74,   144,
     140,    -1,    75,   144,     3,    -1,    75,   144,   140,    -1,
      76,   144,     3,    -1,    76,   144,   140,    -1,    77,   144,
       3,    -1,    77,   144,   140,    -1,    78,   144,     3,    -1,
      78,   144,   140,    -1,    79,   144,     3,    -1,    79,   144,
     140,    -1,    80,   144,     3,    -1,    80,   144,   140,    -1,
      80,   144,   141,    -1,    81,   144,     3,    -1,    81,   144,
     140,    -1,    82,   144,     3,    -1,    82,   144,   152,     3,
     153,    -1,    82,   144,   152,   254,   153,    -1,    83,   144,
       3,    -1,    83,   144,   152,     3,   153,    -1,    83,   144,
     152,   256,   153,    -1,    45,   152,   188,   154,   182,   154,
     195,   154,   196,   154,   197,   154,   198,   153,    -1,    46,
     152,   181,   154,   191,   154,   192,   154,   193,   154,   194,
     154,   195,   235,   153,    -1,    47,   152,   181,   154,   191,
     154,   192,   154,   193,   154,   194,   154,   195,   235,   153,
      -1,    61,   152,   181,   154,   199,   154,   192,   154,   200,
     154,   201,   153,    -1,    48,   152,   181,   154,   238,   153,
      -1,    49,   152,   181,   154,   238,   153,    -1,    50,   152,
     189,   153,    -1,    51,   152,   181,   154,   199,   153,    -1,
      52,   152,   181,   153,    -1,    54,   152,   181,   154,   182,
     154,   183,   153,    -1,    55,   152,   181,   153,    -1,   154,
       3,    -1,    -1,   154,   214,    -1,   216,    -1,   214,   154,
     216,    -1,   137,   152,    78,   144,   140,   153,    -1,   137,
     152,    78,   144,     3,   153,    -1,   215,    -1,    53,   152,
     181,   213,   153,    -1,    56,   152,   190,   153,    -1,   101,
     144,   140,    -1,   101,   144,     3,    -1,   102,   144,   140,
      -1,   102,   144,     3,    -1,   103,   144,   140,    -1,   103,
     144,     3,    -1,   107,   144,   140,    -1,   107,   144,     3,
      -1,   108,   144,   140,    -1,   108,   144,   141,    -1,   108,
     144,     3,    -1,   109,   144,   140,    -1,   109,   144,   141,
      -1,   109,   144,     3,    -1,   112,   144,   140,    -1,   112,
     144,     3,    -1,   227,    -1,   226,   154,   227,    -1,    -1,
     140,    -1,   100,   152,   219,   154,   220,   154,   221,   153,
      -1,   100,   152,   219,   154,   220,   154,   221,   154,   104,
     144,   152,   226,   153,   153,    -1,   105,   152,   219,   153,
      -1,   105,   152,   219,   154,   104,   144,   152,   226,   153,
     153,    -1,   110,   152,   219,   153,    -1,   106,   152,   220,
     154,   222,   153,    -1,   106,   152,   220,   154,   222,   154,
     223,   154,   224,   153,    -1,   113,   152,   219,   154,   225,
     153,    -1,   111,   152,   219,   154,   225,   153,    -1,    99,
     152,   181,   235,   153,    -1,   154,     3,    -1,    -1,   154,
     236,    -1,   237,    -1,   236,   154,   237,    -1,   238,    -1,
     243,    -1,   239,    -1,   242,    -1,   228,    -1,   229,    -1,
     230,    -1,   231,    -1,   232,    -1,   233,    -1,    59,   152,
       3,   153,    -1,    59,   152,   182,   154,   183,   153,    -1,
      68,   152,    70,   144,     3,   153,    -1,    68,   152,    70,
     144,   152,   185,   153,   153,    -1,    -1,   241,    -1,   240,
     154,   241,    -1,   140,    -1,    23,    -1,    24,    -1,    69,
     152,    70,   144,     3,   153,    -1,    69,   152,    70,   144,
     152,   240,   153,   153,    -1,    62,   152,     3,   153,    -1,
      62,   152,    67,   144,     3,   154,    20,   144,     3,   153,
      -1,    62,   152,    67,   144,   140,   154,    20,   144,     3,
     153,    -1,   245,    -1,   155,    -1,   156,    -1,   142,    -1,
     157,    -1,   142,   157,    -1,   158,    -1,   140,   150,   140,
     148,   140,   149,    -1,    -1,    11,   140,    -1,    -1,    12,
     140,    -1,    -1,   155,   251,   156,    -1,   155,     3,   156,
      -1,   252,    -1,   251,   154,   252,    -1,    15,    -1,    18,
      -1,    14,   140,    -1,    13,   140,    -1,    21,    -1,    19,
     253,    -1,    16,    20,   140,    17,   140,    -1,   258,    -1,
     253,   258,    -1,    -1,   255,    -1,   254,   154,   255,    -1,
     140,   150,   140,    -1,    -1,   257,    -1,   256,   154,   257,
      -1,   140,   150,   140,    -1,   140,   150,   140,    -1,   260,
     261,   262,   144,   264,   319,   320,    -1,    -1,     3,   172,
      -1,   142,    -1,   148,   149,    -1,   148,   263,   149,    -1,
     264,    -1,   263,   154,   264,    -1,     3,    -1,   265,    -1,
     266,    -1,     6,   148,   140,   149,    -1,     6,   148,   141,
     149,    -1,     5,   148,   140,   149,    -1,   142,    -1,   143,
      -1,   143,     3,    -1,   267,    -1,   285,    -1,   268,    -1,
     284,    -1,   315,    -1,   273,    -1,   318,    -1,   302,    -1,
     293,    -1,   279,    -1,   311,    -1,   313,    -1,   314,    -1,
     140,    -1,   141,    -1,   264,   159,   264,    -1,   152,   153,
      -1,   152,   263,   153,    -1,    87,   144,   140,    -1,    87,
     144,     3,    -1,    88,   144,   140,    -1,    88,   144,     3,
      -1,    89,   144,   140,    -1,    89,   144,     3,    -1,   140,
      -1,   142,    -1,     3,    -1,   160,    86,   144,   272,   154,
     269,   154,   270,   154,   271,   161,    -1,   160,   269,   154,
     270,   154,   271,   161,    -1,   128,   144,   140,    -1,   128,
     144,     3,    -1,   130,   144,   140,    -1,   130,   144,     3,
      -1,   131,   144,   140,    -1,   131,   144,     3,    -1,   132,
     144,   140,    -1,   132,   144,     3,    -1,   133,   144,   140,
      -1,   133,   144,     3,    -1,   160,   129,   144,   272,   154,
     274,   154,   275,   154,   276,   154,   277,   154,   278,   161,
      -1,   160,   274,   154,   275,   154,   276,   154,   277,   154,
     278,   161,    -1,    90,   144,   140,    -1,    90,   144,     3,
      -1,    91,   144,   140,    -1,    91,   144,     3,    -1,    92,
     144,   140,    -1,    92,   144,     3,    -1,    93,   144,   140,
      -1,    93,   144,     3,    -1,   160,   280,   154,   281,   154,
     282,   154,   283,   161,    -1,   160,     8,   144,   142,   154,
       9,   144,     5,   148,   140,   149,   154,    10,   144,    25,
     148,   143,   149,   161,    -1,    27,   144,     3,    -1,    27,
     144,   285,    -1,    28,   144,   140,    -1,    28,   144,     3,
      -1,    30,   144,   140,    -1,    30,   144,     3,    -1,    29,
     144,   140,    -1,    29,   144,     3,    -1,    31,   144,   264,
      -1,    32,   144,   140,    -1,    32,   144,     3,    -1,    33,
     144,   140,    -1,    33,   144,     3,    -1,   160,    26,   144,
     272,   154,   286,   154,   287,   154,   289,   154,   288,   154,
     290,   154,   291,   154,   292,   161,    -1,   160,   286,   154,
     287,   154,   289,   154,   288,   154,   290,   154,   291,   154,
     292,   161,    -1,   120,   144,   264,    -1,   121,   144,   140,
      -1,   121,   144,     3,    -1,   122,   144,   140,    -1,   122,
     144,     3,    -1,   123,   144,   140,    -1,   123,   144,     3,
      -1,   124,   144,   140,    -1,   124,   144,     3,    -1,   125,
     144,   140,    -1,   125,   144,     3,    -1,   126,   144,   140,
      -1,   126,   144,     3,    -1,   127,   144,     3,    -1,   160,
     119,   144,   272,   154,   294,   154,   295,   154,   296,   154,
     297,   154,   298,   154,   299,   154,   300,   154,   301,   161,
      -1,   160,   294,   154,   295,   154,   296,   154,   297,   154,
     298,   154,   299,   154,   300,   154,   301,   161,    -1,    34,
     144,   140,    -1,    34,   144,     3,    -1,    35,   144,   140,
      -1,    35,   144,     3,    -1,    36,   144,   264,    -1,    37,
     144,     6,   148,   140,   149,    -1,    37,   144,     3,    -1,
      38,   144,   140,    -1,    38,   144,     3,    -1,    40,   144,
     140,    -1,    40,   144,     3,    -1,    41,   144,   140,    -1,
      41,   144,     3,    -1,    42,   144,   140,    -1,    42,   144,
       3,    -1,   160,   303,   154,   304,   154,   305,   154,   306,
     154,   307,   154,   308,   154,   309,   154,   310,   154,    39,
     144,   272,   161,    -1,   160,   303,   154,   304,   154,   305,
     154,   306,   154,   307,   154,   308,   154,   309,   154,   310,
     161,    -1,   116,   144,   140,    -1,   116,   144,   142,    -1,
     116,   144,   267,    -1,   160,   115,   144,   272,   154,   312,
     154,   117,   144,   140,   154,   118,   144,   268,   161,    -1,
     160,   312,   154,   117,   144,   140,   154,   118,   144,   268,
     161,    -1,   160,   134,   144,   272,   154,   135,   144,   140,
     154,   136,   144,   140,   161,    -1,   160,   135,   144,   140,
     154,   136,   144,   140,   161,    -1,   160,    97,   144,   272,
     154,    96,   144,   264,   161,    -1,   160,    96,   144,   264,
     161,    -1,    94,   144,   140,    -1,    94,   144,     3,    -1,
      95,   144,   140,    -1,    95,   144,     3,    -1,   160,    98,
     144,   272,   154,   316,   154,   317,   161,    -1,   160,   316,
     154,   317,   161,    -1,    -1,   142,   321,    -1,    -1,   321,
      -1,   148,   322,   149,    -1,   142,    -1,   322,   142,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   362,   362,   368,   370,   377,   381,   388,   393,   397,
     398,   399,   404,   408,   415,   445,   451,   457,   462,   469,
     479,   485,   494,   501,   505,   512,   519,   522,   525,   531,
     557,   576,   596,   598,   604,   605,   606,   607,   608,   609,
     610,   611,   612,   613,   614,   615,   616,   617,   622,   623,
     629,   638,   639,   648,   649,   650,   654,   655,   660,   661,
     662,   667,   673,   682,   688,   694,   697,   700,   703,   706,
     709,   712,   715,   718,   721,   724,   727,   730,   733,   736,
     742,   743,   749,   755,   799,   800,   806,   812,   835,   836,
     842,   848,   871,   872,   881,   882,   891,   892,   901,   902,
     911,   912,   921,   922,   931,   932,   942,   943,   949,   958,
     959,   968,   969,   970,   975,   976,   977,   982,   991,   996,
    1001,  1006,  1012,  1018,  1023,  1028,  1033,  1049,  1054,  1055,
    1056,  1060,  1062,  1067,  1073,  1078,  1082,  1087,  1093,  1099,
    1103,  1109,  1113,  1119,  1123,  1129,  1133,  1139,  1145,  1149,
    1155,  1161,  1165,  1171,  1175,  1179,  1185,  1188,  1198,  1201,
    1207,  1210,  1216,  1222,  1225,  1231,  1237,  1249,  1255,  1256,
    1257,  1261,  1265,  1273,  1274,  1275,  1276,  1277,  1278,  1279,
    1280,  1281,  1282,  1287,  1290,  1306,  1309,  1314,  1316,  1319,
    1325,  1329,  1330,  1334,  1337,  1343,  1346,  1349,  1359,  1367,
    1371,  1378,  1381,  1384,  1388,  1394,  1414,  1417,  1426,  1429,
    1438,  1441,  1444,  1451,  1455,  1463,  1466,  1469,  1476,  1483,
    1486,  1490,  1507,  1511,  1517,  1518,  1522,  1528,  1540,  1541,
    1545,  1551,  1563,  1576,  1588,  1591,  1597,  1604,  1607,  1613,
    1617,  1624,  1627,  1629,  1632,  1638,  1644,  1650,  1654,  1659,
    1664,  1667,  1670,  1673,  1676,  1679,  1682,  1685,  1688,  1691,
    1694,  1697,  1700,  1708,  1714,  1720,  1731,  1735,  1742,  1748,
    1754,  1757,  1761,  1764,  1768,  1771,  1775,  1779,  1788,  1800,
    1806,  1810,  1816,  1820,  1826,  1830,  1836,  1840,  1846,  1850,
    1862,  1878,  1884,  1888,  1894,  1898,  1904,  1908,  1914,  1918,
    1931,  1945,  1946,  1950,  1956,  1960,  1966,  1970,  1976,  1980,
    1984,  1990,  1994,  2000,  2004,  2018,  2035,  2039,  2045,  2049,
    2055,  2059,  2065,  2069,  2075,  2079,  2085,  2089,  2095,  2099,
    2104,  2119,  2137,  2143,  2147,  2153,  2157,  2161,  2167,  2171,
    2177,  2181,  2187,  2191,  2197,  2201,  2207,  2212,  2227,  2244,
    2251,  2256,  2262,  2274,  2289,  2303,  2321,  2328,  2338,  2344,
    2349,  2355,  2358,  2366,  2377,  2380,  2388,  2391,  2397,  2403,
    2406
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ELLIPSIS", "UDP", "_HTONS_", "_HTONL_",
  "BACK_QUOTED", "SA_FAMILY", "SIN_PORT", "SIN_ADDR", "ACK", "WIN",
  "WSCALE", "MSS", "NOP", "TIMESTAMP", "ECR", "EOL", "TCPSACK", "VAL",
  "SACKOK", "OPTION", "IPV4_TYPE", "IPV6_TYPE", "INET_ADDR",
  "SPP_ASSOC_ID", "SPP_ADDRESS", "SPP_HBINTERVAL", "SPP_PATHMAXRXT",
  "SPP_PATHMTU", "SPP_FLAGS", "SPP_IPV6_FLOWLABEL_", "SPP_DSCP_",
  "SINFO_STREAM", "SINFO_SSN", "SINFO_FLAGS", "SINFO_PPID",
  "SINFO_CONTEXT", "SINFO_ASSOC_ID", "SINFO_TIMETOLIVE", "SINFO_TSN",
  "SINFO_CUMTSN", "SINFO_PR_VALUE", "CHUNK", "MYDATA", "MYINIT",
  "MYINIT_ACK", "MYHEARTBEAT", "MYHEARTBEAT_ACK", "MYABORT", "MYSHUTDOWN",
  "MYSHUTDOWN_ACK", "MYERROR", "MYCOOKIE_ECHO", "MYCOOKIE_ACK",
  "MYSHUTDOWN_COMPLETE", "PAD", "ERROR", "HEARTBEAT_INFORMATION",
  "CAUSE_INFO", "MYSACK", "STATE_COOKIE", "PARAMETER", "MYSCTP", "TYPE",
  "FLAGS", "LEN", "MYSUPPORTED_EXTENSIONS", "MYSUPPORTED_ADDRESS_TYPES",
  "TYPES", "CWR", "ECNE", "TAG", "A_RWND", "OS", "IS", "TSN", "MYSID",
  "SSN", "PPID", "CUM_TSN", "GAPS", "DUPS", "MID", "FSN", "SRTO_ASSOC_ID",
  "SRTO_INITIAL", "SRTO_MAX", "SRTO_MIN", "SINIT_NUM_OSTREAMS",
  "SINIT_MAX_INSTREAMS", "SINIT_MAX_ATTEMPTS", "SINIT_MAX_INIT_TIMEO",
  "MYSACK_DELAY", "SACK_FREQ", "ASSOC_VALUE", "ASSOC_ID", "SACK_ASSOC_ID",
  "RECONFIG", "OUTGOING_SSN_RESET", "REQ_SN", "RESP_SN", "LAST_TSN",
  "SIDS", "INCOMING_SSN_RESET", "RECONFIG_RESPONSE", "RESULT",
  "SENDER_NEXT_TSN", "RECEIVER_NEXT_TSN", "SSN_TSN_RESET",
  "ADD_INCOMING_STREAMS", "NUMBER_OF_NEW_STREAMS", "ADD_OUTGOING_STREAMS",
  "RECONFIG_REQUEST_GENERIC", "SRS_ASSOC_ID", "SRS_FLAGS",
  "SRS_NUMBER_STREAMS", "SRS_STREAM_LIST", "SSTAT_ASSOC_ID", "SSTAT_STATE",
  "SSTAT_RWND", "SSTAT_UNACKDATA", "SSTAT_PENDDATA", "SSTAT_INSTRMS",
  "SSTAT_OUTSTRMS", "SSTAT_FRAGMENTATION_POINT", "SSTAT_PRIMARY",
  "SASOC_ASOCMAXRXT", "SASOC_ASSOC_ID", "SASOC_NUMBER_PEER_DESTINATIONS",
  "SASOC_PEER_RWND", "SASOC_LOCAL_RWND", "SASOC_COOKIE_LIFE",
  "SAS_ASSOC_ID", "SAS_INSTRMS", "SAS_OUTSTRMS",
  "MYINVALID_STREAM_IDENTIFIER", "SID", "MYFLOAT", "INTEGER",
  "HEX_INTEGER", "MYWORD", "MYSTRING", "'='", "'+'", "'*'", "'~'", "'('",
  "')'", "':'", "';'", "'['", "']'", "','", "'<'", "'>'", "'.'", "'-'",
  "'|'", "'{'", "'}'", "$accept", "script", "opt_options", "options",
  "option", "option_flag", "option_value", "events", "event", "event_time",
  "time", "action", "command_spec", "packet_spec", "tcp_packet_spec",
  "udp_packet_spec", "sctp_packet_spec", "sctp_chunk_list", "sctp_chunk",
  "opt_flags", "opt_len", "opt_val", "byte_list", "chunk_types_list",
  "byte", "chunk_type", "opt_data_flags", "opt_abort_flags",
  "opt_shutdown_complete_flags", "opt_tag", "opt_a_rwnd", "opt_os",
  "opt_is", "opt_tsn", "opt_sid", "opt_ssn", "opt_ppid", "opt_cum_tsn",
  "opt_gaps", "opt_dups", "sctp_data_chunk_spec", "sctp_init_chunk_spec",
  "sctp_init_ack_chunk_spec", "sctp_sack_chunk_spec",
  "sctp_heartbeat_chunk_spec", "sctp_heartbeat_ack_chunk_spec",
  "sctp_abort_chunk_spec", "sctp_shutdown_chunk_spec",
  "sctp_shutdown_ack_chunk_spec", "sctp_cookie_echo_chunk_spec",
  "sctp_cookie_ack_chunk_spec", "opt_cause_list", "sctp_cause_list",
  "sctp_invalid_stream_identifier_cause_spec", "sctp_cause_spec",
  "sctp_error_chunk_spec", "sctp_shutdown_complete_chunk_spec",
  "opt_req_sn", "opt_resp_sn", "opt_last_tsn", "opt_result",
  "opt_sender_next_tsn", "opt_receiver_next_tsn",
  "opt_number_of_new_streams", "stream_list", "stream",
  "outgoing_ssn_reset_request", "incoming_ssn_reset_request",
  "ssn_tsn_reset_request", "reconfig_response",
  "add_outgoing_streams_request", "add_incoming_streams_request",
  "sctp_reconfig_chunk_spec", "opt_parameter_list", "sctp_parameter_list",
  "sctp_parameter", "sctp_heartbeat_information_parameter",
  "sctp_supported_extensions_parameter", "address_types_list",
  "address_type", "sctp_supported_address_types_parameter",
  "sctp_state_cookie_parameter", "packet_prefix", "direction", "flags",
  "seq", "opt_ack", "opt_window", "opt_tcp_options", "tcp_option_list",
  "tcp_option", "sack_block_list", "gap_list", "gap", "dup_list", "dup",
  "sack_block", "syscall_spec", "opt_end_time", "function_name",
  "function_arguments", "expression_list", "expression", "decimal_integer",
  "hex_integer", "binary_expression", "array", "srto_initial", "srto_max",
  "srto_min", "sctp_assoc_id", "sctp_rtoinfo", "sasoc_asocmaxrxt",
  "sasoc_number_peer_destinations", "sasoc_peer_rwnd", "sasoc_local_rwnd",
  "sasoc_cookie_life", "sctp_assocparams", "sinit_num_ostreams",
  "sinit_max_instreams", "sinit_max_attempts", "sinit_max_init_timeo",
  "sctp_initmsg", "sockaddr", "spp_address", "spp_hbinterval",
  "spp_pathmtu", "spp_pathmaxrxt", "spp_flags", "spp_ipv6_flowlabel",
  "spp_dscp", "sctp_paddrparams", "sstat_state", "sstat_rwnd",
  "sstat_unackdata", "sstat_penddata", "sstat_instrms", "sstat_outstrms",
  "sstat_fragmentation_point", "sstat_primary", "sctp_status",
  "sinfo_stream", "sinfo_ssn", "sinfo_flags", "sinfo_ppid",
  "sinfo_context", "sinfo_timetolive", "sinfo_tsn", "sinfo_cumtsn",
  "sctp_sndrcvinfo", "srs_flags", "sctp_reset_streams", "sctp_add_streams",
  "sctp_assoc_value", "sack_delay", "sack_freq", "sctp_sackinfo",
  "opt_errno", "opt_note", "note", "word_list", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,    61,    43,    42,   126,    40,    41,
      58,    59,    91,    93,    44,    60,    62,    46,    45,   124,
     123,   125
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   162,   163,   164,   164,   165,   165,   166,   167,   168,
     168,   168,   169,   169,   170,   171,   171,   171,   171,   171,
     172,   172,   173,   173,   173,   174,   175,   175,   175,   176,
     177,   178,   179,   179,   180,   180,   180,   180,   180,   180,
     180,   180,   180,   180,   180,   180,   180,   180,   181,   181,
     181,   182,   182,   183,   183,   183,   184,   184,   185,   185,
     185,   186,   186,   187,   187,   187,   187,   187,   187,   187,
     187,   187,   187,   187,   187,   187,   187,   187,   187,   187,
     188,   188,   188,   188,   189,   189,   189,   189,   190,   190,
     190,   190,   191,   191,   192,   192,   193,   193,   194,   194,
     195,   195,   196,   196,   197,   197,   198,   198,   198,   199,
     199,   200,   200,   200,   201,   201,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   213,
     213,   214,   214,   215,   215,   216,   217,   218,   219,   219,
     220,   220,   221,   221,   222,   222,   223,   223,   223,   224,
     224,   224,   225,   225,   226,   226,   227,   227,   228,   228,
     229,   229,   230,   231,   231,   232,   233,   234,   235,   235,
     235,   236,   236,   237,   237,   237,   237,   237,   237,   237,
     237,   237,   237,   238,   238,   239,   239,   240,   240,   240,
     241,   241,   241,   242,   242,   243,   243,   243,   244,   245,
     245,   246,   246,   246,   246,   247,   248,   248,   249,   249,
     250,   250,   250,   251,   251,   252,   252,   252,   252,   252,
     252,   252,   253,   253,   254,   254,   254,   255,   256,   256,
     256,   257,   258,   259,   260,   260,   261,   262,   262,   263,
     263,   264,   264,   264,   264,   264,   264,   264,   264,   264,
     264,   264,   264,   264,   264,   264,   264,   264,   264,   264,
     264,   264,   264,   265,   266,   267,   268,   268,   269,   269,
     270,   270,   271,   271,   272,   272,   272,   273,   273,   274,
     274,   275,   275,   276,   276,   277,   277,   278,   278,   279,
     279,   280,   280,   281,   281,   282,   282,   283,   283,   284,
     285,   286,   286,   287,   287,   288,   288,   289,   289,   290,
     291,   291,   292,   292,   293,   293,   294,   295,   295,   296,
     296,   297,   297,   298,   298,   299,   299,   300,   300,   301,
     302,   302,   303,   303,   304,   304,   305,   306,   306,   307,
     307,   308,   308,   309,   309,   310,   310,   311,   311,   312,
     312,   312,   313,   313,   314,   314,   315,   315,   316,   316,
     317,   317,   318,   318,   319,   319,   320,   320,   321,   322,
     322
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     0,     1,     1,     2,     3,     1,     1,
       1,     1,     1,     2,     2,     2,     1,     1,     3,     5,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     6,
       5,     4,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     3,
       3,     3,     3,     3,     5,     5,     1,     3,     0,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     5,     5,     3,     5,     5,    14,    15,    15,
      12,     6,     6,     4,     6,     4,     8,     4,     2,     0,
       2,     1,     3,     6,     6,     1,     5,     4,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     3,     0,     1,     8,    14,
       4,    10,     4,     6,    10,     6,     6,     5,     2,     0,
       2,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     4,     6,     6,     8,     0,     1,     3,
       1,     1,     1,     6,     8,     4,    10,    10,     1,     1,
       1,     1,     1,     2,     1,     6,     0,     2,     0,     2,
       0,     3,     3,     1,     3,     1,     1,     2,     2,     1,
       2,     5,     1,     2,     0,     1,     3,     3,     0,     1,
       3,     3,     3,     7,     0,     2,     1,     2,     3,     1,
       3,     1,     1,     1,     4,     4,     4,     1,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     2,     3,     3,     3,
       3,     3,     3,     3,     1,     1,     1,    11,     7,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,    15,
      11,     3,     3,     3,     3,     3,     3,     3,     3,     9,
      19,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,    19,    15,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
      21,    17,     3,     3,     3,     3,     3,     6,     3,     3,
       3,     3,     3,     3,     3,     3,     3,    21,    17,     3,
       3,     3,    15,    11,    13,     9,     9,     5,     3,     3,
       3,     3,     9,     5,     0,     2,     0,     1,     3,     1,
       2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       3,     8,     0,     0,     4,     5,     0,     1,    20,    21,
       0,    17,     2,    12,   234,    16,     6,     0,    15,    13,
       0,    25,   199,   200,    14,    24,    22,    26,    27,    28,
       0,   198,    23,     0,     0,     9,    10,    11,     7,     0,
     235,     0,     0,   201,   202,   204,     0,   236,     0,    18,
       0,     0,     0,   203,     0,   206,     0,     0,    19,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    31,    32,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    47,    45,    46,
       0,     0,   208,   241,     0,     0,   263,   264,   247,   248,
     237,     0,     0,     0,   239,   242,   243,   250,   252,   255,
     259,   253,   251,   258,   257,   260,   261,   262,   254,   256,
       0,    30,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   207,     0,
     210,     0,     0,   249,   266,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   238,     0,     0,   364,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   129,     0,
       0,     0,     0,     0,   169,    33,     0,   209,     0,    29,
       0,     0,     0,   267,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   240,   265,     0,   366,     0,     0,     0,     0,     0,
       0,     0,     0,   123,     0,   125,     0,     0,     0,   127,
       0,   137,     0,     0,     0,     0,     0,     0,     0,   215,
       0,   216,     0,   219,     0,   213,   246,   244,   245,     0,
     276,   274,   275,     0,   301,     0,   302,   333,   332,     0,
     269,   268,   292,   291,   359,   358,     0,     0,     0,     0,
     263,   247,     0,   250,     0,   316,   280,   279,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   365,   233,   367,
      80,    82,    81,    83,     0,     0,    48,    50,    49,     0,
       0,     0,     0,     0,     0,    84,    86,    85,    87,     0,
       0,   128,     0,   130,   135,   131,   136,     0,    88,    90,
      89,    91,     0,   168,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   177,   178,   179,   180,   181,   182,   170,
     171,   173,   175,   176,   174,   167,   205,   212,   218,   217,
       0,     0,   220,   222,     0,   211,     0,     0,     0,   357,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   363,   369,     0,     0,     0,     0,     0,     0,     0,
     121,   122,     0,   124,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     223,   214,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   271,   270,     0,     0,   282,   281,     0,     0,
     294,   293,     0,     0,   304,   303,     0,     0,   318,   317,
       0,     0,   335,   334,     0,     0,     0,   361,   360,   370,
     368,    51,    52,     0,     0,    92,    93,     0,     0,     0,
       0,     0,   109,   110,     0,   132,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   172,     0,   232,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   278,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   183,     0,     0,     0,   126,     0,   195,     0,
       0,     0,     0,     0,   160,     0,     0,     0,   162,     0,
       0,   221,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   273,   272,   284,   283,     0,     0,   296,   295,
       0,     0,   308,   307,     0,     0,   320,   319,     0,     0,
     336,     0,     0,     0,   100,   101,     0,     0,    94,    95,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,    58,     0,   187,   139,   138,     0,     0,
     141,   140,     0,     0,     0,     0,     0,     0,     0,     0,
     356,   362,     0,     0,     0,     0,   355,     0,     0,     0,
     299,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   184,   134,   133,     0,    62,    61,     0,
      56,     0,     0,     0,     0,   185,    65,    66,    67,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    68,
      79,    64,    63,     0,    59,   193,   191,   192,   190,     0,
     188,     0,     0,     0,   163,     0,     0,   166,   165,     0,
       0,     0,     0,     0,     0,     0,   286,   285,     0,     0,
     298,   297,   306,   305,     0,     0,   322,   321,     0,     0,
     338,     0,     0,     0,     0,   102,   103,     0,     0,    96,
      97,     0,     0,     0,    54,    55,     0,   111,   224,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   156,
     145,   144,     0,     0,   153,   152,     0,     0,   277,     0,
       0,     0,     0,     0,   290,     0,     0,     0,     0,     0,
       0,     0,   353,     0,     0,     0,     0,     0,    57,     0,
       0,     0,   225,     0,   120,     0,     0,   186,    60,   194,
     189,     0,   158,     0,   157,     0,   154,     0,     0,     0,
       0,     0,     0,     0,     0,   288,   287,   309,     0,     0,
     324,   323,     0,     0,     0,   340,   339,     0,     0,   104,
     105,     0,     0,    98,    99,   169,   169,   112,     0,   113,
       0,   114,   228,     0,     0,   143,   142,     0,     0,   156,
     148,   146,   147,     0,     0,     0,     0,     0,     0,     0,
     354,     0,     0,     0,     0,   337,     0,     0,     0,   117,
       0,     0,   227,   226,     0,     0,     0,   229,   196,   197,
       0,   161,   155,     0,   164,     0,     0,     0,     0,     0,
     311,   310,     0,     0,   326,   325,     0,     0,   342,   341,
       0,     0,   106,   107,   108,   118,   119,   115,     0,   116,
       0,   156,   151,   149,   150,     0,     0,   352,     0,   289,
       0,   315,     0,     0,     0,     0,   231,   230,     0,     0,
       0,     0,   313,   312,   328,   327,     0,     0,   344,   343,
       0,     0,     0,     0,     0,     0,     0,   331,     0,     0,
     348,   159,     0,     0,     0,   329,   346,   345,     0,   300,
     314,     0,     0,     0,     0,   330,   347
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,     4,     5,     6,    38,    12,    13,    14,
      15,    24,    25,    26,    27,    28,    29,    74,    75,   180,
     325,   497,   659,   683,   660,   684,   178,   185,   192,   330,
     488,   601,   732,   484,   597,   728,   822,   340,   609,   740,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,   247,   343,   344,   345,    87,    88,   504,   507,   748,
     623,   753,   844,   625,   795,   796,   363,   364,   365,   366,
     367,   368,    89,   254,   369,   370,   371,   372,   689,   690,
     373,   374,    30,    31,    46,    55,    92,   140,   199,   264,
     265,   382,   781,   782,   866,   867,   383,    32,    33,    48,
      57,   103,   104,   105,   106,   107,   108,   165,   302,   455,
     273,   109,   166,   304,   459,   577,   709,   110,   167,   306,
     463,   581,   111,   112,   168,   308,   585,   467,   715,   809,
     883,   113,   169,   310,   471,   589,   719,   813,   887,   927,
     114,   170,   312,   475,   592,   723,   818,   891,   931,   115,
     171,   116,   117,   118,   172,   315,   119,   234,   318,   317,
     413
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -646
static const yytype_int16 yypact[] =
{
      36,  -646,   130,   126,    36,  -646,   -80,  -646,  -646,  -646,
     129,  -646,   126,  -646,    32,   -93,  -646,   -74,   -71,  -646,
     129,  -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,
      10,  -646,  -646,    -6,   129,  -646,  -646,  -646,  -646,    -5,
    -646,    -4,   133,    73,  -646,  -646,    11,  -646,    45,  -646,
     129,    94,   277,  -646,   152,   225,     5,   160,  -646,   168,
     187,   190,   195,   197,   209,   214,   219,   220,   223,   226,
     227,   228,   229,   234,   169,  -646,  -646,  -646,  -646,  -646,
    -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,
     124,   237,   375,  -646,   242,   243,  -646,  -646,  -646,   389,
    -646,    18,   254,   -89,   236,  -646,  -646,  -646,  -646,  -646,
    -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,
      39,  -646,   327,   330,   330,   330,   330,   331,   330,   330,
     330,   330,   330,   336,   330,   330,   277,   255,  -646,   266,
     263,   268,   -90,  -646,  -646,   125,   265,   280,   283,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   295,   296,
     297,   298,   299,   300,   301,   256,   284,   302,   303,   304,
     305,   306,   307,  -646,    39,    39,  -112,   308,   309,   310,
     311,   312,   313,   314,   318,   316,   317,   319,   320,   321,
     323,   326,   324,   325,   328,  -646,   315,  -646,   398,  -646,
     329,   332,   334,  -646,   322,    50,    12,    33,    50,    37,
      49,    54,    39,    50,    50,    50,    43,    50,    39,    74,
      50,    50,   333,   358,   350,   356,   420,   363,   414,   368,
     355,   236,   236,   338,   338,    22,   384,    15,   380,   380,
     428,   428,    56,  -646,   408,  -646,    13,   335,   384,  -646,
      64,  -646,   408,   208,   337,   342,   339,   352,   353,  -646,
     474,  -646,   357,  -646,  -113,  -646,  -646,  -646,  -646,   344,
    -646,  -646,  -646,   345,  -646,   488,  -646,  -646,  -646,   346,
    -646,  -646,  -646,  -646,  -646,  -646,   -88,   347,   348,   349,
     351,   354,   236,   359,   360,   236,  -646,  -646,   361,   362,
     364,   365,   366,   367,   369,   373,   370,   377,   371,   378,
     372,   383,   374,   385,   386,   343,   390,  -646,  -646,  -646,
    -646,  -646,  -646,  -646,   387,   379,  -646,  -646,  -646,   391,
     382,   388,   392,   381,   393,  -646,  -646,  -646,  -646,   394,
     395,  -646,   397,   396,  -646,  -646,  -646,   399,  -646,  -646,
    -646,  -646,   400,  -646,   403,   404,   405,   406,   409,   410,
     411,   412,   413,  -646,  -646,  -646,  -646,  -646,  -646,   415,
    -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,
     401,   402,   357,  -646,   407,  -646,   497,   480,   423,  -646,
     416,   425,   421,   419,   417,   424,   430,    75,   451,    76,
     429,    77,   455,    78,   514,    79,   445,    80,   515,   431,
      81,  -646,  -646,  -120,    82,   491,    83,   496,   496,    58,
    -646,  -646,    84,  -646,   494,   436,   554,   496,    59,   505,
     506,   476,   476,   477,   476,   476,   476,   294,   561,   440,
    -646,  -646,   437,   432,   433,   438,   434,   435,   439,   441,
     446,   447,  -646,  -646,   448,   422,  -646,  -646,   450,   442,
    -646,  -646,   453,   444,  -646,  -646,   456,   449,  -646,  -646,
     457,   452,  -646,  -646,   458,   454,   459,  -646,  -646,  -646,
    -646,  -646,  -646,   460,   461,  -646,  -646,   463,   462,   464,
     466,   467,  -646,  -646,   465,  -646,   468,   469,   470,   472,
     473,   479,   482,   483,   475,   132,   484,   478,   481,   485,
     486,  -646,   471,  -646,   579,   420,   358,    39,   355,   493,
     363,   350,   490,   495,    85,  -646,    86,   499,    87,   492,
      88,   569,    89,   510,    39,   568,   502,    91,   536,    92,
     562,   562,  -646,   554,    93,     1,  -646,   556,  -646,    95,
      14,    17,    97,   477,  -646,   532,    98,   534,  -646,   530,
     530,  -646,   500,   489,   498,   123,   501,   503,   504,   507,
     509,   508,  -646,  -646,  -646,  -646,   511,   512,  -646,  -646,
     513,   516,  -646,  -646,   520,   517,  -646,  -646,   521,   518,
     236,   523,   519,   524,  -646,  -646,   526,   522,  -646,  -646,
     531,   525,   527,   529,   533,   535,  -646,    60,   539,   537,
     538,   540,   542,   244,   543,     9,  -646,  -646,   544,   541,
    -646,  -646,   545,   153,   546,   547,   548,   553,   514,   451,
    -646,  -646,   557,   445,   429,   551,  -646,    99,   566,   100,
    -646,   101,   613,   102,   550,    31,   607,   528,   103,   570,
     104,   574,   574,  -646,  -646,  -646,   549,  -646,  -646,   156,
    -646,    23,   571,   631,   633,  -646,  -646,  -646,  -646,  -646,
    -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,
    -646,  -646,  -646,   158,  -646,  -646,  -646,  -646,  -646,   162,
    -646,   575,   552,   105,  -646,   576,   106,  -646,  -646,   558,
     555,   559,   560,   563,   564,   567,  -646,  -646,   572,   565,
    -646,  -646,  -646,  -646,   577,   573,  -646,  -646,   578,   580,
    -646,   581,   584,   582,   585,  -646,  -646,   586,   583,  -646,
    -646,   587,   588,   589,  -646,  -646,   134,  -646,   107,   591,
     592,   594,   595,   596,   244,   597,     9,   603,   181,   593,
    -646,  -646,   604,   598,  -646,  -646,   599,   569,  -646,   590,
     510,   499,   600,   108,  -646,    39,   624,   109,   616,   611,
     110,   619,  -646,   111,   623,   112,   491,   491,  -646,   601,
     605,   183,  -646,    24,  -646,   657,   702,  -646,  -646,  -646,
    -646,   113,  -646,   602,  -646,   192,  -646,    67,   606,   700,
     608,   612,   609,   610,   614,  -646,  -646,   236,   615,   617,
    -646,  -646,   621,   618,   620,  -646,  -646,   622,   625,  -646,
    -646,   626,   627,  -646,  -646,   328,   328,  -646,   628,  -646,
     634,  -646,   114,   629,   630,  -646,  -646,   632,   635,   593,
    -646,  -646,  -646,   637,   636,   640,   613,   528,   550,   566,
    -646,   115,   679,   116,   641,  -646,   117,   672,    69,  -646,
     638,   639,  -646,  -646,   642,   643,   201,  -646,  -646,  -646,
     644,  -646,  -646,    72,  -646,   694,   645,   646,   647,   648,
    -646,  -646,   650,   649,  -646,  -646,   653,   651,  -646,  -646,
     654,   652,  -646,  -646,  -646,  -646,  -646,  -646,   660,  -646,
     662,   593,  -646,  -646,  -646,   655,   624,  -646,   616,  -646,
     118,  -646,   119,   658,   120,   681,  -646,  -646,   204,   661,
     659,   663,  -646,  -646,  -646,  -646,   664,   665,  -646,  -646,
     667,  -123,   666,   669,   679,   641,   721,  -646,   121,   686,
    -646,  -646,   668,   670,   671,  -646,  -646,  -646,   676,  -646,
    -646,   658,    50,   673,   674,  -646,  -646
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -646,  -646,  -646,  -646,   728,  -646,  -646,  -646,   745,  -646,
     253,  -646,  -646,  -646,  -646,  -646,  -646,  -646,   678,     3,
    -245,   215,  -646,  -646,    25,    16,  -646,  -646,  -646,   656,
    -399,   232,   135,  -417,  -646,  -646,  -646,   675,  -646,  -646,
    -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,
    -646,  -646,  -646,  -646,   418,  -646,  -646,  -293,   224,  -646,
    -646,  -646,  -646,   218,  -115,   -49,  -646,  -646,  -646,  -646,
    -646,  -646,  -646,  -461,  -646,   426,   127,  -646,  -646,    66,
    -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,
     443,  -646,  -646,   -15,  -646,   -84,   487,  -646,  -646,  -646,
    -646,   720,  -119,  -646,  -646,   677,  -645,   680,   340,   193,
    -208,  -646,   682,   341,   189,    63,   -21,  -646,  -646,  -646,
    -646,  -646,  -646,   683,   684,   376,    90,   202,   -14,   -73,
     -98,  -646,   685,   427,   205,   122,   -11,   -69,   -95,  -110,
    -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,  -646,
     687,  -646,  -646,  -646,   689,   688,  -646,  -646,  -646,   690,
    -646
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -352
static const yytype_int16 yytable[] =
{
     279,   176,   724,   347,   606,   287,   288,   289,    93,   294,
      94,    95,   298,   299,    41,   274,   341,   612,   326,   489,
     614,    93,   479,    94,    95,   320,   737,   831,   498,   480,
     233,   939,   686,   687,   720,    20,   277,   721,   940,    21,
     280,   384,    93,   385,    94,    95,    93,   175,    94,    95,
     201,   202,   282,   270,    34,   231,   232,   284,     1,   335,
     173,   490,   499,   656,    17,   174,    35,   348,    36,    37,
     840,   175,   892,   389,    42,   902,    39,   296,   452,   456,
     460,   464,   468,   472,   477,   481,   485,   492,   572,   574,
     578,   582,   586,   286,   594,   598,   604,   292,   610,   295,
     616,   620,   706,   710,   712,   716,   725,   729,   750,   754,
     779,   805,   810,   815,   819,   823,   835,   864,   880,   884,
     888,   922,   924,   928,   946,   324,   500,   181,   182,   183,
       7,   186,   187,   188,   189,   190,    47,   193,   194,   505,
      50,   508,   509,   510,    51,    96,    97,    98,    99,   688,
     342,    54,    43,   607,   100,   327,   328,   101,    96,    97,
      98,    99,   321,   322,   323,   102,   613,    44,    45,   615,
     101,   144,   275,   278,   491,   738,   832,   281,   102,    96,
      97,    98,    99,   290,    97,   291,    99,    22,    23,   283,
     271,   101,   272,    56,   285,   101,   336,   337,   338,   102,
     657,   658,   877,   102,   349,   350,   351,   841,   842,   893,
     894,   353,   903,   904,   297,   453,   457,   461,   465,   469,
     473,   478,   482,   486,   493,   573,   575,   579,   583,   587,
      53,   595,   599,   605,    59,   611,    91,   617,   621,   707,
     711,   713,   717,   726,   730,   751,   755,   780,   806,   811,
     816,   820,   824,   836,   865,   881,   885,   889,   923,   925,
     929,   947,   146,    18,   137,     8,     9,   332,     8,     9,
     354,    10,    11,    40,   657,   658,   355,   356,   203,   174,
     147,   148,   175,    52,   630,   554,   555,    49,   149,   666,
     667,   668,   669,   670,   671,   672,   673,   674,   675,   676,
     677,   678,    90,    58,   120,   679,   694,   695,   357,   735,
     736,   743,   744,   358,   359,   745,   746,   121,   360,   361,
     136,   362,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,   792,   793,   829,   830,    72,   122,
     150,   151,   123,   680,   152,   838,   839,   124,   153,   125,
     154,   155,   156,   332,   899,   900,   354,   932,   839,   825,
     826,   126,   355,   356,   860,   861,   127,   333,   334,   157,
     158,   128,   129,   159,   160,   130,    73,   138,   131,   132,
     133,   134,   161,   162,   681,   682,   135,   139,   163,   164,
     141,   142,   143,   177,   357,   175,   179,   184,   565,   358,
     359,   256,   191,   196,   360,   361,   197,   362,   200,   204,
     223,   257,   258,   259,   260,   590,   261,   262,   198,   263,
     257,   258,   259,   260,   205,   261,   262,   206,   263,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   224,   216,
     217,   218,   219,   220,   221,   222,   301,   305,   307,   311,
     314,   324,   235,   329,   237,   255,   225,   226,   227,   228,
     229,   230,   242,   236,   269,   238,   239,   240,   241,   243,
     250,   244,   245,   300,   246,   248,   249,   251,   266,   252,
     303,   267,   253,   268,   309,   313,   316,   332,   346,   339,
     375,   376,   378,   379,   380,   377,   146,   381,   386,   387,
     388,   390,   391,   392,   411,  -349,   442,   148,  -350,   397,
     151,   399,   445,  -351,   393,   394,   395,   401,   396,   153,
     398,   403,   405,   400,   402,   404,   406,   407,   408,   409,
     410,   414,   412,   415,   420,   416,   417,   158,   422,   160,
     454,   438,   418,   466,   419,   161,   421,   462,   423,   424,
     425,   474,   439,   426,   427,   428,   429,   430,   431,   450,
     458,   432,   433,   434,   435,   436,   451,   470,   483,   437,
     487,   476,   494,   342,   496,   501,   502,   503,   512,   506,
     513,   514,   517,   525,   562,   580,   515,   516,   518,   519,
     522,   523,   524,   520,   526,   521,   527,   528,   529,   584,
     530,   532,   534,   531,   537,   591,   533,   539,   535,   544,
     567,   561,   545,   536,   596,   538,   540,   549,   541,   542,
     593,   543,   546,   550,   547,   548,   551,   552,   556,   553,
     570,   576,   557,   588,   558,   571,   619,   600,   608,   559,
     560,   622,   624,   628,   714,   722,   807,   632,   627,   727,
     731,   741,   629,   742,   739,   637,   808,   639,   633,   817,
     833,   634,   631,   635,   641,   643,   638,   645,   647,   636,
     648,   642,   644,   646,   718,   650,   649,   640,   747,   651,
     101,   652,   653,   661,   752,   692,   654,   705,   655,   693,
     696,   662,   663,   699,   664,   665,   685,   702,   691,   708,
     697,   698,   734,   821,   749,   834,   837,   756,   801,   757,
     845,   762,   882,   890,   759,   843,   763,   760,   761,   905,
     758,   765,   767,   930,   945,   948,   764,   766,   770,   769,
     773,   775,    16,   794,   768,   783,   771,   774,   785,   786,
     804,   812,   776,   777,   954,   784,   772,   791,   797,   787,
     789,   814,   798,   799,   827,   828,   847,    19,   603,   851,
     788,   778,   846,   848,   849,   853,   856,   886,   862,   855,
     858,   852,   854,   602,   780,   850,   870,   618,   626,   857,
     859,   873,   868,   869,   875,   926,   918,   733,   871,   874,
     872,   895,   896,   898,   910,   897,   901,   912,   914,   906,
     916,   908,   865,   919,   933,   913,   915,   907,   936,   909,
     911,   938,   790,   934,   195,   863,   917,   935,   942,   941,
     952,   145,   701,   704,   803,   951,   937,   441,   879,   949,
     700,   950,   876,   920,   955,   956,   943,   878,   703,   921,
     944,   953,     0,   495,     0,     0,     0,   800,     0,     0,
       0,     0,     0,     0,     0,     0,   564,     0,     0,     0,
       0,     0,   569,   511,     0,     0,     0,     0,     0,   440,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   802,     0,     0,     0,     0,     0,     0,   276,
       0,   563,     0,   293,     0,   331,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   319,     0,     0,   352,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   568,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   444,     0,
       0,   443,     0,     0,     0,     0,   449,     0,   448,   447,
     446,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   566
};

static const yytype_int16 yycheck[] =
{
     208,   120,   647,   248,     3,   213,   214,   215,     3,   217,
       5,     6,   220,   221,     4,     3,     3,     3,     3,   418,
       3,     3,   142,     5,     6,     3,     3,     3,   427,   149,
     142,   154,    23,    24,     3,     3,     3,     6,   161,     7,
       3,   154,     3,   156,     5,     6,     3,   159,     5,     6,
     140,   141,     3,     3,   147,   174,   175,     3,    22,     3,
     149,     3,     3,     3,   144,   154,   140,     3,   142,   143,
       3,   159,     3,   161,    64,     3,   147,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,   212,     3,     3,     3,   216,     3,   218,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,    67,    67,   124,   125,   126,
       0,   128,   129,   130,   131,   132,   142,   134,   135,   432,
     145,   434,   435,   436,   148,   140,   141,   142,   143,   140,
     137,   140,   142,   152,   149,   140,   141,   152,   140,   141,
     142,   143,   140,   141,   142,   160,   152,   157,   158,   152,
     152,   153,   160,   140,   419,   152,   152,   140,   160,   140,
     141,   142,   143,   140,   141,   142,   143,   155,   156,   140,
     140,   152,   142,   148,   140,   152,   140,   141,   142,   160,
     140,   141,   847,   160,   140,   141,   142,   140,   141,   140,
     141,     3,   140,   141,   140,   140,   140,   140,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   140,   140,   140,
     157,   140,   140,   140,   140,   140,    11,   140,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   140,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   140,   140,   140,
     140,   140,     8,    10,   140,   139,   140,    59,   139,   140,
      62,   145,   146,    20,   140,   141,    68,    69,   153,   154,
      26,    27,   159,   150,   161,   153,   154,    34,    34,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,   150,    50,   144,    61,   153,   154,   100,   153,
     154,   153,   154,   105,   106,   153,   154,   149,   110,   111,
     151,   113,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,   153,   154,   153,   154,    61,   152,
      86,    87,   152,    99,    90,   153,   154,   152,    94,   152,
      96,    97,    98,    59,   153,   154,    62,   153,   154,   776,
     777,   152,    68,    69,   825,   826,   152,   240,   241,   115,
     116,   152,   152,   119,   120,   152,    99,   140,   152,   152,
     152,   152,   128,   129,   140,   141,   152,    12,   134,   135,
     148,   148,     3,    66,   100,   159,    66,    66,   517,   105,
     106,     3,    66,   148,   110,   111,   140,   113,   140,   144,
     154,    13,    14,    15,    16,   534,    18,    19,   155,    21,
      13,    14,    15,    16,   144,    18,    19,   144,    21,   144,
     144,   144,   144,   144,   144,   144,   144,   144,   154,   144,
     144,   144,   144,   144,   144,   144,    88,    91,    28,    35,
      95,    67,   144,    73,   144,   140,   154,   154,   154,   154,
     154,   154,   144,   154,   142,   154,   154,   154,   154,   153,
     144,   154,   153,   140,   154,   154,   153,   153,   149,   154,
     130,   149,   154,   149,   121,   117,   148,    59,   153,    81,
     153,   149,   140,   140,    20,   156,     8,   140,   154,   154,
     154,   154,   154,   154,   161,   154,     9,    27,   154,   144,
      87,   144,    96,   154,   154,   154,   154,   144,   154,    94,
     154,   144,   144,   154,   154,   154,   154,   144,   154,   144,
     144,   144,   142,   154,   153,   144,   154,   116,   144,   120,
      89,   140,   154,    29,   152,   128,   153,    92,   153,   152,
     154,    36,   150,   154,   154,   152,   152,   152,   152,   135,
     131,   152,   152,   152,   152,   152,   136,   122,    77,   154,
      74,   140,    78,   137,    20,    70,    70,   101,    17,   102,
     140,   144,   144,   161,     5,    93,   154,   154,   154,   154,
     144,   144,   144,   154,   144,   154,   154,   144,   154,    30,
     144,   144,   144,   154,   144,    37,   154,   144,   154,   144,
     117,   140,   144,   154,    78,   154,   154,   144,   154,   153,
     118,   154,   153,   144,   154,   153,   144,   144,   144,   154,
     140,   132,   154,   123,   153,   140,   104,    75,    82,   154,
     154,   107,   112,   154,    31,    38,   765,   144,   148,    79,
      76,    20,   154,    20,    83,   144,    32,   144,   154,    40,
       3,   154,   161,   154,   144,   144,   154,   144,   144,   161,
     144,   154,   154,   154,   124,   144,   154,   161,   103,   154,
     152,   154,   153,   144,   108,   144,   153,   136,   153,   144,
     144,   154,   154,   140,   154,   153,   153,   140,   154,   133,
     153,   153,   153,    80,   152,     3,   104,   149,   118,   154,
      10,   144,    33,    41,   154,   109,   144,   154,   154,    25,
     161,   144,   144,    42,     3,    39,   161,   154,   144,   148,
     144,   144,     4,   140,   154,   144,   154,   154,   144,   144,
     140,   125,   154,   154,   952,   153,   161,   144,   144,   153,
     153,   140,   154,   154,   153,   150,   144,    12,   543,   144,
     744,   736,   154,   154,   154,   144,   144,   126,   140,   149,
     144,   154,   154,   541,   140,   161,   144,   553,   560,   154,
     153,   144,   153,   153,   144,   127,   901,   652,   153,   153,
     839,   153,   153,   150,   144,   153,   152,   144,   144,   154,
     140,   154,   140,   148,   143,   154,   154,   161,   144,   161,
     161,   144,   746,   154,   136,   830,   900,   154,   149,   153,
     144,   101,   629,   634,   761,   154,   161,   384,   849,   161,
     628,   161,   846,   906,   161,   161,   934,   848,   633,   908,
     935,   951,    -1,   425,    -1,    -1,    -1,   757,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   516,    -1,    -1,    -1,
      -1,    -1,   521,   437,    -1,    -1,    -1,    -1,    -1,   382,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   760,    -1,    -1,    -1,    -1,    -1,    -1,   206,
      -1,   515,    -1,   216,    -1,   239,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   234,    -1,    -1,   252,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   520,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   388,    -1,
      -1,   387,    -1,    -1,    -1,    -1,   394,    -1,   393,   392,
     391,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   518
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,    22,   163,   164,   165,   166,   167,     0,   139,   140,
     145,   146,   169,   170,   171,   172,   166,   144,   172,   170,
       3,     7,   155,   156,   173,   174,   175,   176,   177,   178,
     244,   245,   259,   260,   147,   140,   142,   143,   168,   147,
     172,     4,    64,   142,   157,   158,   246,   142,   261,   172,
     145,   148,   150,   157,   140,   247,   148,   262,   172,   140,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    61,    99,   179,   180,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   217,   218,   234,
     150,    11,   248,     3,     5,     6,   140,   141,   142,   143,
     149,   152,   160,   263,   264,   265,   266,   267,   268,   273,
     279,   284,   285,   293,   302,   311,   313,   314,   315,   318,
     144,   149,   152,   152,   152,   152,   152,   152,   152,   152,
     152,   152,   152,   152,   152,   152,   151,   140,   140,    12,
     249,   148,   148,     3,   153,   263,     8,    26,    27,    34,
      86,    87,    90,    94,    96,    97,    98,   115,   116,   119,
     120,   128,   129,   134,   135,   269,   274,   280,   286,   294,
     303,   312,   316,   149,   154,   159,   264,    66,   188,    66,
     181,   181,   181,   181,    66,   189,   181,   181,   181,   181,
     181,    66,   190,   181,   181,   180,   148,   140,   155,   250,
     140,   140,   141,   153,   144,   144,   144,   144,   144,   144,
     144,   144,   144,   144,   144,   144,   144,   144,   144,   144,
     144,   144,   144,   154,   154,   154,   154,   154,   154,   154,
     154,   264,   264,   142,   319,   144,   154,   144,   154,   154,
     154,   154,   144,   153,   154,   153,   154,   213,   154,   153,
     144,   153,   154,   154,   235,   140,     3,    13,    14,    15,
      16,    18,    19,    21,   251,   252,   149,   149,   149,   142,
       3,   140,   142,   272,     3,   160,   285,     3,   140,   272,
       3,   140,     3,   140,     3,   140,   264,   272,   272,   272,
     140,   142,   264,   267,   272,   264,     3,   140,   272,   272,
     140,    88,   270,   130,   275,    91,   281,    28,   287,   121,
     295,    35,   304,   117,    95,   317,   148,   321,   320,   321,
       3,   140,   141,   142,    67,   182,     3,   140,   141,    73,
     191,   191,    59,   238,   238,     3,   140,   141,   142,    81,
     199,     3,   137,   214,   215,   216,   153,   182,     3,   140,
     141,   142,   199,     3,    62,    68,    69,   100,   105,   106,
     110,   111,   113,   228,   229,   230,   231,   232,   233,   236,
     237,   238,   239,   242,   243,   153,   149,   156,   140,   140,
      20,   140,   253,   258,   154,   156,   154,   154,   154,   161,
     154,   154,   154,   154,   154,   154,   154,   144,   154,   144,
     154,   144,   154,   144,   154,   144,   154,   144,   154,   144,
     144,   161,   142,   322,   144,   154,   144,   154,   154,   152,
     153,   153,   144,   153,   152,   154,   154,   154,   152,   152,
     152,   152,   152,   152,   152,   152,   152,   154,   140,   150,
     258,   252,     9,   286,   269,    96,   316,   312,   294,   274,
     135,   136,     3,   140,    89,   271,     3,   140,   131,   276,
       3,   140,    92,   282,     3,   140,    29,   289,     3,   140,
     122,   296,     3,   140,    36,   305,   140,     3,   140,   142,
     149,     3,   140,    77,   195,     3,   140,    74,   192,   192,
       3,   182,     3,   140,    78,   216,    20,   183,   192,     3,
      67,    70,    70,   101,   219,   219,   102,   220,   219,   219,
     219,   237,    17,   140,   144,   154,   154,   144,   154,   154,
     154,   154,   144,   144,   144,   161,   144,   154,   144,   154,
     144,   154,   144,   154,   144,   154,   154,   144,   154,   144,
     154,   154,   153,   154,   144,   144,   153,   154,   153,   144,
     144,   144,   144,   154,   153,   154,   144,   154,   153,   154,
     154,   140,     5,   287,   270,   264,   317,   117,   295,   275,
     140,   140,     3,   140,     3,   140,   132,   277,     3,   140,
      93,   283,     3,   140,    30,   288,     3,   140,   123,   297,
     264,    37,   306,   118,     3,   140,    78,   196,     3,   140,
      75,   193,   193,   183,     3,   140,     3,   152,    82,   200,
       3,   140,     3,   152,     3,   152,     3,   140,   220,   104,
       3,   140,   107,   222,   112,   225,   225,   148,   154,   154,
     161,   161,   144,   154,   154,   154,   161,   144,   154,   144,
     161,   144,   154,   144,   154,   144,   154,   144,   144,   154,
     144,   154,   154,   153,   153,   153,     3,   140,   141,   184,
     186,   144,   154,   154,   154,   153,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    61,
      99,   140,   141,   185,   187,   153,    23,    24,   140,   240,
     241,   154,   144,   144,   153,   154,   144,   153,   153,   140,
     289,   271,   140,   296,   276,   136,     3,   140,   133,   278,
       3,   140,     3,   140,    31,   290,     3,   140,   124,   298,
       3,     6,    38,   307,   268,     3,   140,    79,   197,     3,
     140,    76,   194,   194,   153,   153,   154,     3,   152,    83,
     201,    20,    20,   153,   154,   153,   154,   103,   221,   152,
       3,   140,   108,   223,     3,   140,   149,   154,   161,   154,
     154,   154,   144,   144,   161,   144,   154,   144,   154,   148,
     144,   154,   161,   144,   154,   144,   154,   154,   186,     3,
     140,   254,   255,   144,   153,   144,   144,   153,   187,   153,
     241,   144,   153,   154,   140,   226,   227,   144,   154,   154,
     288,   118,   297,   277,   140,     3,   140,   264,    32,   291,
       3,   140,   125,   299,   140,     3,   140,    40,   308,     3,
     140,    80,   198,     3,   140,   195,   195,   153,   150,   153,
     154,     3,   152,     3,     3,     3,   140,   104,   153,   154,
       3,   140,   141,   109,   224,    10,   154,   144,   154,   154,
     161,   144,   154,   144,   154,   149,   144,   154,   144,   153,
     235,   235,   140,   255,     3,   140,   256,   257,   153,   153,
     144,   153,   227,   144,   153,   144,   290,   268,   298,   278,
       3,   140,    33,   292,     3,   140,   126,   300,     3,   140,
      41,   309,     3,   140,   141,   153,   153,   153,   150,   153,
     154,   152,     3,   140,   141,    25,   154,   161,   154,   161,
     144,   161,   144,   154,   144,   154,   140,   257,   226,   148,
     291,   299,     3,   140,     3,   140,   127,   301,     3,   140,
      42,   310,   153,   143,   154,   154,   144,   161,   144,   154,
     161,   153,   149,   292,   300,     3,     3,   140,    39,   161,
     161,   154,   144,   301,   272,   161,   161
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;
/* Location data for the look-ahead symbol.  */
YYLTYPE yylloc;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;

  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[2];

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;
#if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 0;
#endif

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
	YYSTACK_RELOCATE (yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 362 "parser.y"
    {
    (yyval.string) = NULL;    /* The parser output is in out_script */
;}
    break;

  case 3:
#line 368 "parser.y"
    { (yyval.option) = NULL;
    parse_and_finalize_config(invocation);;}
    break;

  case 4:
#line 370 "parser.y"
    {
    (yyval.option) = (yyvsp[(1) - (1)].option);
    parse_and_finalize_config(invocation);
;}
    break;

  case 5:
#line 377 "parser.y"
    {
    out_script->addOption((yyvsp[(1) - (1)].option));
    (yyval.option) = (yyvsp[(1) - (1)].option);    /* return the tail so we can append to it */
;}
    break;

  case 6:
#line 381 "parser.y"
    {
    out_script->addOption((yyvsp[(2) - (2)].option));
    (yyval.option) = (yyvsp[(2) - (2)].option);    /* return the tail so we can append to it */
;}
    break;

  case 7:
#line 388 "parser.y"
    {
    (yyval.option) = new PacketDrillOption((yyvsp[(1) - (3)].string), (yyvsp[(3) - (3)].string));
;}
    break;

  case 8:
#line 393 "parser.y"
    { (yyval.string) = (yyvsp[(1) - (1)].reserved); ;}
    break;

  case 9:
#line 397 "parser.y"
    { (yyval.string) = strdup(yytext); ;}
    break;

  case 10:
#line 398 "parser.y"
    { (yyval.string) = (yyvsp[(1) - (1)].string); ;}
    break;

  case 11:
#line 399 "parser.y"
    { (yyval.string) = (yyvsp[(1) - (1)].string); ;}
    break;

  case 12:
#line 404 "parser.y"
    {
    out_script->addEvent((yyvsp[(1) - (1)].event));    /* save pointer to event list as output of parser */
    (yyval.event) = (yyvsp[(1) - (1)].event);    /* return the tail so that we can append to it */
;}
    break;

  case 13:
#line 408 "parser.y"
    {
    out_script->addEvent((yyvsp[(2) - (2)].event));
    (yyval.event) = (yyvsp[(2) - (2)].event);    /* return the tail so that we can append to it */
;}
    break;

  case 14:
#line 415 "parser.y"
    {
    (yyval.event) = (yyvsp[(2) - (2)].event);
    (yyval.event)->setLineNumber((yyvsp[(1) - (2)].event)->getLineNumber());    /* use timestamp's line */
    (yyval.event)->setEventTime((yyvsp[(1) - (2)].event)->getEventTime());
    (yyval.event)->setEventTimeEnd((yyvsp[(1) - (2)].event)->getEventTimeEnd());
    (yyval.event)->setTimeType((yyvsp[(1) - (2)].event)->getTimeType());
    (yyvsp[(1) - (2)].event)->getLineNumber(),
    (yyvsp[(1) - (2)].event)->getEventTime().dbl(),
    (yyvsp[(1) - (2)].event)->getEventTimeEnd().dbl(),
    (yyvsp[(1) - (2)].event)->getTimeType();
    if ((yyval.event)->getEventTimeEnd() != NO_TIME_RANGE) {
        if ((yyval.event)->getEventTimeEnd() < (yyval.event)->getEventTime())
            semantic_error("time range is backwards");
    }
    if ((yyval.event)->getTimeType() == ANY_TIME &&  ((yyval.event)->getType() != PACKET_EVENT ||
        ((yyval.event)->getPacket())->getDirection() != DIRECTION_OUTBOUND)) {
        yylineno = (yyval.event)->getLineNumber();
        semantic_error("event time <star> can only be used with outbound packets");
    } else if (((yyval.event)->getTimeType() == ABSOLUTE_RANGE_TIME ||
        (yyval.event)->getTimeType() == RELATIVE_RANGE_TIME) &&
        ((yyval.event)->getType() != PACKET_EVENT ||
        ((yyval.event)->getPacket())->getDirection() != DIRECTION_OUTBOUND)) {
        yylineno = (yyval.event)->getLineNumber();
        semantic_error("event time range can only be used with outbound packets");
    }
    delete((yyvsp[(1) - (2)].event));
;}
    break;

  case 15:
#line 445 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(INVALID_EVENT);
    (yyval.event)->setLineNumber((yylsp[(2) - (2)]).first_line);
    (yyval.event)->setEventTime((yyvsp[(2) - (2)].time_usecs));
    (yyval.event)->setTimeType(RELATIVE_TIME);
;}
    break;

  case 16:
#line 451 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(INVALID_EVENT);
    (yyval.event)->setLineNumber((yylsp[(1) - (1)]).first_line);
    (yyval.event)->setEventTime((yyvsp[(1) - (1)].time_usecs));
    (yyval.event)->setTimeType(ABSOLUTE_TIME);
;}
    break;

  case 17:
#line 457 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(INVALID_EVENT);
    (yyval.event)->setLineNumber((yylsp[(1) - (1)]).first_line);
    (yyval.event)->setTimeType(ANY_TIME);
;}
    break;

  case 18:
#line 462 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(INVALID_EVENT);
    (yyval.event)->setLineNumber((yylsp[(1) - (3)]).first_line);
    (yyval.event)->setTimeType(ABSOLUTE_RANGE_TIME);
    (yyval.event)->setEventTime((yyvsp[(1) - (3)].time_usecs));
    (yyval.event)->setEventTimeEnd((yyvsp[(3) - (3)].time_usecs));
;}
    break;

  case 19:
#line 469 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(INVALID_EVENT);
    (yyval.event)->setLineNumber((yylsp[(1) - (5)]).first_line);
    (yyval.event)->setTimeType(RELATIVE_RANGE_TIME);
    (yyval.event)->setEventTime((yyvsp[(2) - (5)].time_usecs));
    (yyval.event)->setEventTimeEnd((yyvsp[(5) - (5)].time_usecs));
;}
    break;

  case 20:
#line 479 "parser.y"
    {
    if ((yyvsp[(1) - (1)].floating) < 0) {
        semantic_error("negative time");
    }
    (yyval.time_usecs) = (int64)((yyvsp[(1) - (1)].floating) * 1.0e6); /* convert float secs to s64 microseconds */
;}
    break;

  case 21:
#line 485 "parser.y"
    {
    if ((yyvsp[(1) - (1)].integer) < 0) {
        semantic_error("negative time");
    }
    (yyval.time_usecs) = (int64)((yyvsp[(1) - (1)].integer) * 1000000); /* convert int secs to s64 microseconds */
;}
    break;

  case 22:
#line 494 "parser.y"
    {
    if ((yyvsp[(1) - (1)].packet)) {
        (yyval.event) = new PacketDrillEvent(PACKET_EVENT);  (yyval.event)->setPacket((yyvsp[(1) - (1)].packet));
    } else {
        (yyval.event) = NULL;
    }
;}
    break;

  case 23:
#line 501 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(SYSCALL_EVENT);
    (yyval.event)->setSyscall((yyvsp[(1) - (1)].syscall));
;}
    break;

  case 24:
#line 505 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(COMMAND_EVENT);
    (yyval.event)->setCommand((yyvsp[(1) - (1)].command));
;}
    break;

  case 25:
#line 512 "parser.y"
    {
    (yyval.command) = (struct command_spec *)calloc(1, sizeof(struct command_spec));
    (yyval.command)->command_line = (yyvsp[(1) - (1)].reserved);
;}
    break;

  case 26:
#line 519 "parser.y"
    {
    (yyval.packet) = (yyvsp[(1) - (1)].packet);
;}
    break;

  case 27:
#line 522 "parser.y"
    {
    (yyval.packet) = (yyvsp[(1) - (1)].packet);
;}
    break;

  case 28:
#line 525 "parser.y"
    {
    (yyval.packet) = (yyvsp[(1) - (1)].packet);
;}
    break;

  case 29:
#line 531 "parser.y"
    {
    char *error = NULL;
    PacketDrillPacket *outer = (yyvsp[(1) - (6)].packet), *inner = NULL;
    enum direction_t direction = outer->getDirection();

    if (((yyvsp[(6) - (6)].tcp_options) == NULL) && (direction != DIRECTION_OUTBOUND)) {
        yylineno = (yylsp[(6) - (6)]).first_line;
        printf("<...> for TCP options can only be used with outbound packets");
    }
    cPacket* pkt = PacketDrill::buildTCPPacket(in_config->getWireProtocol(), direction,
                                               (yyvsp[(2) - (6)].string),
                                               (yyvsp[(3) - (6)].tcp_sequence_info).start_sequence, (yyvsp[(3) - (6)].tcp_sequence_info).payload_bytes,
                                               (yyvsp[(4) - (6)].sequence_number), (yyvsp[(5) - (6)].window), (yyvsp[(6) - (6)].tcp_options), &error);

    free((yyvsp[(2) - (6)].string));

    inner = new PacketDrillPacket();
    inner->setInetPacket(pkt);

    inner->setDirection(direction);

    (yyval.packet) = inner;
;}
    break;

  case 30:
#line 557 "parser.y"
    {
    char *error = NULL;
    PacketDrillPacket *outer = (yyvsp[(1) - (5)].packet), *inner = NULL;

    enum direction_t direction = outer->getDirection();
    cPacket* pkt = PacketDrill::buildUDPPacket(in_config->getWireProtocol(), direction, (yyvsp[(4) - (5)].integer), &error);
    if (direction == DIRECTION_INBOUND)
        pkt->setName("parserInbound");
    else
        pkt->setName("parserOutbound");
    inner = new PacketDrillPacket();
    inner->setInetPacket(pkt);
    inner->setDirection(direction);

    (yyval.packet) = inner;
;}
    break;

  case 31:
#line 576 "parser.y"
    {
    PacketDrillPacket *inner = NULL;
    enum direction_t direction = (yyvsp[(1) - (4)].packet)->getDirection();
    cPacket* pkt = PacketDrill::buildSCTPPacket(in_config->getWireProtocol(), direction, (yyvsp[(4) - (4)].sctp_chunk_list));
    if (pkt) {
        if (direction == DIRECTION_INBOUND)
            pkt->setName("parserInbound");
        else
            pkt->setName("parserOutbound");
        inner = new PacketDrillPacket();
        inner->setInetPacket(pkt);
        inner->setDirection(direction);
    } else {
        semantic_error("inbound packets must be fully specified");
    }
    (yyval.packet) = inner;
;}
    break;

  case 32:
#line 596 "parser.y"
    { (yyval.sctp_chunk_list) = new cQueue("sctpChunkList");
                                   (yyval.sctp_chunk_list)->insert((cObject*)(yyvsp[(1) - (1)].sctp_chunk)); ;}
    break;

  case 33:
#line 598 "parser.y"
    { (yyval.sctp_chunk_list) = (yyvsp[(1) - (3)].sctp_chunk_list);
                                   (yyval.sctp_chunk_list)->insert((yyvsp[(3) - (3)].sctp_chunk)); ;}
    break;

  case 34:
#line 604 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 35:
#line 605 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 36:
#line 606 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 37:
#line 607 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 38:
#line 608 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 39:
#line 609 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 40:
#line 610 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 41:
#line 611 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 42:
#line 612 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 43:
#line 613 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 44:
#line 614 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 45:
#line 615 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 46:
#line 616 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 47:
#line 617 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 48:
#line 622 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 49:
#line 623 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        semantic_error("flags value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 50:
#line 629 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        semantic_error("flags value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 51:
#line 638 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 52:
#line 639 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("length value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 53:
#line 648 "parser.y"
    { (yyval.byte_list) = NULL; ;}
    break;

  case 54:
#line 649 "parser.y"
    { (yyval.byte_list) = NULL; ;}
    break;

  case 55:
#line 650 "parser.y"
    { (yyval.byte_list) = (yyvsp[(4) - (5)].byte_list); ;}
    break;

  case 56:
#line 654 "parser.y"
    { (yyval.byte_list) = new PacketDrillBytes((yyvsp[(1) - (1)].byte)); ;}
    break;

  case 57:
#line 655 "parser.y"
    { (yyval.byte_list) = (yyvsp[(1) - (3)].byte_list);
                       (yyvsp[(1) - (3)].byte_list)->appendByte((yyvsp[(3) - (3)].byte)); ;}
    break;

  case 58:
#line 660 "parser.y"
    { (yyval.byte_list) = new PacketDrillBytes();;}
    break;

  case 59:
#line 661 "parser.y"
    { (yyval.byte_list) = new PacketDrillBytes((yyvsp[(1) - (1)].integer));;}
    break;

  case 60:
#line 662 "parser.y"
    { (yyval.byte_list) = (yyvsp[(1) - (3)].byte_list);
                       (yyvsp[(1) - (3)].byte_list)->appendByte((yyvsp[(3) - (3)].integer)); ;}
    break;

  case 61:
#line 667 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(1) - (1)].integer))) {
        semantic_error("byte value out of range");
    }
    (yyval.byte) = (yyvsp[(1) - (1)].integer);
;}
    break;

  case 62:
#line 673 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(1) - (1)].integer))) {
        semantic_error("byte value out of range");
    }
    (yyval.byte) = (yyvsp[(1) - (1)].integer);
;}
    break;

  case 63:
#line 682 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(1) - (1)].integer))) {
        semantic_error("type value out of range");
    }
    (yyval.integer) = (yyvsp[(1) - (1)].integer);
;}
    break;

  case 64:
#line 688 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(1) - (1)].integer))) {
        semantic_error("type value out of range");
    }
    (yyval.integer) = (yyvsp[(1) - (1)].integer);
;}
    break;

  case 65:
#line 694 "parser.y"
    {
    (yyval.integer) = SCTP_DATA_CHUNK_TYPE;
;}
    break;

  case 66:
#line 697 "parser.y"
    {
    (yyval.integer) = SCTP_INIT_CHUNK_TYPE;
;}
    break;

  case 67:
#line 700 "parser.y"
    {
    (yyval.integer) = SCTP_INIT_ACK_CHUNK_TYPE;
;}
    break;

  case 68:
#line 703 "parser.y"
    {
    (yyval.integer) = SCTP_SACK_CHUNK_TYPE;
;}
    break;

  case 69:
#line 706 "parser.y"
    {
    (yyval.integer) = SCTP_HEARTBEAT_CHUNK_TYPE;
;}
    break;

  case 70:
#line 709 "parser.y"
    {
    (yyval.integer) = SCTP_HEARTBEAT_ACK_CHUNK_TYPE;
;}
    break;

  case 71:
#line 712 "parser.y"
    {
    (yyval.integer) = SCTP_ABORT_CHUNK_TYPE;
;}
    break;

  case 72:
#line 715 "parser.y"
    {
    (yyval.integer) = SCTP_SHUTDOWN_CHUNK_TYPE;
;}
    break;

  case 73:
#line 718 "parser.y"
    {
    (yyval.integer) = SCTP_SHUTDOWN_ACK_CHUNK_TYPE;
;}
    break;

  case 74:
#line 721 "parser.y"
    {
    (yyval.integer) = SCTP_ERROR_CHUNK_TYPE;
;}
    break;

  case 75:
#line 724 "parser.y"
    {
    (yyval.integer) = SCTP_COOKIE_ECHO_CHUNK_TYPE;
;}
    break;

  case 76:
#line 727 "parser.y"
    {
    (yyval.integer) = SCTP_COOKIE_ACK_CHUNK_TYPE;
;}
    break;

  case 77:
#line 730 "parser.y"
    {
    (yyval.integer) = SCTP_SHUTDOWN_COMPLETE_CHUNK_TYPE;
;}
    break;

  case 78:
#line 733 "parser.y"
    {
    (yyval.integer) = SCTP_PAD_CHUNK_TYPE;
;}
    break;

  case 79:
#line 736 "parser.y"
    {
    (yyval.integer) = SCTP_RECONFIG_CHUNK_TYPE;
;}
    break;

  case 80:
#line 742 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 81:
#line 743 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        semantic_error("flags value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 82:
#line 749 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        semantic_error("flags value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 83:
#line 755 "parser.y"
    {
    uint64 flags;
    char *c;

    flags = 0;
    for (c = (yyvsp[(3) - (3)].string); *c != '\0'; c++) {
        switch (*c) {
        case 'I':
            if (flags & SCTP_DATA_CHUNK_I_BIT) {
                semantic_error("I-bit specified multiple times");
            } else {
                flags |= SCTP_DATA_CHUNK_I_BIT;
            }
            break;
        case 'U':
            if (flags & SCTP_DATA_CHUNK_U_BIT) {
                semantic_error("U-bit specified multiple times");
            } else {
                flags |= SCTP_DATA_CHUNK_U_BIT;
            }
            break;
        case 'B':
            if (flags & SCTP_DATA_CHUNK_B_BIT) {
                semantic_error("B-bit specified multiple times");
            } else {
                flags |= SCTP_DATA_CHUNK_B_BIT;
            }
            break;
        case 'E':
            if (flags & SCTP_DATA_CHUNK_E_BIT) {
                semantic_error("E-bit specified multiple times");
            } else {
                flags |= SCTP_DATA_CHUNK_E_BIT;
            }
            break;
        default:
            semantic_error("Only expecting IUBE as flags");
        }
    }
    (yyval.integer) = flags;
;}
    break;

  case 84:
#line 799 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 85:
#line 800 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        semantic_error("flags value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 86:
#line 806 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        semantic_error("flags value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 87:
#line 812 "parser.y"
    {
    uint64 flags;
    char *c;

    flags = 0;
    for (c = (yyvsp[(3) - (3)].string); *c != '\0'; c++) {
        switch (*c) {
        case 'T':
            if (flags & SCTP_ABORT_CHUNK_T_BIT) {
                semantic_error("T-bit specified multiple times");
            } else {
                flags |= SCTP_ABORT_CHUNK_T_BIT;
            }
            break;
        default:
            semantic_error("Only expecting T as flags");
        }
    }
    (yyval.integer) = flags;
;}
    break;

  case 88:
#line 835 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 89:
#line 836 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        semantic_error("flags value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 90:
#line 842 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        semantic_error("flags value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 91:
#line 848 "parser.y"
    {
    uint64 flags;
    char *c;

    flags = 0;
    for (c = (yyvsp[(3) - (3)].string); *c != '\0'; c++) {
        switch (*c) {
        case 'T':
            if (flags & SCTP_SHUTDOWN_COMPLETE_CHUNK_T_BIT) {
                semantic_error("T-bit specified multiple times");
            } else {
                flags |= SCTP_SHUTDOWN_COMPLETE_CHUNK_T_BIT;
            }
            break;
        default:
            semantic_error("Only expecting T as flags");
        }
    }
    (yyval.integer) = flags;
;}
    break;

  case 92:
#line 871 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 93:
#line 872 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("tag value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 94:
#line 881 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 95:
#line 882 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("a_rwnd value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 96:
#line 891 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 97:
#line 892 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("os value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 98:
#line 901 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 99:
#line 902 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("is value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 100:
#line 911 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 101:
#line 912 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("tsn value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 102:
#line 921 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 103:
#line 922 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("sid value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 104:
#line 931 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 105:
#line 932 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("ssn value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 106:
#line 942 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 107:
#line 943 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("ppid value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 108:
#line 949 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("ppid value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 109:
#line 958 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 110:
#line 959 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("cum_tsn value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 111:
#line 968 "parser.y"
    { (yyval.sack_block_list) = NULL; ;}
    break;

  case 112:
#line 969 "parser.y"
    { (yyval.sack_block_list) = NULL; ;}
    break;

  case 113:
#line 970 "parser.y"
    { (yyval.sack_block_list) = (yyvsp[(4) - (5)].sack_block_list); ;}
    break;

  case 114:
#line 975 "parser.y"
    { (yyval.sack_block_list) = NULL; ;}
    break;

  case 115:
#line 976 "parser.y"
    { (yyval.sack_block_list) = NULL; ;}
    break;

  case 116:
#line 977 "parser.y"
    { (yyval.sack_block_list) = (yyvsp[(4) - (5)].sack_block_list); ;}
    break;

  case 117:
#line 982 "parser.y"
    {
    if (((yyvsp[(5) - (14)].integer) != -1) &&
        (!is_valid_u16((yyvsp[(5) - (14)].integer)) || ((yyvsp[(5) - (14)].integer) < SCTP_DATA_CHUNK_LENGTH))) {
        semantic_error("length value out of range");
    }
    (yyval.sctp_chunk) = PacketDrill::buildDataChunk((yyvsp[(3) - (14)].integer), (yyvsp[(5) - (14)].integer), (yyvsp[(7) - (14)].integer), (yyvsp[(9) - (14)].integer), (yyvsp[(11) - (14)].integer), (yyvsp[(13) - (14)].integer));
;}
    break;

  case 118:
#line 991 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildInitChunk((yyvsp[(3) - (15)].integer), (yyvsp[(5) - (15)].integer), (yyvsp[(7) - (15)].integer), (yyvsp[(9) - (15)].integer), (yyvsp[(11) - (15)].integer), (yyvsp[(13) - (15)].integer), (yyvsp[(14) - (15)].expression_list));
;}
    break;

  case 119:
#line 996 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildInitAckChunk((yyvsp[(3) - (15)].integer), (yyvsp[(5) - (15)].integer), (yyvsp[(7) - (15)].integer), (yyvsp[(9) - (15)].integer), (yyvsp[(11) - (15)].integer), (yyvsp[(13) - (15)].integer), (yyvsp[(14) - (15)].expression_list));
;}
    break;

  case 120:
#line 1001 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildSackChunk((yyvsp[(3) - (12)].integer), (yyvsp[(5) - (12)].integer), (yyvsp[(7) - (12)].integer), (yyvsp[(9) - (12)].sack_block_list), (yyvsp[(11) - (12)].sack_block_list));
;}
    break;

  case 121:
#line 1006 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildHeartbeatChunk((yyvsp[(3) - (6)].integer), (yyvsp[(5) - (6)].sctp_parameter));
;}
    break;

  case 122:
#line 1012 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildHeartbeatAckChunk((yyvsp[(3) - (6)].integer), (yyvsp[(5) - (6)].sctp_parameter));
;}
    break;

  case 123:
#line 1018 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildAbortChunk((yyvsp[(3) - (4)].integer));
;}
    break;

  case 124:
#line 1023 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildShutdownChunk((yyvsp[(3) - (6)].integer), (yyvsp[(5) - (6)].integer));
;}
    break;

  case 125:
#line 1028 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildShutdownAckChunk((yyvsp[(3) - (4)].integer));
;}
    break;

  case 126:
#line 1033 "parser.y"
    {
    if (((yyvsp[(5) - (8)].integer) != -1) &&
        (!is_valid_u16((yyvsp[(5) - (8)].integer)) || ((yyvsp[(5) - (8)].integer) < SCTP_COOKIE_ACK_LENGTH))) {
        semantic_error("length value out of range");
    }
    if (((yyvsp[(5) - (8)].integer) != -1) && ((yyvsp[(7) - (8)].byte_list) != NULL) &&
        ((yyvsp[(5) - (8)].integer) != SCTP_COOKIE_ACK_LENGTH + (yyvsp[(7) - (8)].byte_list)->getListLength())) {
        semantic_error("length value incompatible with val");
    }
    if (((yyvsp[(5) - (8)].integer) == -1) && ((yyvsp[(7) - (8)].byte_list) != NULL)) {
        semantic_error("length needs to be specified");
    }
    (yyval.sctp_chunk) = PacketDrill::buildCookieEchoChunk((yyvsp[(3) - (8)].integer), (yyvsp[(5) - (8)].integer), (yyvsp[(7) - (8)].byte_list));
;}
    break;

  case 127:
#line 1049 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildCookieAckChunk((yyvsp[(3) - (4)].integer));
;}
    break;

  case 128:
#line 1054 "parser.y"
    { (yyval.cause_list) = NULL; ;}
    break;

  case 129:
#line 1055 "parser.y"
    { (yyval.cause_list) = new cQueue("empty"); ;}
    break;

  case 130:
#line 1056 "parser.y"
    { (yyval.cause_list) = (yyvsp[(2) - (2)].cause_list); ;}
    break;

  case 131:
#line 1060 "parser.y"
    { (yyval.cause_list) = new cQueue("cause list");
                                             (yyval.cause_list)->insert((yyvsp[(1) - (1)].cause_item)); ;}
    break;

  case 132:
#line 1062 "parser.y"
    { (yyval.cause_list) = (yyvsp[(1) - (3)].cause_list);
                                             (yyval.cause_list)->insert((yyvsp[(3) - (3)].cause_item)); ;}
    break;

  case 133:
#line 1067 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(5) - (6)].integer))) {
        semantic_error("stream identifier out of range");
    }
    (yyval.cause_item) = new PacketDrillStruct(INVALID_STREAM_IDENTIFIER, (yyvsp[(5) - (6)].integer));
;}
    break;

  case 134:
#line 1073 "parser.y"
    {
    (yyval.cause_item) = new PacketDrillStruct(INVALID_STREAM_IDENTIFIER, -1);
;}
    break;

  case 135:
#line 1078 "parser.y"
    { (yyval.cause_item) = (yyvsp[(1) - (1)].cause_item); ;}
    break;

  case 136:
#line 1082 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildErrorChunk((yyvsp[(3) - (5)].integer), (yyvsp[(4) - (5)].cause_list));
;}
    break;

  case 137:
#line 1087 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildShutdownCompleteChunk((yyvsp[(3) - (4)].integer));
;}
    break;

  case 138:
#line 1093 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("req_sn out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 139:
#line 1099 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 140:
#line 1103 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("resp_sn out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 141:
#line 1109 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 142:
#line 1113 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
    semantic_error("last_tsn out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 143:
#line 1119 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 144:
#line 1123 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
    semantic_error("result out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 145:
#line 1129 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 146:
#line 1133 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
    semantic_error("sender_next_tsn out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 147:
#line 1139 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
    semantic_error("sender_next_tsn out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 148:
#line 1145 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 149:
#line 1149 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
    semantic_error("receiver_next_tsn out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 150:
#line 1155 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("receiver_next_tsn out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 151:
#line 1161 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 152:
#line 1165 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("number_of_new_streams out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 153:
#line 1171 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 154:
#line 1175 "parser.y"
    {
    (yyval.stream_list) = new cQueue("stream_list");
    (yyval.stream_list)->insert((yyvsp[(1) - (1)].expression));
;}
    break;

  case 155:
#line 1179 "parser.y"
    {
    (yyval.stream_list) = (yyvsp[(1) - (3)].stream_list); (yyval.stream_list)->insert((yyvsp[(3) - (3)].expression));
;}
    break;

  case 156:
#line 1185 "parser.y"
    {
    (yyval.expression) = new_integer_expression(-1, "%d");
;}
    break;

  case 157:
#line 1188 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(1) - (1)].integer))) {
        semantic_error("Stream number value out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(1) - (1)].integer), "%u");
;}
    break;

  case 158:
#line 1198 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(OUTGOING_RESET_REQUEST_PARAMETER, 16, new PacketDrillStruct((yyvsp[(3) - (8)].integer), (yyvsp[(5) - (8)].integer), (yyvsp[(7) - (8)].integer), -2, NULL));
;}
    break;

  case 159:
#line 1201 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(OUTGOING_RESET_REQUEST_PARAMETER, 16, new PacketDrillStruct((yyvsp[(3) - (14)].integer), (yyvsp[(5) - (14)].integer), (yyvsp[(7) - (14)].integer), -2, (yyvsp[(12) - (14)].stream_list)));
;}
    break;

  case 160:
#line 1207 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(INCOMING_RESET_REQUEST_PARAMETER, 8, new PacketDrillStruct((yyvsp[(3) - (4)].integer), -2, -2, -2, NULL));
;}
    break;

  case 161:
#line 1210 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(INCOMING_RESET_REQUEST_PARAMETER, 8, new PacketDrillStruct((yyvsp[(3) - (10)].integer), -2, -2, -2, (yyvsp[(8) - (10)].stream_list)));
;}
    break;

  case 162:
#line 1216 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(SSN_TSN_RESET_REQUEST_PARAMETER, 8, new PacketDrillStruct((yyvsp[(3) - (4)].integer), -2, -2, -2, NULL));
;}
    break;

  case 163:
#line 1222 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(STREAM_RESET_RESPONSE_PARAMETER, 8, new PacketDrillStruct((yyvsp[(3) - (6)].integer), (yyvsp[(5) - (6)].integer), -2, -2, NULL));
;}
    break;

  case 164:
#line 1225 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(STREAM_RESET_RESPONSE_PARAMETER, 12, new PacketDrillStruct((yyvsp[(3) - (10)].integer), (yyvsp[(5) - (10)].integer), (yyvsp[(7) - (10)].integer), (yyvsp[(9) - (10)].integer), NULL));
;}
    break;

  case 165:
#line 1231 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(ADD_OUTGOING_STREAMS_REQUEST_PARAMETER, 12, new PacketDrillStruct((yyvsp[(3) - (6)].integer), (yyvsp[(5) - (6)].integer), -2, -2, NULL));
;}
    break;

  case 166:
#line 1237 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(ADD_INCOMING_STREAMS_REQUEST_PARAMETER, 12, new PacketDrillStruct((yyvsp[(3) - (6)].integer), (yyvsp[(5) - (6)].integer), -2, -2, NULL));
;}
    break;

  case 167:
#line 1249 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildReconfigChunk((yyvsp[(3) - (5)].integer), (yyvsp[(4) - (5)].expression_list));
;}
    break;

  case 168:
#line 1255 "parser.y"
    { (yyval.expression_list) = NULL; ;}
    break;

  case 169:
#line 1256 "parser.y"
    { (yyval.expression_list) = new cQueue("empty"); ;}
    break;

  case 170:
#line 1257 "parser.y"
    { (yyval.expression_list) = (yyvsp[(2) - (2)].expression_list); ;}
    break;

  case 171:
#line 1261 "parser.y"
    {
    (yyval.expression_list) = new cQueue("sctp_parameter_list");
    (yyval.expression_list)->insert((yyvsp[(1) - (1)].sctp_parameter));
;}
    break;

  case 172:
#line 1265 "parser.y"
    {
    (yyval.expression_list) = (yyvsp[(1) - (3)].expression_list);
    (yyval.expression_list)->insert((yyvsp[(3) - (3)].sctp_parameter));
;}
    break;

  case 173:
#line 1273 "parser.y"
    { (yyval.sctp_parameter) = (yyvsp[(1) - (1)].sctp_parameter); ;}
    break;

  case 174:
#line 1274 "parser.y"
    { (yyval.sctp_parameter) = (yyvsp[(1) - (1)].sctp_parameter); ;}
    break;

  case 175:
#line 1275 "parser.y"
    { (yyval.sctp_parameter) = (yyvsp[(1) - (1)].sctp_parameter); ;}
    break;

  case 176:
#line 1276 "parser.y"
    { (yyval.sctp_parameter) = (yyvsp[(1) - (1)].sctp_parameter); ;}
    break;

  case 177:
#line 1277 "parser.y"
    { (yyval.sctp_parameter) = (yyvsp[(1) - (1)].sctp_parameter); ;}
    break;

  case 178:
#line 1278 "parser.y"
    { (yyval.sctp_parameter) = (yyvsp[(1) - (1)].sctp_parameter); ;}
    break;

  case 179:
#line 1279 "parser.y"
    { (yyval.sctp_parameter) = (yyvsp[(1) - (1)].sctp_parameter); ;}
    break;

  case 180:
#line 1280 "parser.y"
    { (yyval.sctp_parameter) = (yyvsp[(1) - (1)].sctp_parameter); ;}
    break;

  case 181:
#line 1281 "parser.y"
    { (yyval.sctp_parameter) = (yyvsp[(1) - (1)].sctp_parameter); ;}
    break;

  case 182:
#line 1282 "parser.y"
    { (yyval.sctp_parameter) = (yyvsp[(1) - (1)].sctp_parameter); ;}
    break;

  case 183:
#line 1287 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(HEARTBEAT_INFORMATION, -1, NULL);
;}
    break;

  case 184:
#line 1290 "parser.y"
    {
    if (((yyvsp[(3) - (6)].integer) != -1) &&
        (!is_valid_u16((yyvsp[(3) - (6)].integer)) || ((yyvsp[(3) - (6)].integer) < 4))) {
        semantic_error("length value out of range");
    }
    if (((yyvsp[(3) - (6)].integer) != -1) && ((yyvsp[(5) - (6)].byte_list) != NULL) &&
        ((yyvsp[(3) - (6)].integer) != 4 + (yyvsp[(5) - (6)].byte_list)->getListLength())) {
        semantic_error("length value incompatible with val");
    }
    if (((yyvsp[(3) - (6)].integer) == -1) && ((yyvsp[(5) - (6)].byte_list) != NULL)) {
        semantic_error("length needs to be specified");
    }
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(HEARTBEAT_INFORMATION, (yyvsp[(3) - (6)].integer), (yyvsp[(5) - (6)].byte_list));
;}
    break;

  case 185:
#line 1306 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(SUPPORTED_EXTENSIONS, -1, NULL);
;}
    break;

  case 186:
#line 1309 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(SUPPORTED_EXTENSIONS, (yyvsp[(6) - (8)].byte_list)->getListLength(), (yyvsp[(6) - (8)].byte_list));
;}
    break;

  case 187:
#line 1314 "parser.y"
    { (yyval.stream_list) = new cQueue("empty_address_types_list");
;}
    break;

  case 188:
#line 1316 "parser.y"
    { (yyval.stream_list) = new cQueue("address_types_list");
                                        (yyval.stream_list)->insert((yyvsp[(1) - (1)].expression));
;}
    break;

  case 189:
#line 1319 "parser.y"
    { (yyval.stream_list) = (yyvsp[(1) - (3)].stream_list);
                                        (yyval.stream_list)->insert((yyvsp[(3) - (3)].expression));
;}
    break;

  case 190:
#line 1325 "parser.y"
    { if (!is_valid_u16((yyvsp[(1) - (1)].integer))) {
                  semantic_error("address type value out of range");
                  }
                  (yyval.expression) = new_integer_expression((yyvsp[(1) - (1)].integer), "%u"); ;}
    break;

  case 191:
#line 1329 "parser.y"
    { (yyval.expression) = new_integer_expression(SCTP_IPV4_ADDRESS_PARAMETER_TYPE, "%u"); ;}
    break;

  case 192:
#line 1330 "parser.y"
    { (yyval.expression) = new_integer_expression(SCTP_IPV6_ADDRESS_PARAMETER_TYPE, "%u"); ;}
    break;

  case 193:
#line 1334 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(SUPPORTED_ADDRESS_TYPES, -1, NULL);
;}
    break;

  case 194:
#line 1337 "parser.y"
    {
(yyvsp[(6) - (8)].stream_list)->setName("SupportedAddressTypes");
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(SUPPORTED_ADDRESS_TYPES, (yyvsp[(6) - (8)].stream_list)->getLength(), (yyvsp[(6) - (8)].stream_list));
;}
    break;

  case 195:
#line 1343 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(STATE_COOKIE, -1, NULL);
;}
    break;

  case 196:
#line 1346 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(STATE_COOKIE, -1, NULL);
;}
    break;

  case 197:
#line 1349 "parser.y"
    {
    if (((yyvsp[(5) - (10)].integer) < 4) || !is_valid_u32((yyvsp[(5) - (10)].integer))) {
        semantic_error("len value out of range");
    }
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(STATE_COOKIE, (yyvsp[(5) - (10)].integer), NULL);
;}
    break;

  case 198:
#line 1359 "parser.y"
    {
    (yyval.packet) = new PacketDrillPacket();
    (yyval.packet)->setDirection((yyvsp[(1) - (1)].direction));
;}
    break;

  case 199:
#line 1367 "parser.y"
    {
    (yyval.direction) = DIRECTION_INBOUND;
    current_script_line = yylineno;
;}
    break;

  case 200:
#line 1371 "parser.y"
    {
    (yyval.direction) = DIRECTION_OUTBOUND;
    current_script_line = yylineno;
;}
    break;

  case 201:
#line 1378 "parser.y"
    {
    (yyval.string) = (yyvsp[(1) - (1)].string);
;}
    break;

  case 202:
#line 1381 "parser.y"
    {
    (yyval.string) = strdup(".");
;}
    break;

  case 203:
#line 1384 "parser.y"
    {
    asprintf(&((yyval.string)), "%s.", (yyvsp[(1) - (2)].string));
    free((yyvsp[(1) - (2)].string));
;}
    break;

  case 204:
#line 1388 "parser.y"
    {
    (yyval.string) = strdup("");
;}
    break;

  case 205:
#line 1394 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(1) - (6)].integer))) {
        semantic_error("TCP start sequence number out of range");
    }
    if (!is_valid_u32((yyvsp[(3) - (6)].integer))) {
        semantic_error("TCP end sequence number out of range");
    }
    if (!is_valid_u16((yyvsp[(5) - (6)].integer))) {
        semantic_error("TCP payload size out of range");
    }
    if ((yyvsp[(3) - (6)].integer) != ((yyvsp[(1) - (6)].integer) +(yyvsp[(5) - (6)].integer))) {
        semantic_error("inconsistent TCP sequence numbers and payload size");
    }
    (yyval.tcp_sequence_info).start_sequence = (yyvsp[(1) - (6)].integer);
    (yyval.tcp_sequence_info).payload_bytes = (yyvsp[(5) - (6)].integer);
    (yyval.tcp_sequence_info).protocol = IPPROTO_TCP;
;}
    break;

  case 206:
#line 1414 "parser.y"
    {
    (yyval.sequence_number) = 0;
;}
    break;

  case 207:
#line 1417 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(2) - (2)].integer))) {
        semantic_error("TCP ack sequence number out of range");
    }
    (yyval.sequence_number) = (yyvsp[(2) - (2)].integer);
;}
    break;

  case 208:
#line 1426 "parser.y"
    {
    (yyval.window) = -1;
;}
    break;

  case 209:
#line 1429 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(2) - (2)].integer))) {
        semantic_error("TCP window value out of range");
    }
    (yyval.window) = (yyvsp[(2) - (2)].integer);
;}
    break;

  case 210:
#line 1438 "parser.y"
    {
    (yyval.tcp_options) = new cQueue("opt_tcp_options");
;}
    break;

  case 211:
#line 1441 "parser.y"
    {
    (yyval.tcp_options) = (yyvsp[(2) - (3)].tcp_options);
;}
    break;

  case 212:
#line 1444 "parser.y"
    {
    (yyval.tcp_options) = NULL; /* FLAG_OPTIONS_NOCHECK */
;}
    break;

  case 213:
#line 1451 "parser.y"
    {
    (yyval.tcp_options) = new cQueue("tcp_option");
    (yyval.tcp_options)->insert((yyvsp[(1) - (1)].tcp_option));
;}
    break;

  case 214:
#line 1455 "parser.y"
    {
    (yyval.tcp_options) = (yyvsp[(1) - (3)].tcp_options);
    (yyval.tcp_options)->insert((yyvsp[(3) - (3)].tcp_option));
;}
    break;

  case 215:
#line 1463 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_NOP, 1);
;}
    break;

  case 216:
#line 1466 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_EOL, 1);
;}
    break;

  case 217:
#line 1469 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_MAXSEG, TCPOLEN_MAXSEG);
    if (!is_valid_u16((yyvsp[(2) - (2)].integer))) {
        semantic_error("mss value out of range");
    }
    (yyval.tcp_option)->setMss((yyvsp[(2) - (2)].integer));
;}
    break;

  case 218:
#line 1476 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_WINDOW, TCPOLEN_WINDOW);
    if (!is_valid_u8((yyvsp[(2) - (2)].integer))) {
        semantic_error("window scale shift count out of range");
    }
    (yyval.tcp_option)->setWindowScale((yyvsp[(2) - (2)].integer));
;}
    break;

  case 219:
#line 1483 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_SACK_PERMITTED, TCPOLEN_SACK_PERMITTED);
;}
    break;

  case 220:
#line 1486 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_SACK, 2+8*(yyvsp[(2) - (2)].sack_block_list)->getLength());
    (yyval.tcp_option)->setBlockList((yyvsp[(2) - (2)].sack_block_list));
;}
    break;

  case 221:
#line 1490 "parser.y"
    {
    uint32 val, ecr;
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_TIMESTAMP, TCPOLEN_TIMESTAMP);
    if (!is_valid_u32((yyvsp[(3) - (5)].integer))) {
        semantic_error("ts val out of range");
    }
    if (!is_valid_u32((yyvsp[(5) - (5)].integer))) {
        semantic_error("ecr val out of range");
    }
    val = (yyvsp[(3) - (5)].integer);
    ecr = (yyvsp[(5) - (5)].integer);
    (yyval.tcp_option)->setVal(val);
    (yyval.tcp_option)->setEcr(ecr);
;}
    break;

  case 222:
#line 1507 "parser.y"
    {
    (yyval.sack_block_list) = new cQueue("sack_block_list");
    (yyval.sack_block_list)->insert((yyvsp[(1) - (1)].sack_block));
;}
    break;

  case 223:
#line 1511 "parser.y"
    {
    (yyval.sack_block_list) = (yyvsp[(1) - (2)].sack_block_list); (yyval.sack_block_list)->insert((yyvsp[(2) - (2)].sack_block));
;}
    break;

  case 224:
#line 1517 "parser.y"
    { (yyval.sack_block_list) = new cQueue("gap_list");;}
    break;

  case 225:
#line 1518 "parser.y"
    {
    (yyval.sack_block_list) = new cQueue("gap_list");
    (yyval.sack_block_list)->insert((yyvsp[(1) - (1)].sack_block));
;}
    break;

  case 226:
#line 1522 "parser.y"
    {
    (yyval.sack_block_list) = (yyvsp[(1) - (3)].sack_block_list); (yyval.sack_block_list)->insert((yyvsp[(3) - (3)].sack_block));
;}
    break;

  case 227:
#line 1528 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(1) - (3)].integer))) {
        semantic_error("start value out of range");
    }
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("end value out of range");
    }
    (yyval.sack_block) = new PacketDrillStruct((yyvsp[(1) - (3)].integer), (yyvsp[(3) - (3)].integer));
;}
    break;

  case 228:
#line 1540 "parser.y"
    { (yyval.sack_block_list) = new cQueue("dup_list");;}
    break;

  case 229:
#line 1541 "parser.y"
    {
    (yyval.sack_block_list) = new cQueue("dup_list");
    (yyval.sack_block_list)->insert((yyvsp[(1) - (1)].sack_block));
;}
    break;

  case 230:
#line 1545 "parser.y"
    {
    (yyval.sack_block_list) = (yyvsp[(1) - (3)].sack_block_list); (yyval.sack_block_list)->insert((yyvsp[(3) - (3)].sack_block));
;}
    break;

  case 231:
#line 1551 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(1) - (3)].integer))) {
        semantic_error("start value out of range");
    }
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("end value out of range");
    }
    (yyval.sack_block) = new PacketDrillStruct((yyvsp[(1) - (3)].integer), (yyvsp[(3) - (3)].integer));
;}
    break;

  case 232:
#line 1563 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(1) - (3)].integer))) {
        semantic_error("TCP SACK left sequence number out of range\n");
    }
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("TCP SACK right sequence number out of range");
    }
    PacketDrillStruct *block = new PacketDrillStruct((yyvsp[(1) - (3)].integer), (yyvsp[(3) - (3)].integer));
    (yyval.sack_block) = block;
;}
    break;

  case 233:
#line 1576 "parser.y"
    {
    (yyval.syscall) = (struct syscall_spec *)calloc(1, sizeof(struct syscall_spec));
    (yyval.syscall)->end_usecs = (yyvsp[(1) - (7)].time_usecs);
    (yyval.syscall)->name = (yyvsp[(2) - (7)].string);
    (yyval.syscall)->arguments = (yyvsp[(3) - (7)].expression_list);
    (yyval.syscall)->result = (yyvsp[(5) - (7)].expression);
    (yyval.syscall)->error = (yyvsp[(6) - (7)].errno_info);
    (yyval.syscall)->note = (yyvsp[(7) - (7)].string);
;}
    break;

  case 234:
#line 1588 "parser.y"
    {
    (yyval.time_usecs) = -1;
;}
    break;

  case 235:
#line 1591 "parser.y"
    {
    (yyval.time_usecs) = (yyvsp[(2) - (2)].time_usecs);
;}
    break;

  case 236:
#line 1597 "parser.y"
    {
    (yyval.string) = (yyvsp[(1) - (1)].string);
    current_script_line = yylineno;
;}
    break;

  case 237:
#line 1604 "parser.y"
    {
    (yyval.expression_list) = NULL;
;}
    break;

  case 238:
#line 1607 "parser.y"
    {
    (yyval.expression_list) = (yyvsp[(2) - (3)].expression_list);
;}
    break;

  case 239:
#line 1613 "parser.y"
    {
    (yyval.expression_list) = new cQueue("new_expressionList");
    (yyval.expression_list)->insert((cObject*)(yyvsp[(1) - (1)].expression));
;}
    break;

  case 240:
#line 1617 "parser.y"
    {
    (yyval.expression_list) = (yyvsp[(1) - (3)].expression_list);
    (yyval.expression_list)->insert((yyvsp[(3) - (3)].expression));
;}
    break;

  case 241:
#line 1624 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS);
;}
    break;

  case 242:
#line 1627 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression); ;}
    break;

  case 243:
#line 1629 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 244:
#line 1632 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (4)].integer))) {
        semantic_error("number out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (4)].integer), "%lu");
;}
    break;

  case 245:
#line 1638 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (4)].integer))) {
        semantic_error("number out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (4)].integer), "%lu");
;}
    break;

  case 246:
#line 1644 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (4)].integer))) {
        semantic_error("number out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (4)].integer), "%lu");
;}
    break;

  case 247:
#line 1650 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_WORD);
    (yyval.expression)->setString((yyvsp[(1) - (1)].string));
;}
    break;

  case 248:
#line 1654 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_STRING);
    (yyval.expression)->setString((yyvsp[(1) - (1)].string));
    (yyval.expression)->setFormat("\"%s\"");
;}
    break;

  case 249:
#line 1659 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_STRING);
    (yyval.expression)->setString((yyvsp[(1) - (2)].string));
    (yyval.expression)->setFormat("\"%s\"...");
;}
    break;

  case 250:
#line 1664 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 251:
#line 1667 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 252:
#line 1670 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 253:
#line 1673 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 254:
#line 1676 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 255:
#line 1679 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 256:
#line 1682 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 257:
#line 1685 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 258:
#line 1688 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 259:
#line 1691 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 260:
#line 1694 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 261:
#line 1697 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 262:
#line 1700 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 263:
#line 1708 "parser.y"
    {
    (yyval.expression) = new_integer_expression((yyvsp[(1) - (1)].integer), "%ld");
;}
    break;

  case 264:
#line 1714 "parser.y"
    {
    (yyval.expression) = new_integer_expression((yyvsp[(1) - (1)].integer), "%#lx");
;}
    break;

  case 265:
#line 1720 "parser.y"
    {    /* bitwise OR */
    (yyval.expression) = new PacketDrillExpression(EXPR_BINARY);
    struct binary_expression *binary = (struct binary_expression *) malloc(sizeof(struct binary_expression));
    binary->op = strdup("|");
    binary->lhs = (yyvsp[(1) - (3)].expression);
    binary->rhs = (yyvsp[(3) - (3)].expression);
    (yyval.expression)->setBinary(binary);
;}
    break;

  case 266:
#line 1731 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_LIST);
    (yyval.expression)->setList(NULL);
;}
    break;

  case 267:
#line 1735 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_LIST);
    (yyval.expression)->setList((yyvsp[(2) - (3)].expression_list));
;}
    break;

  case 268:
#line 1742 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("srto_initial out of range\n");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 269:
#line 1748 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS);
;}
    break;

  case 270:
#line 1754 "parser.y"
    {
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 271:
#line 1757 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 272:
#line 1761 "parser.y"
    {
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 273:
#line 1764 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 274:
#line 1768 "parser.y"
    {
    (yyval.expression) = new_integer_expression((yyvsp[(1) - (1)].integer), "%u");
;}
    break;

  case 275:
#line 1771 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_WORD);
    (yyval.expression)->setString((yyvsp[(1) - (1)].string));
;}
    break;

  case 276:
#line 1775 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 277:
#line 1779 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_RTOINFO);
    struct sctp_rtoinfo_expr *rtoinfo = (struct sctp_rtoinfo_expr *) malloc(sizeof(struct sctp_rtoinfo_expr));
    rtoinfo->srto_assoc_id = (yyvsp[(4) - (11)].expression);
    rtoinfo->srto_initial = (yyvsp[(6) - (11)].expression);
    rtoinfo->srto_max = (yyvsp[(8) - (11)].expression);
    rtoinfo->srto_min = (yyvsp[(10) - (11)].expression);
    (yyval.expression)->setRtoinfo(rtoinfo);
;}
    break;

  case 278:
#line 1788 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_RTOINFO);
    struct sctp_rtoinfo_expr *rtoinfo = (struct sctp_rtoinfo_expr *) malloc(sizeof(struct sctp_rtoinfo_expr));
    rtoinfo->srto_assoc_id = new PacketDrillExpression(EXPR_ELLIPSIS);
    rtoinfo->srto_initial = (yyvsp[(2) - (7)].expression);
    rtoinfo->srto_max = (yyvsp[(4) - (7)].expression);
    rtoinfo->srto_min = (yyvsp[(6) - (7)].expression);
    (yyval.expression)->setRtoinfo(rtoinfo);
;}
    break;

  case 279:
#line 1800 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("sasoc_asocmaxrxt out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 280:
#line 1806 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 281:
#line 1810 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("sasoc_number_peer_destinations out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 282:
#line 1816 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 283:
#line 1820 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("sasoc_peer_rwnd out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 284:
#line 1826 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 285:
#line 1830 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("sasoc_local_rwnd out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 286:
#line 1836 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 287:
#line 1840 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("sasoc_cookie_life out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 288:
#line 1846 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 289:
#line 1851 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_ASSOCPARAMS);
    struct sctp_assocparams_expr *assocparams = (struct sctp_assocparams_expr *) malloc(sizeof(struct sctp_assocparams_expr));
    assocparams->sasoc_assoc_id = (yyvsp[(4) - (15)].expression);
    assocparams->sasoc_asocmaxrxt = (yyvsp[(6) - (15)].expression);
    assocparams->sasoc_number_peer_destinations = (yyvsp[(8) - (15)].expression);
    assocparams->sasoc_peer_rwnd = (yyvsp[(10) - (15)].expression);
    assocparams->sasoc_local_rwnd = (yyvsp[(12) - (15)].expression);
    assocparams->sasoc_cookie_life = (yyvsp[(14) - (15)].expression);
    (yyval.expression)->setAssocParams(assocparams);
;}
    break;

  case 290:
#line 1863 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_ASSOCPARAMS);
    struct sctp_assocparams_expr *assocparams = (struct sctp_assocparams_expr *) malloc(sizeof(struct sctp_assocparams_expr));
    assocparams->sasoc_assoc_id = new PacketDrillExpression(EXPR_ELLIPSIS);
    assocparams->sasoc_asocmaxrxt = (yyvsp[(2) - (11)].expression);
    assocparams->sasoc_number_peer_destinations = (yyvsp[(4) - (11)].expression);
    assocparams->sasoc_peer_rwnd = (yyvsp[(6) - (11)].expression);
    assocparams->sasoc_local_rwnd = (yyvsp[(8) - (11)].expression);
    assocparams->sasoc_cookie_life = (yyvsp[(10) - (11)].expression);
    (yyval.expression)->setAssocParams(assocparams);
;}
    break;

  case 291:
#line 1878 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("sinit_num_ostreams out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 292:
#line 1884 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 293:
#line 1888 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("sinit_max_instreams out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 294:
#line 1894 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 295:
#line 1898 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("sinit_max_attempts out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 296:
#line 1904 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 297:
#line 1908 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("sinit_max_init_timeo out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 298:
#line 1914 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 299:
#line 1919 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_INITMSG);
    struct sctp_initmsg_expr *initmsg = (struct sctp_initmsg_expr *) malloc(sizeof(struct sctp_initmsg_expr));
    initmsg->sinit_num_ostreams = (yyvsp[(2) - (9)].expression);
    initmsg->sinit_max_instreams = (yyvsp[(4) - (9)].expression);
    initmsg->sinit_max_attempts = (yyvsp[(6) - (9)].expression);
    initmsg->sinit_max_init_timeo = (yyvsp[(8) - (9)].expression);
    (yyval.expression)->setInitmsg(initmsg);
;}
    break;

  case 300:
#line 1933 "parser.y"
    {
    if (strcmp((yyvsp[(4) - (19)].string), "AF_INET") == 0) {
        (yyval.expression) = new PacketDrillExpression(EXPR_SOCKET_ADDRESS_IPV4);
        (yyval.expression)->setIp(new L3Address(IPv4Address()));
    } else if (strcmp((yyvsp[(4) - (19)].string), "AF_INET6") == 0) {
        (yyval.expression) = new PacketDrillExpression(EXPR_SOCKET_ADDRESS_IPV6);
        (yyval.expression)->setIp(new L3Address(IPv6Address()));
    }
;}
    break;

  case 301:
#line 1945 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 302:
#line 1946 "parser.y"
    { (yyval.expression) = (yyvsp[(3) - (3)].expression); ;}
    break;

  case 303:
#line 1950 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("spp_hbinterval out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 304:
#line 1956 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 305:
#line 1960 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
         semantic_error("spp_pathmtu out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 306:
#line 1966 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 307:
#line 1970 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("spp_pathmaxrxt out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 308:
#line 1976 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 309:
#line 1980 "parser.y"
    { (yyval.expression) = (yyvsp[(3) - (3)].expression); ;}
    break;

  case 310:
#line 1984 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("spp_ipv6_flowlabel out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 311:
#line 1990 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 312:
#line 1994 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        semantic_error("spp_dscp out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hhu");
;}
    break;

  case 313:
#line 2000 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 314:
#line 2005 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_PEER_ADDR_PARAMS);
    struct sctp_paddrparams_expr *params = (struct sctp_paddrparams_expr *) malloc(sizeof(struct sctp_paddrparams_expr));
    params->spp_assoc_id = (yyvsp[(4) - (19)].expression);
    params->spp_address = (yyvsp[(6) - (19)].expression);
    params->spp_hbinterval = (yyvsp[(8) - (19)].expression);
    params->spp_pathmaxrxt = (yyvsp[(10) - (19)].expression);
    params->spp_pathmtu = (yyvsp[(12) - (19)].expression);
    params->spp_flags = (yyvsp[(14) - (19)].expression);
    params->spp_ipv6_flowlabel = (yyvsp[(16) - (19)].expression);
    params->spp_dscp = (yyvsp[(18) - (19)].expression);
    (yyval.expression)->setPaddrParams(params);
;}
    break;

  case 315:
#line 2019 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_PEER_ADDR_PARAMS);
    struct sctp_paddrparams_expr *params = (struct sctp_paddrparams_expr *) malloc(sizeof(struct sctp_paddrparams_expr));
    params->spp_assoc_id = new PacketDrillExpression(EXPR_ELLIPSIS);
    params->spp_address = (yyvsp[(2) - (15)].expression);
    params->spp_hbinterval = (yyvsp[(4) - (15)].expression);
    params->spp_pathmaxrxt = (yyvsp[(6) - (15)].expression);
    params->spp_pathmtu = (yyvsp[(8) - (15)].expression);
    params->spp_flags = (yyvsp[(10) - (15)].expression);
    params->spp_ipv6_flowlabel = (yyvsp[(12) - (15)].expression);
    params->spp_dscp = (yyvsp[(14) - (15)].expression);
    (yyval.expression)->setPaddrParams(params);
;}
    break;

  case 316:
#line 2035 "parser.y"
    { (yyval.expression) = (yyvsp[(3) - (3)].expression); ;}
    break;

  case 317:
#line 2039 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("sstat_rwnd out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 318:
#line 2045 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 319:
#line 2049 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("sstat_unackdata out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 320:
#line 2055 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 321:
#line 2059 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("sstat_penddata out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 322:
#line 2065 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 323:
#line 2069 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("sstat_instrms out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 324:
#line 2075 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 325:
#line 2079 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("sstat_outstrms out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 326:
#line 2085 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 327:
#line 2089 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("sstat_fragmentation_point out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 328:
#line 2095 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 329:
#line 2099 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 330:
#line 2105 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_STATUS);
    struct sctp_status_expr *stat = (struct sctp_status_expr *) calloc(1, sizeof(struct sctp_status_expr));
    stat->sstat_assoc_id = (yyvsp[(4) - (21)].expression);
    stat->sstat_state = (yyvsp[(6) - (21)].expression);
    stat->sstat_rwnd = (yyvsp[(8) - (21)].expression);
    stat->sstat_unackdata = (yyvsp[(10) - (21)].expression);
    stat->sstat_penddata = (yyvsp[(12) - (21)].expression);
    stat->sstat_instrms = (yyvsp[(14) - (21)].expression);
    stat->sstat_outstrms = (yyvsp[(16) - (21)].expression);
    stat->sstat_fragmentation_point = (yyvsp[(18) - (21)].expression);
    stat->sstat_primary = (yyvsp[(20) - (21)].expression);
    (yyval.expression)->setStatus(stat);
;}
    break;

  case 331:
#line 2120 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_STATUS);
    struct sctp_status_expr *stat = (struct sctp_status_expr *) calloc(1, sizeof(struct sctp_status_expr));
    stat->sstat_assoc_id = new PacketDrillExpression(EXPR_ELLIPSIS);
    stat->sstat_state = (yyvsp[(2) - (17)].expression);
    stat->sstat_rwnd = (yyvsp[(4) - (17)].expression);
    stat->sstat_unackdata = (yyvsp[(6) - (17)].expression);
    stat->sstat_penddata = (yyvsp[(8) - (17)].expression);
    stat->sstat_instrms = (yyvsp[(10) - (17)].expression);
    stat->sstat_outstrms = (yyvsp[(12) - (17)].expression);
    stat->sstat_fragmentation_point = (yyvsp[(14) - (17)].expression);
    stat->sstat_primary = (yyvsp[(16) - (17)].expression);
    (yyval.expression)->setStatus(stat);
;}
    break;

  case 332:
#line 2137 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("sinfo_stream out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 333:
#line 2143 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 334:
#line 2147 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("sinfo_ssn out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 335:
#line 2153 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 336:
#line 2157 "parser.y"
    { (yyval.expression) = (yyvsp[(3) - (3)].expression); ;}
    break;

  case 337:
#line 2161 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(5) - (6)].integer))) {
        semantic_error("sinfo_ppid out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(5) - (6)].integer), "%u");
;}
    break;

  case 338:
#line 2167 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 339:
#line 2171 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("sinfo_context out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 340:
#line 2177 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 341:
#line 2181 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("sinfo_timetolive out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 342:
#line 2187 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 343:
#line 2191 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("sinfo_tsn out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 344:
#line 2197 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 345:
#line 2201 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("sinfo_cumtsn out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 346:
#line 2207 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 347:
#line 2213 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_SNDRCVINFO);
    struct sctp_sndrcvinfo_expr *info = (struct sctp_sndrcvinfo_expr *) calloc(1, sizeof(struct sctp_sndrcvinfo_expr));
    info->sinfo_stream = (yyvsp[(2) - (21)].expression);
    info->sinfo_ssn = (yyvsp[(4) - (21)].expression);
    info->sinfo_flags = (yyvsp[(6) - (21)].expression);
    info->sinfo_ppid = (yyvsp[(8) - (21)].expression);
    info->sinfo_context = (yyvsp[(10) - (21)].expression);
    info->sinfo_timetolive = (yyvsp[(12) - (21)].expression);
    info->sinfo_tsn = (yyvsp[(14) - (21)].expression);
    info->sinfo_cumtsn = (yyvsp[(16) - (21)].expression);
    info->sinfo_assoc_id = (yyvsp[(20) - (21)].expression);
    (yyval.expression)->setSndRcvInfo(info);
;}
    break;

  case 348:
#line 2228 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_SNDRCVINFO);
    struct sctp_sndrcvinfo_expr *info = (struct sctp_sndrcvinfo_expr *) malloc(sizeof(struct sctp_sndrcvinfo_expr));
    info->sinfo_stream = (yyvsp[(2) - (17)].expression);
    info->sinfo_ssn = (yyvsp[(4) - (17)].expression);
    info->sinfo_flags = (yyvsp[(6) - (17)].expression);
    info->sinfo_ppid = (yyvsp[(8) - (17)].expression);
    info->sinfo_context = (yyvsp[(10) - (17)].expression);
    info->sinfo_timetolive = (yyvsp[(12) - (17)].expression);
    info->sinfo_tsn = (yyvsp[(14) - (17)].expression);
    info->sinfo_cumtsn = (yyvsp[(16) - (17)].expression);
    info->sinfo_assoc_id = new PacketDrillExpression(EXPR_ELLIPSIS);
    (yyval.expression)->setSndRcvInfo(info);
;}
    break;

  case 349:
#line 2244 "parser.y"
    {
printf("SRS_FLAGS = INTEGER\n");
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("srs_flags out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 350:
#line 2251 "parser.y"
    {
printf("SRS_FLAGS = MYWORD\n");
    (yyval.expression) = new PacketDrillExpression(EXPR_WORD);
    (yyval.expression)->setString((yyvsp[(3) - (3)].string));
;}
    break;

  case 351:
#line 2256 "parser.y"
    {
    (yyval.expression) = (yyvsp[(3) - (3)].expression);
;}
    break;

  case 352:
#line 2262 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_RESET_STREAMS);
    struct sctp_reset_streams_expr *rs = (struct sctp_reset_streams_expr *) malloc(sizeof(struct sctp_reset_streams_expr));
    rs->srs_assoc_id = (yyvsp[(4) - (15)].expression);
    rs->srs_flags = (yyvsp[(6) - (15)].expression);
    if (!is_valid_u16((yyvsp[(10) - (15)].integer))) {
        semantic_error("srs_number_streams out of range");
    }
    rs->srs_number_streams = new_integer_expression((yyvsp[(10) - (15)].integer), "%hu");
    rs->srs_stream_list = (yyvsp[(14) - (15)].expression);
    (yyval.expression)->setResetStreams(rs);
;}
    break;

  case 353:
#line 2274 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_RESET_STREAMS);
    struct sctp_reset_streams_expr *rs = (struct sctp_reset_streams_expr *) malloc(sizeof(struct sctp_reset_streams_expr));
    rs->srs_assoc_id = new PacketDrillExpression(EXPR_ELLIPSIS);
    rs->srs_flags = (yyvsp[(2) - (11)].expression);
    if (!is_valid_u16((yyvsp[(6) - (11)].integer))) {
        semantic_error("srs_number_streams out of range");
    }
    rs->srs_number_streams = new_integer_expression((yyvsp[(6) - (11)].integer), "%hu");
    rs->srs_stream_list = (yyvsp[(10) - (11)].expression);
    (yyval.expression)->setResetStreams(rs);
;}
    break;

  case 354:
#line 2289 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_ADD_STREAMS);
    struct sctp_add_streams_expr *rs = (struct sctp_add_streams_expr *) malloc(sizeof(struct sctp_add_streams_expr));
    rs->sas_assoc_id = (yyvsp[(4) - (13)].expression);
    if (!is_valid_u16((yyvsp[(8) - (13)].integer))) {
        semantic_error("sas_instrms out of range");
    }
    rs->sas_instrms = new_integer_expression((yyvsp[(8) - (13)].integer), "%hu");
    if (!is_valid_u16((yyvsp[(12) - (13)].integer))) {
        semantic_error("sas_outstrms out of range");
    }
    rs->sas_outstrms = new_integer_expression((yyvsp[(12) - (13)].integer), "%hu");
    (yyval.expression)->setAddStreams(rs);
;}
    break;

  case 355:
#line 2303 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_ADD_STREAMS);
    struct sctp_add_streams_expr *rs = (struct sctp_add_streams_expr *) malloc(sizeof(struct sctp_add_streams_expr));
    rs->sas_assoc_id = new PacketDrillExpression(EXPR_ELLIPSIS);
    if (!is_valid_u16((yyvsp[(4) - (9)].integer))) {
        semantic_error("sas_instrms out of range");
    }
    rs->sas_instrms = new_integer_expression((yyvsp[(4) - (9)].integer), "%hu");
    if (!is_valid_u16((yyvsp[(8) - (9)].integer))) {
        semantic_error("sas_outstrms out of range");
    }
    rs->sas_outstrms = new_integer_expression((yyvsp[(8) - (9)].integer), "%hu");
    (yyval.expression)->setAddStreams(rs);
;}
    break;

  case 356:
#line 2321 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_ASSOCVAL);
    struct sctp_assoc_value_expr *assocval = (struct sctp_assoc_value_expr *) malloc(sizeof(struct sctp_assoc_value_expr));
    assocval->assoc_id = (yyvsp[(4) - (9)].expression);
    assocval->assoc_value = (yyvsp[(8) - (9)].expression);
    (yyval.expression)->setAssocval(assocval);
;}
    break;

  case 357:
#line 2328 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_ASSOCVAL);
    struct sctp_assoc_value_expr *assocval = (struct sctp_assoc_value_expr *) malloc(sizeof(struct sctp_assoc_value_expr));
    assocval->assoc_id = new PacketDrillExpression(EXPR_ELLIPSIS);
    assocval->assoc_value = (yyvsp[(4) - (5)].expression);
    (yyval.expression)->setAssocval(assocval);
;}
    break;

  case 358:
#line 2338 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("sack_delay out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 359:
#line 2344 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS);
;}
    break;

  case 360:
#line 2349 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("sack_freq out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 361:
#line 2355 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 362:
#line 2358 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_SACKINFO);
    struct sctp_sack_info_expr *sackinfo = (struct sctp_sack_info_expr *) malloc(sizeof(struct sctp_sack_info_expr));
    sackinfo->sack_assoc_id = (yyvsp[(4) - (9)].expression);
    sackinfo->sack_delay = (yyvsp[(6) - (9)].expression);
    sackinfo->sack_freq = (yyvsp[(8) - (9)].expression);
    (yyval.expression)->setSackinfo(sackinfo);
;}
    break;

  case 363:
#line 2366 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_SACKINFO);
    struct sctp_sack_info_expr *sackinfo = (struct sctp_sack_info_expr *) malloc(sizeof(struct sctp_sack_info_expr));
    sackinfo->sack_assoc_id = new PacketDrillExpression(EXPR_ELLIPSIS);
    sackinfo->sack_delay = (yyvsp[(2) - (5)].expression);
    sackinfo->sack_freq = (yyvsp[(4) - (5)].expression);
    (yyval.expression)->setSackinfo(sackinfo);
;}
    break;

  case 364:
#line 2377 "parser.y"
    {
    (yyval.errno_info) = NULL;
;}
    break;

  case 365:
#line 2380 "parser.y"
    {
    (yyval.errno_info) = (struct errno_spec*)malloc(sizeof(struct errno_spec));
    (yyval.errno_info)->errno_macro = (yyvsp[(1) - (2)].string);
    (yyval.errno_info)->strerror = (yyvsp[(2) - (2)].string);
;}
    break;

  case 366:
#line 2388 "parser.y"
    {
    (yyval.string) = NULL;
;}
    break;

  case 367:
#line 2391 "parser.y"
    {
    (yyval.string) = (yyvsp[(1) - (1)].string);
;}
    break;

  case 368:
#line 2397 "parser.y"
    {
    (yyval.string) = (yyvsp[(2) - (3)].string);
;}
    break;

  case 369:
#line 2403 "parser.y"
    {
    (yyval.string) = (yyvsp[(1) - (1)].string);
;}
    break;

  case 370:
#line 2406 "parser.y"
    {
    asprintf(&((yyval.string)), "%s %s", (yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string));
    free((yyvsp[(1) - (2)].string));
    free((yyvsp[(2) - (2)].string));
;}
    break;


/* Line 1267 of yacc.c.  */
#line 5860 "parser.cc"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }

  yyerror_range[0] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, &yylloc);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[0] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[0] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the look-ahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, &yylloc);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



