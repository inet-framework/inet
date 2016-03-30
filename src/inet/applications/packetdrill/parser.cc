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
     ACK = 260,
     WIN = 261,
     WSCALE = 262,
     MSS = 263,
     NOP = 264,
     TIMESTAMP = 265,
     ECR = 266,
     EOL = 267,
     TCPSACK = 268,
     VAL = 269,
     SACKOK = 270,
     OPTION = 271,
     CHUNK = 272,
     MYDATA = 273,
     MYINIT = 274,
     MYINIT_ACK = 275,
     MYHEARTBEAT = 276,
     MYHEARTBEAT_ACK = 277,
     MYABORT = 278,
     MYSHUTDOWN = 279,
     MYSHUTDOWN_ACK = 280,
     MYERROR = 281,
     MYCOOKIE_ECHO = 282,
     MYCOOKIE_ACK = 283,
     MYSHUTDOWN_COMPLETE = 284,
     HEARTBEAT_INFORMATION = 285,
     CAUSE_INFO = 286,
     MYSACK = 287,
     STATE_COOKIE = 288,
     PARAMETER = 289,
     MYSCTP = 290,
     TYPE = 291,
     FLAGS = 292,
     LEN = 293,
     TAG = 294,
     A_RWND = 295,
     OS = 296,
     IS = 297,
     TSN = 298,
     MYSID = 299,
     SSN = 300,
     PPID = 301,
     CUM_TSN = 302,
     GAPS = 303,
     DUPS = 304,
     SRTO_ASSOC_ID = 305,
     SRTO_INITIAL = 306,
     SRTO_MAX = 307,
     SRTO_MIN = 308,
     SINIT_NUM_OSTREAMS = 309,
     SINIT_MAX_INSTREAMS = 310,
     SINIT_MAX_ATTEMPTS = 311,
     SINIT_MAX_INIT_TIMEO = 312,
     MYSACK_DELAY = 313,
     SACK_FREQ = 314,
     ASSOC_VALUE = 315,
     ASSOC_ID = 316,
     SACK_ASSOC_ID = 317,
     MYFLOAT = 318,
     INTEGER = 319,
     HEX_INTEGER = 320,
     MYWORD = 321,
     MYSTRING = 322
   };
#endif
/* Tokens.  */
#define ELLIPSIS 258
#define UDP 259
#define ACK 260
#define WIN 261
#define WSCALE 262
#define MSS 263
#define NOP 264
#define TIMESTAMP 265
#define ECR 266
#define EOL 267
#define TCPSACK 268
#define VAL 269
#define SACKOK 270
#define OPTION 271
#define CHUNK 272
#define MYDATA 273
#define MYINIT 274
#define MYINIT_ACK 275
#define MYHEARTBEAT 276
#define MYHEARTBEAT_ACK 277
#define MYABORT 278
#define MYSHUTDOWN 279
#define MYSHUTDOWN_ACK 280
#define MYERROR 281
#define MYCOOKIE_ECHO 282
#define MYCOOKIE_ACK 283
#define MYSHUTDOWN_COMPLETE 284
#define HEARTBEAT_INFORMATION 285
#define CAUSE_INFO 286
#define MYSACK 287
#define STATE_COOKIE 288
#define PARAMETER 289
#define MYSCTP 290
#define TYPE 291
#define FLAGS 292
#define LEN 293
#define TAG 294
#define A_RWND 295
#define OS 296
#define IS 297
#define TSN 298
#define MYSID 299
#define SSN 300
#define PPID 301
#define CUM_TSN 302
#define GAPS 303
#define DUPS 304
#define SRTO_ASSOC_ID 305
#define SRTO_INITIAL 306
#define SRTO_MAX 307
#define SRTO_MIN 308
#define SINIT_NUM_OSTREAMS 309
#define SINIT_MAX_INSTREAMS 310
#define SINIT_MAX_ATTEMPTS 311
#define SINIT_MAX_INIT_TIMEO 312
#define MYSACK_DELAY 313
#define SACK_FREQ 314
#define ASSOC_VALUE 315
#define ASSOC_ID 316
#define SACK_ASSOC_ID 317
#define MYFLOAT 318
#define INTEGER 319
#define HEX_INTEGER 320
#define MYWORD 321
#define MYSTRING 322




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

#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

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

/* Bison emits code to call this method when there's a parse-time error.
 * We print the line number and the error message.
 */
static void yyerror(const char *message) {
    fprintf(stderr, "%s:%d: parse error at '%s': %s\n",
        current_script_path, yylineno, yytext, message);
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
#line 213 "parser.y"
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
    struct option_list *option;
    PacketDrillEvent *event;
    PacketDrillPacket *packet;
    struct syscall_spec *syscall;
    PacketDrillStruct *sack_block;
    PacketDrillExpression *expression;
    cQueue *expression_list;
    PacketDrillTcpOption *tcp_option;
    PacketDrillSctpParameter *sctp_parameter;
    cQueue *tcp_options;
    struct errno_spec *errno_info;
    cQueue *sctp_chunk_list;
    cQueue *sctp_parameter_list;
    cQueue *sack_block_list;
    PacketDrillBytes *byte_list;
    uint8 byte;
    PacketDrillSctpChunk *sctp_chunk;
}
/* Line 193 of yacc.c.  */
#line 471 "parser.cc"
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
#line 496 "parser.cc"

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
#   define YYCOPY(To, From, Count)        \
      do                    \
    {                    \
      YYSIZE_T yyi;                \
      for (yyi = 0; yyi < (Count); yyi++)    \
        (To)[yyi] = (From)[yyi];        \
    }                    \
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)                    \
    do                                    \
      {                                    \
    YYSIZE_T yynewbytes;                        \
    YYCOPY (&yyptr->Stack, Stack, yysize);                \
    Stack = &yyptr->Stack;                        \
    yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
    yyptr += yynewbytes / sizeof (*yyptr);                \
      }                                    \
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  11
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   496

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  85
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  92
/* YYNRULES -- Number of rules.  */
#define YYNRULES  201
/* YYNRULES -- Number of states.  */
#define YYNSTATES  466

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   322

#define YYTRANSLATE(YYX)                        \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      71,    72,    69,    68,    74,    81,    80,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    73,     2,
      78,    75,    79,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    76,     2,    77,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    83,    82,    84,    70,     2,     2,     2,
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
      65,    66,    67
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    13,    16,    18,    20,
      24,    30,    32,    34,    36,    38,    40,    42,    44,    51,
      57,    62,    64,    68,    70,    72,    74,    76,    78,    80,
      82,    84,    86,    88,    90,    92,    96,   100,   104,   108,
     112,   116,   122,   128,   130,   134,   136,   138,   142,   146,
     150,   154,   158,   162,   166,   170,   174,   178,   182,   186,
     190,   194,   198,   202,   206,   210,   214,   218,   222,   226,
     230,   234,   238,   242,   246,   250,   254,   258,   262,   266,
     272,   278,   282,   288,   294,   309,   325,   341,   354,   361,
     368,   373,   380,   385,   394,   399,   404,   407,   410,   412,
     416,   418,   420,   425,   432,   437,   448,   459,   461,   463,
     465,   467,   469,   472,   474,   481,   482,   485,   486,   489,
     490,   494,   498,   500,   504,   506,   508,   511,   514,   516,
     519,   525,   527,   530,   531,   533,   537,   541,   542,   544,
     548,   552,   556,   564,   565,   568,   570,   573,   577,   579,
     583,   585,   587,   589,   591,   593,   596,   598,   600,   602,
     604,   606,   608,   610,   612,   616,   619,   623,   627,   631,
     635,   639,   643,   647,   649,   651,   653,   665,   673,   677,
     681,   685,   689,   693,   697,   701,   705,   715,   725,   731,
     735,   739,   743,   747,   757,   763,   764,   767,   768,   770,
     774,   776
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      86,     0,    -1,    87,    -1,    88,    -1,    87,    88,    -1,
      89,    91,    -1,    68,    90,    -1,    90,    -1,    69,    -1,
      90,    70,    90,    -1,    68,    90,    70,    68,    90,    -1,
      63,    -1,    64,    -1,    92,    -1,   149,    -1,    93,    -1,
      94,    -1,    95,    -1,   134,   136,   137,   138,   139,   140,
      -1,   134,     4,    71,    64,    72,    -1,   134,    35,    73,
      96,    -1,    97,    -1,    96,    74,    97,    -1,   117,    -1,
     118,    -1,   119,    -1,   120,    -1,   121,    -1,   122,    -1,
     123,    -1,   124,    -1,   125,    -1,   126,    -1,   127,    -1,
     128,    -1,    37,    75,     3,    -1,    37,    75,    65,    -1,
      37,    75,    64,    -1,    38,    75,     3,    -1,    38,    75,
      64,    -1,    14,    75,     3,    -1,    14,    75,    76,     3,
      77,    -1,    14,    75,    76,   101,    77,    -1,   102,    -1,
     101,    74,   102,    -1,    65,    -1,    64,    -1,    37,    75,
       3,    -1,    37,    75,    65,    -1,    37,    75,    64,    -1,
      37,    75,    66,    -1,    37,    75,     3,    -1,    37,    75,
      65,    -1,    37,    75,    64,    -1,    37,    75,    66,    -1,
      37,    75,     3,    -1,    37,    75,    65,    -1,    37,    75,
      64,    -1,    37,    75,    66,    -1,    39,    75,     3,    -1,
      39,    75,    64,    -1,    40,    75,     3,    -1,    40,    75,
      64,    -1,    41,    75,     3,    -1,    41,    75,    64,    -1,
      42,    75,     3,    -1,    42,    75,    64,    -1,    43,    75,
       3,    -1,    43,    75,    64,    -1,    44,    75,     3,    -1,
      44,    75,    64,    -1,    45,    75,     3,    -1,    45,    75,
      64,    -1,    46,    75,     3,    -1,    46,    75,    64,    -1,
      46,    75,    65,    -1,    47,    75,     3,    -1,    47,    75,
      64,    -1,    48,    75,     3,    -1,    48,    75,    76,     3,
      77,    -1,    48,    75,    76,   144,    77,    -1,    49,    75,
       3,    -1,    49,    75,    76,     3,    77,    -1,    49,    75,
      76,   146,    77,    -1,    18,    76,   103,    74,    99,    74,
     110,    74,   111,    74,   112,    74,   113,    77,    -1,    19,
      76,    98,    74,   106,    74,   107,    74,   108,    74,   109,
      74,   110,   129,    77,    -1,    20,    76,    98,    74,   106,
      74,   107,    74,   108,    74,   109,    74,   110,   129,    77,
      -1,    32,    76,    98,    74,   114,    74,   107,    74,   115,
      74,   116,    77,    -1,    21,    76,    98,    74,   132,    77,
      -1,    22,    76,    98,    74,   132,    77,    -1,    23,    76,
     104,    77,    -1,    24,    76,    98,    74,   114,    77,    -1,
      25,    76,    98,    77,    -1,    27,    76,    98,    74,    99,
      74,   100,    77,    -1,    28,    76,    98,    77,    -1,    29,
      76,   105,    77,    -1,    74,     3,    -1,    74,   130,    -1,
     131,    -1,   130,    74,   131,    -1,   132,    -1,   133,    -1,
      30,    76,     3,    77,    -1,    30,    76,    99,    74,   100,
      77,    -1,    33,    76,     3,    77,    -1,    33,    76,    38,
      75,     3,    74,    14,    75,     3,    77,    -1,    33,    76,
      38,    75,    64,    74,    14,    75,     3,    77,    -1,   135,
      -1,    78,    -1,    79,    -1,    66,    -1,    80,    -1,    66,
      80,    -1,    81,    -1,    64,    73,    64,    71,    64,    72,
      -1,    -1,     5,    64,    -1,    -1,     6,    64,    -1,    -1,
      78,   141,    79,    -1,    78,     3,    79,    -1,   142,    -1,
     141,    74,   142,    -1,     9,    -1,    12,    -1,     8,    64,
      -1,     7,    64,    -1,    15,    -1,    13,   143,    -1,    10,
      14,    64,    11,    64,    -1,   148,    -1,   143,   148,    -1,
      -1,   145,    -1,   144,    74,   145,    -1,    64,    73,    64,
      -1,    -1,   147,    -1,   146,    74,   147,    -1,    64,    73,
      64,    -1,    64,    73,    64,    -1,   150,   151,   152,    75,
     154,   173,   174,    -1,    -1,     3,    90,    -1,    66,    -1,
      71,    72,    -1,    71,   153,    72,    -1,   154,    -1,   153,
      74,   154,    -1,     3,    -1,   155,    -1,   156,    -1,    66,
      -1,    67,    -1,    67,     3,    -1,   157,    -1,   158,    -1,
     168,    -1,   169,    -1,   163,    -1,   172,    -1,    64,    -1,
      65,    -1,   154,    82,   154,    -1,    76,    77,    -1,    76,
     153,    77,    -1,    51,    75,    64,    -1,    51,    75,     3,
      -1,    52,    75,    64,    -1,    52,    75,     3,    -1,    53,
      75,    64,    -1,    53,    75,     3,    -1,    64,    -1,    66,
      -1,     3,    -1,    83,    50,    75,   162,    74,   159,    74,
     160,    74,   161,    84,    -1,    83,   159,    74,   160,    74,
     161,    84,    -1,    54,    75,    64,    -1,    54,    75,     3,
      -1,    55,    75,    64,    -1,    55,    75,     3,    -1,    56,
      75,    64,    -1,    56,    75,     3,    -1,    57,    75,    64,
      -1,    57,    75,     3,    -1,    83,   164,    74,   165,    74,
     166,    74,   167,    84,    -1,    83,    61,    75,   162,    74,
      60,    75,   154,    84,    -1,    83,    60,    75,   154,    84,
      -1,    58,    75,    64,    -1,    58,    75,     3,    -1,    59,
      75,    64,    -1,    59,    75,     3,    -1,    83,    62,    75,
     162,    74,   170,    74,   171,    84,    -1,    83,   170,    74,
     171,    84,    -1,    -1,    66,   175,    -1,    -1,   175,    -1,
      71,   176,    72,    -1,    66,    -1,   176,    66,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   312,   312,   319,   323,   330,   360,   366,   372,   377,
     384,   394,   400,   409,   412,   419,   422,   425,   431,   457,
     476,   492,   494,   500,   501,   502,   503,   504,   505,   506,
     507,   508,   509,   510,   511,   516,   517,   523,   532,   533,
     542,   543,   544,   548,   549,   554,   560,   569,   570,   576,
     582,   627,   628,   634,   640,   664,   665,   671,   677,   702,
     703,   712,   713,   722,   723,   732,   733,   742,   743,   752,
     753,   762,   763,   773,   774,   780,   789,   790,   799,   800,
     801,   806,   807,   808,   813,   822,   827,   832,   837,   843,
     849,   854,   859,   864,   880,   885,   890,   891,   895,   899,
     907,   908,   913,   916,   933,   936,   939,   949,   957,   961,
     968,   971,   974,   981,   987,  1007,  1010,  1019,  1022,  1031,
    1034,  1037,  1044,  1048,  1056,  1059,  1062,  1069,  1076,  1079,
    1083,  1100,  1104,  1110,  1111,  1115,  1121,  1133,  1134,  1138,
    1144,  1156,  1175,  1187,  1190,  1196,  1203,  1206,  1212,  1216,
    1223,  1226,  1228,  1231,  1235,  1240,  1245,  1248,  1251,  1254,
    1257,  1260,  1268,  1274,  1280,  1291,  1295,  1302,  1308,  1314,
    1317,  1321,  1324,  1328,  1331,  1335,  1339,  1348,  1360,  1366,
    1370,  1376,  1380,  1386,  1390,  1396,  1400,  1413,  1420,  1430,
    1436,  1441,  1447,  1450,  1458,  1469,  1472,  1480,  1483,  1489,
    1495,  1498
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ELLIPSIS", "UDP", "ACK", "WIN",
  "WSCALE", "MSS", "NOP", "TIMESTAMP", "ECR", "EOL", "TCPSACK", "VAL",
  "SACKOK", "OPTION", "CHUNK", "MYDATA", "MYINIT", "MYINIT_ACK",
  "MYHEARTBEAT", "MYHEARTBEAT_ACK", "MYABORT", "MYSHUTDOWN",
  "MYSHUTDOWN_ACK", "MYERROR", "MYCOOKIE_ECHO", "MYCOOKIE_ACK",
  "MYSHUTDOWN_COMPLETE", "HEARTBEAT_INFORMATION", "CAUSE_INFO", "MYSACK",
  "STATE_COOKIE", "PARAMETER", "MYSCTP", "TYPE", "FLAGS", "LEN", "TAG",
  "A_RWND", "OS", "IS", "TSN", "MYSID", "SSN", "PPID", "CUM_TSN", "GAPS",
  "DUPS", "SRTO_ASSOC_ID", "SRTO_INITIAL", "SRTO_MAX", "SRTO_MIN",
  "SINIT_NUM_OSTREAMS", "SINIT_MAX_INSTREAMS", "SINIT_MAX_ATTEMPTS",
  "SINIT_MAX_INIT_TIMEO", "MYSACK_DELAY", "SACK_FREQ", "ASSOC_VALUE",
  "ASSOC_ID", "SACK_ASSOC_ID", "MYFLOAT", "INTEGER", "HEX_INTEGER",
  "MYWORD", "MYSTRING", "'+'", "'*'", "'~'", "'('", "')'", "':'", "','",
  "'='", "'['", "']'", "'<'", "'>'", "'.'", "'-'", "'|'", "'{'", "'}'",
  "$accept", "script", "events", "event", "event_time", "time", "action",
  "packet_spec", "tcp_packet_spec", "udp_packet_spec", "sctp_packet_spec",
  "sctp_chunk_list", "sctp_chunk", "opt_flags", "opt_len", "opt_val",
  "byte_list", "byte", "opt_data_flags", "opt_abort_flags",
  "opt_shutdown_complete_flags", "opt_tag", "opt_a_rwnd", "opt_os",
  "opt_is", "opt_tsn", "opt_sid", "opt_ssn", "opt_ppid", "opt_cum_tsn",
  "opt_gaps", "opt_dups", "sctp_data_chunk_spec", "sctp_init_chunk_spec",
  "sctp_init_ack_chunk_spec", "sctp_sack_chunk_spec",
  "sctp_heartbeat_chunk_spec", "sctp_heartbeat_ack_chunk_spec",
  "sctp_abort_chunk_spec", "sctp_shutdown_chunk_spec",
  "sctp_shutdown_ack_chunk_spec", "sctp_cookie_echo_chunk_spec",
  "sctp_cookie_ack_chunk_spec", "sctp_shutdown_complete_chunk_spec",
  "opt_parameter_list", "sctp_parameter_list", "sctp_parameter",
  "sctp_heartbeat_information_parameter", "sctp_state_cookie_parameter",
  "packet_prefix", "direction", "flags", "seq", "opt_ack", "opt_window",
  "opt_tcp_options", "tcp_option_list", "tcp_option", "sack_block_list",
  "gap_list", "gap", "dup_list", "dup", "sack_block", "syscall_spec",
  "opt_end_time", "function_name", "function_arguments", "expression_list",
  "expression", "decimal_integer", "hex_integer", "binary_expression",
  "array", "srto_initial", "srto_max", "srto_min", "sctp_assoc_id",
  "sctp_rtoinfo", "sinit_num_ostreams", "sinit_max_instreams",
  "sinit_max_attempts", "sinit_max_init_timeo", "sctp_initmsg",
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
     315,   316,   317,   318,   319,   320,   321,   322,    43,    42,
     126,    40,    41,    58,    44,    61,    91,    93,    60,    62,
      46,    45,   124,   123,   125
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    85,    86,    87,    87,    88,    89,    89,    89,    89,
      89,    90,    90,    91,    91,    92,    92,    92,    93,    94,
      95,    96,    96,    97,    97,    97,    97,    97,    97,    97,
      97,    97,    97,    97,    97,    98,    98,    98,    99,    99,
     100,   100,   100,   101,   101,   102,   102,   103,   103,   103,
     103,   104,   104,   104,   104,   105,   105,   105,   105,   106,
     106,   107,   107,   108,   108,   109,   109,   110,   110,   111,
     111,   112,   112,   113,   113,   113,   114,   114,   115,   115,
     115,   116,   116,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   129,   130,   130,
     131,   131,   132,   132,   133,   133,   133,   134,   135,   135,
     136,   136,   136,   136,   137,   138,   138,   139,   139,   140,
     140,   140,   141,   141,   142,   142,   142,   142,   142,   142,
     142,   143,   143,   144,   144,   144,   145,   146,   146,   146,
     147,   148,   149,   150,   150,   151,   152,   152,   153,   153,
     154,   154,   154,   154,   154,   154,   154,   154,   154,   154,
     154,   154,   155,   156,   157,   158,   158,   159,   159,   160,
     160,   161,   161,   162,   162,   162,   163,   163,   164,   164,
     165,   165,   166,   166,   167,   167,   168,   169,   169,   170,
     170,   171,   171,   172,   172,   173,   173,   174,   174,   175,
     176,   176
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     2,     2,     2,     1,     1,     3,
       5,     1,     1,     1,     1,     1,     1,     1,     6,     5,
       4,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     3,     3,     3,     3,
       3,     5,     5,     1,     3,     1,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     5,
       5,     3,     5,     5,    14,    15,    15,    12,     6,     6,
       4,     6,     4,     8,     4,     4,     2,     2,     1,     3,
       1,     1,     4,     6,     4,    10,    10,     1,     1,     1,
       1,     1,     2,     1,     6,     0,     2,     0,     2,     0,
       3,     3,     1,     3,     1,     1,     2,     2,     1,     2,
       5,     1,     2,     0,     1,     3,     3,     0,     1,     3,
       3,     3,     7,     0,     2,     1,     2,     3,     1,     3,
       1,     1,     1,     1,     1,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     2,     3,     3,     3,     3,
       3,     3,     3,     1,     1,     1,    11,     7,     3,     3,
       3,     3,     3,     3,     3,     3,     9,     9,     5,     3,
       3,     3,     3,     9,     5,     0,     2,     0,     1,     3,
       1,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    11,    12,     0,     8,     0,     2,     3,   143,     7,
       6,     1,     4,     0,   108,   109,     5,    13,    15,    16,
      17,     0,   107,    14,     0,     0,     0,   144,     0,     0,
     110,   111,   113,     0,   145,     0,     9,     0,     0,     0,
     112,     0,   115,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    20,
      21,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,     0,     0,   117,   150,   162,   163,   153,
     154,   146,     0,     0,     0,   148,   151,   152,   156,   157,
     160,   158,   159,   161,     0,    19,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     116,     0,   119,   155,   165,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   147,     0,     0,   195,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,     0,   118,     0,
      18,   166,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   149,   164,     0,   197,     0,     0,     0,     0,
       0,     0,     0,     0,    90,     0,    92,     0,    94,     0,
      95,     0,     0,     0,     0,     0,   124,     0,   125,     0,
     128,     0,   122,   175,   173,   174,     0,   168,   167,   179,
     178,   190,   189,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   196,   142,   198,    47,    49,    48,    50,
       0,     0,    35,    37,    36,     0,     0,     0,     0,     0,
       0,    51,    53,    52,    54,     0,     0,     0,    55,    57,
      56,    58,     0,   114,   121,   127,   126,     0,     0,   129,
     131,     0,   120,     0,   188,     0,     0,     0,     0,     0,
       0,     0,   194,   200,     0,     0,     0,     0,     0,     0,
       0,    88,    89,     0,    91,     0,     0,     0,     0,   132,
     123,     0,     0,     0,   170,   169,     0,     0,   181,   180,
       0,     0,   192,   191,   201,   199,    38,    39,     0,     0,
      59,    60,     0,     0,     0,     0,     0,    76,    77,     0,
       0,     0,     0,   141,     0,     0,     0,     0,   177,     0,
       0,     0,     0,     0,     0,     0,   102,     0,     0,    93,
       0,   130,     0,     0,     0,   172,   171,   183,   182,     0,
       0,    67,    68,     0,     0,    61,    62,     0,     0,     0,
       0,    40,     0,     0,     0,     0,   187,   193,     0,   186,
       0,     0,     0,     0,     0,   103,     0,    46,    45,     0,
      43,     0,     0,     0,   185,   184,    69,    70,     0,     0,
      63,    64,     0,     0,     0,    41,     0,    42,    78,   133,
       0,     0,   176,     0,     0,     0,     0,     0,    44,     0,
       0,     0,   134,     0,    87,    71,    72,     0,     0,    65,
      66,     0,     0,    79,     0,     0,    80,    81,   137,     0,
      84,     0,     0,     0,   136,   135,     0,     0,     0,   138,
      73,    74,    75,    96,     0,    97,    98,   100,   101,    85,
      86,    82,     0,     0,    83,     0,     0,   140,   139,     0,
       0,    99,   104,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   105,   106
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     5,     6,     7,     8,     9,    16,    17,    18,    19,
      20,    59,    60,   133,   221,   310,   369,   370,   131,   138,
     144,   226,   303,   348,   383,   299,   344,   379,   408,   236,
     354,   391,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,   422,   435,   436,   437,   438,    21,
      22,    33,    42,    75,   112,   150,   191,   192,   249,   401,
     402,   428,   429,   250,    23,    24,    35,    44,    84,    85,
      86,    87,    88,    89,   123,   207,   287,   196,    90,   124,
     209,   291,   340,    91,    92,   125,   211,    93,   165,   214,
     213,   264
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -256
static const yytype_int16 yypact[] =
{
     -45,  -256,  -256,   141,  -256,    28,   -45,  -256,     5,   -66,
     -32,  -256,  -256,   141,  -256,  -256,  -256,  -256,  -256,  -256,
    -256,    11,  -256,  -256,   -23,   141,   -18,  -256,    68,    85,
      84,  -256,  -256,   103,  -256,    99,  -256,   141,   118,   113,
    -256,   129,   172,    -1,   135,  -256,   143,   137,   138,   144,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   145,
    -256,  -256,  -256,  -256,  -256,  -256,  -256,  -256,  -256,  -256,
    -256,  -256,  -256,   131,   167,   212,  -256,  -256,  -256,  -256,
     229,  -256,     3,   111,    72,   155,  -256,  -256,  -256,  -256,
    -256,  -256,  -256,  -256,    23,  -256,   196,   197,   197,   197,
     197,   198,   197,   197,   197,   197,   199,   197,   113,   168,
    -256,   174,   162,  -256,  -256,   -47,   166,   169,   170,   171,
     173,   175,   176,   178,   179,   180,  -256,    23,    23,   -57,
     181,   183,   185,   187,   188,   189,   190,   191,   165,   193,
     182,   194,   192,   195,   200,   201,  -256,   207,  -256,   140,
    -256,  -256,    29,    17,    41,    48,    23,    29,    29,   203,
     210,   213,   155,   155,   202,   202,     8,   205,    39,   208,
     208,   219,   219,    32,  -256,   211,  -256,   205,  -256,    36,
    -256,   211,   204,   206,   214,   215,  -256,   260,  -256,   216,
    -256,   -43,  -256,  -256,  -256,  -256,   209,  -256,  -256,  -256,
    -256,  -256,  -256,   116,   217,   218,   220,   222,   223,   225,
     226,   221,   224,  -256,  -256,  -256,  -256,  -256,  -256,  -256,
     227,   230,  -256,  -256,  -256,   228,   232,   233,   234,   231,
     235,  -256,  -256,  -256,  -256,   236,   237,   239,  -256,  -256,
    -256,  -256,   241,  -256,  -256,  -256,  -256,   245,   243,   216,
    -256,   184,  -256,   238,  -256,   240,   259,    49,   244,    50,
     262,    51,  -256,  -256,   -50,    52,   250,    53,   242,   242,
       7,  -256,  -256,    54,  -256,   267,   242,   273,   255,  -256,
    -256,   246,   247,   249,  -256,  -256,   251,   248,  -256,  -256,
     252,   254,  -256,  -256,  -256,  -256,  -256,  -256,   256,   261,
    -256,  -256,   258,   263,   264,   253,   265,  -256,  -256,   266,
     257,   268,   272,  -256,   203,    23,   213,    55,  -256,    56,
     283,    57,   277,    58,   284,   284,  -256,   267,     0,  -256,
     276,  -256,   269,   119,   270,  -256,  -256,  -256,  -256,   271,
     274,  -256,  -256,   275,   278,  -256,  -256,   280,   279,   282,
     285,  -256,    44,   286,   289,   244,  -256,  -256,   104,  -256,
     120,   299,   121,   287,   287,  -256,   288,  -256,  -256,   -37,
    -256,     2,   296,   290,  -256,  -256,  -256,  -256,   291,   293,
    -256,  -256,   294,   297,   298,  -256,   142,  -256,  -256,   122,
     295,   300,  -256,   123,   301,   124,   250,   250,  -256,   302,
     303,    77,  -256,     9,  -256,  -256,  -256,   305,   304,  -256,
    -256,   308,   308,  -256,   309,   311,  -256,  -256,   125,    46,
    -256,   127,   306,   307,  -256,  -256,   310,   312,    82,  -256,
    -256,  -256,  -256,  -256,   281,   314,  -256,  -256,  -256,  -256,
    -256,  -256,   322,   325,  -256,    10,   133,  -256,  -256,   313,
     316,  -256,  -256,   126,   318,   319,   334,   335,   320,   321,
     348,   356,   317,   323,  -256,  -256
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -256,  -256,  -256,   354,  -256,     4,  -256,  -256,  -256,  -256,
    -256,  -256,   186,    76,  -176,   -41,  -256,   -99,  -256,  -256,
    -256,   292,  -255,    43,   -76,  -188,  -256,  -256,  -256,   315,
    -256,  -256,  -256,  -256,  -256,  -256,  -256,  -256,  -256,  -256,
    -256,  -256,  -256,  -256,   -48,  -256,   -68,    40,  -256,  -256,
    -256,  -256,  -256,  -256,  -256,  -256,  -256,   156,  -256,  -256,
     -17,  -256,   -46,   157,  -256,  -256,  -256,  -256,   326,   -94,
    -256,  -256,  -256,  -256,   158,    87,    47,    59,  -256,  -256,
    -256,  -256,  -256,  -256,  -256,   159,    83,  -256,  -256,  -256,
     324,  -256
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
     129,   237,    76,   351,    25,   388,    76,    10,    13,   164,
     305,   216,   417,   449,   304,    28,   294,    27,     1,     2,
     197,   311,   295,     3,     4,   128,    76,   127,    11,    36,
     151,   251,   193,   162,   163,   231,   252,   386,    26,   238,
     387,    45,   222,    34,   199,   220,    29,   366,   450,   430,
      37,   201,   284,   288,   292,   296,   300,   307,   335,   337,
     341,   345,   203,    77,    78,    79,    80,    77,    78,    79,
      80,    81,   217,   218,   219,    82,   352,    30,   389,    82,
     114,   198,    83,    14,    15,   418,    83,    77,    78,    79,
      80,    31,    32,   194,   306,   195,   232,   233,   234,    82,
     239,   240,   241,   223,   224,   200,    83,   374,   367,   368,
     431,   432,   202,   285,   289,   293,   297,   301,   308,   336,
     338,   342,   346,   376,   380,   399,   405,   409,   426,   454,
     433,    47,    48,    49,    50,    51,    52,    53,    54,    38,
      55,    56,    57,   183,   126,    58,   127,   184,   185,   186,
     187,   415,   188,   189,   416,   190,   443,   228,    39,   444,
     434,   116,   117,   228,    40,   118,   434,    41,   375,   119,
      43,   120,   121,   122,   134,   135,   136,    74,   139,   140,
     141,   142,    46,   145,   377,   381,   400,   406,   410,   427,
     455,   184,   185,   186,   187,   109,   188,   189,   128,   190,
     254,   128,    73,   356,     1,     2,   367,   368,   411,   412,
      94,   229,   230,    96,    97,    95,   204,   205,   111,   108,
      98,   333,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   110,   113,   130,   132,   137,   143,   128,   148,   147,
     149,   152,   174,   220,   153,   154,   155,   225,   156,   228,
     157,   158,   159,   160,   161,   206,   166,   167,   235,   176,
     168,   169,   170,   171,   172,   208,   173,   175,   177,   178,
     179,   182,   210,   212,   247,   181,   243,   180,   245,   246,
     248,   309,   302,   253,   312,   244,   350,   398,   384,   117,
     263,   255,   256,   298,   146,   257,   258,   286,   259,   260,
     282,   261,   265,   267,   266,   262,   268,   269,   271,   277,
     270,   273,   272,   275,   274,   276,   278,   119,   290,   313,
     314,   343,   315,   316,   353,   347,   317,   319,   320,   382,
     326,   321,   318,   323,   329,   322,   331,   324,   325,   327,
     339,   328,   330,   355,   378,   390,   358,   407,   458,   459,
     360,   462,   361,   363,   357,   362,   364,   445,   359,   463,
      12,   371,   365,   372,   423,   385,   393,   394,   349,   395,
     403,   396,   397,   424,   392,   400,   414,   404,   451,   413,
     419,   420,   421,   439,   440,   442,   447,   441,   446,   427,
     452,   453,   456,   457,   464,   460,   461,   448,   425,   334,
     465,   332,   373,     0,     0,     0,   279,   280,   115,     0,
       0,   281,     0,     0,     0,   283,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   227,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   215,
       0,     0,     0,     0,     0,     0,   242
};

static const yytype_int16 yycheck[] =
{
      94,   177,     3,     3,    70,     3,     3,     3,     3,    66,
       3,     3,     3,     3,   269,     4,    66,    13,    63,    64,
       3,   276,    72,    68,    69,    82,     3,    74,     0,    25,
      77,    74,     3,   127,   128,     3,    79,    74,    70,     3,
      77,    37,     3,    66,     3,    38,    35,     3,    38,     3,
      68,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,   156,    64,    65,    66,    67,    64,    65,    66,
      67,    72,    64,    65,    66,    76,    76,    66,    76,    76,
      77,    64,    83,    78,    79,    76,    83,    64,    65,    66,
      67,    80,    81,    64,   270,    66,    64,    65,    66,    76,
      64,    65,    66,    64,    65,    64,    83,     3,    64,    65,
      64,    65,    64,    64,    64,    64,    64,    64,    64,    64,
      64,    64,    64,     3,     3,     3,     3,     3,     3,     3,
       3,    18,    19,    20,    21,    22,    23,    24,    25,    71,
      27,    28,    29,     3,    72,    32,    74,     7,     8,     9,
      10,    74,    12,    13,    77,    15,    74,    30,    73,    77,
      33,    50,    51,    30,    80,    54,    33,    64,    64,    58,
      71,    60,    61,    62,    98,    99,   100,     5,   102,   103,
     104,   105,    64,   107,    64,    64,    64,    64,    64,    64,
      64,     7,     8,     9,    10,    64,    12,    13,    82,    15,
      84,    82,    73,    84,    63,    64,    64,    65,   396,   397,
      75,   171,   172,    76,    76,    72,   157,   158,     6,    74,
      76,   315,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    64,     3,    37,    37,    37,    37,    82,    64,    71,
      78,    75,    77,    38,    75,    75,    75,    39,    75,    30,
      75,    75,    74,    74,    74,    52,    75,    74,    47,    77,
      75,    74,    74,    74,    74,    55,    75,    74,    74,    77,
      75,    64,    59,    71,    14,    74,    72,    77,    64,    64,
      64,    14,    40,    74,    11,    79,   327,   386,   364,    51,
      66,    74,    74,    43,   108,    75,    74,    53,    75,    74,
      60,    75,    75,    75,    74,    84,    74,    74,    77,    64,
      76,    75,    77,    74,    77,    74,    73,    58,    56,    64,
      74,    44,    75,    74,    48,    41,    75,    75,    74,    42,
      77,    75,    84,    75,    77,    74,    64,    74,    74,    74,
      57,    75,    74,    74,    45,    49,    75,    46,    14,    14,
      75,     3,    74,    74,    84,    75,    74,    76,    84,     3,
       6,    75,    77,    74,   412,    77,    75,    74,   325,    75,
      75,    74,    74,    64,    84,    64,    73,    77,   446,    77,
      75,    77,    74,    77,    77,    73,    64,    77,    74,    64,
      77,    75,    74,    74,    77,    75,    75,   443,   415,   316,
      77,   314,   355,    -1,    -1,    -1,   249,   251,    82,    -1,
      -1,   253,    -1,    -1,    -1,   256,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   165,
      -1,    -1,    -1,    -1,    -1,    -1,   181
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    63,    64,    68,    69,    86,    87,    88,    89,    90,
      90,     0,    88,     3,    78,    79,    91,    92,    93,    94,
      95,   134,   135,   149,   150,    70,    70,    90,     4,    35,
      66,    80,    81,   136,    66,   151,    90,    68,    71,    73,
      80,    64,   137,    71,   152,    90,    64,    18,    19,    20,
      21,    22,    23,    24,    25,    27,    28,    29,    32,    96,
      97,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,    73,     5,   138,     3,    64,    65,    66,
      67,    72,    76,    83,   153,   154,   155,   156,   157,   158,
     163,   168,   169,   172,    75,    72,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    74,    64,
      64,     6,   139,     3,    77,   153,    50,    51,    54,    58,
      60,    61,    62,   159,   164,   170,    72,    74,    82,   154,
      37,   103,    37,    98,    98,    98,    98,    37,   104,    98,
      98,    98,    98,    37,   105,    98,    97,    71,    64,    78,
     140,    77,    75,    75,    75,    75,    75,    75,    75,    74,
      74,    74,   154,   154,    66,   173,    75,    74,    75,    74,
      74,    74,    74,    75,    77,    74,    77,    74,    77,    75,
      77,    74,    64,     3,     7,     8,     9,    10,    12,    13,
      15,   141,   142,     3,    64,    66,   162,     3,    64,     3,
      64,     3,    64,   154,   162,   162,    52,   160,    55,   165,
      59,   171,    71,   175,   174,   175,     3,    64,    65,    66,
      38,    99,     3,    64,    65,    39,   106,   106,    30,   132,
     132,     3,    64,    65,    66,    47,   114,    99,     3,    64,
      65,    66,   114,    72,    79,    64,    64,    14,    64,   143,
     148,    74,    79,    74,    84,    74,    74,    75,    74,    75,
      74,    75,    84,    66,   176,    75,    74,    75,    74,    74,
      76,    77,    77,    75,    77,    74,    74,    64,    73,   148,
     142,   159,    60,   170,     3,    64,    53,   161,     3,    64,
      56,   166,     3,    64,    66,    72,     3,    64,    43,   110,
       3,    64,    40,   107,   107,     3,    99,     3,    64,    14,
     100,   107,    11,    64,    74,    75,    74,    75,    84,    75,
      74,    75,    74,    75,    74,    74,    77,    74,    75,    77,
      74,    64,   160,   154,   171,     3,    64,     3,    64,    57,
     167,     3,    64,    44,   111,     3,    64,    41,   108,   108,
     100,     3,    76,    48,   115,    74,    84,    84,    75,    84,
      75,    74,    75,    74,    74,    77,     3,    64,    65,   101,
     102,    75,    74,   161,     3,    64,     3,    64,    45,   112,
       3,    64,    42,   109,   109,    77,    74,    77,     3,    76,
      49,   116,    84,    75,    74,    75,    74,    74,   102,     3,
      64,   144,   145,    75,    77,     3,    64,    46,   113,     3,
      64,   110,   110,    77,    73,    74,    77,     3,    76,    75,
      77,    74,   129,   129,    64,   145,     3,    64,   146,   147,
       3,    64,    65,     3,    33,   130,   131,   132,   133,    77,
      77,    77,    73,    74,    77,    76,    74,    64,   147,     3,
      38,   131,    77,    75,     3,    64,    74,    74,    14,    14,
      75,    75,     3,     3,    77,    77
};

#define yyerrok        (yyerrstatus = 0)
#define yyclearin    (yychar = YYEMPTY)
#define YYEMPTY        (-2)
#define YYEOF        0

#define YYACCEPT    goto yyacceptlab
#define YYABORT        goto yyabortlab
#define YYERROR        goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL        goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                    \
do                                \
  if (yychar == YYEMPTY && yylen == 1)                \
    {                                \
      yychar = (Token);                        \
      yylval = (Value);                        \
      yytoken = YYTRANSLATE (yychar);                \
      YYPOPSTACK (1);                        \
      goto yybackup;                        \
    }                                \
  else                                \
    {                                \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                            \
    }                                \
while (YYID (0))


#define YYTERROR    1
#define YYERRCODE    256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                \
    do                                    \
      if (YYID (N))                                                    \
    {                                \
      (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;    \
      (Current).first_column = YYRHSLOC (Rhs, 1).first_column;    \
      (Current).last_line    = YYRHSLOC (Rhs, N).last_line;        \
      (Current).last_column  = YYRHSLOC (Rhs, N).last_column;    \
    }                                \
      else                                \
    {                                \
      (Current).first_line   = (Current).last_line   =        \
        YYRHSLOC (Rhs, 0).last_line;                \
      (Current).first_column = (Current).last_column =        \
        YYRHSLOC (Rhs, 0).last_column;                \
    }                                \
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)            \
     fprintf (File, "%d.%d-%d.%d",            \
          (Loc).first_line, (Loc).first_column,    \
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

# define YYDPRINTF(Args)            \
do {                        \
  if (yydebug)                    \
    YYFPRINTF Args;                \
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)              \
do {                                      \
  if (yydebug)                                  \
    {                                      \
      YYFPRINTF (stderr, "%s ", Title);                      \
      yy_symbol_print (stderr,                          \
          Type, Value, Location); \
      YYFPRINTF (stderr, "\n");                          \
    }                                      \
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

# define YY_STACK_PRINT(Bottom, Top)                \
do {                                \
  if (yydebug)                            \
    yy_stack_print ((Bottom), (Top));                \
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
               , &(yylsp[(yyi + 1) - (yynrhs)])               );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)        \
do {                    \
  if (yydebug)                \
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
#ifndef    YYINITDEPTH
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
  yychar = YYEMPTY;        /* Cause a token to be read.  */

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
#line 312 "parser.y"
    {
    (yyval.string) = NULL;    /* The parser output is in out_script */
;}
    break;

  case 3:
#line 319 "parser.y"
    {
    out_script->addEvent((yyvsp[(1) - (1)].event));    /* save pointer to event list as output of parser */
    (yyval.event) = (yyvsp[(1) - (1)].event);    /* return the tail so that we can append to it */
;}
    break;

  case 4:
#line 323 "parser.y"
    {
    out_script->addEvent((yyvsp[(2) - (2)].event));
    (yyval.event) = (yyvsp[(2) - (2)].event);    /* return the tail so that we can append to it */
;}
    break;

  case 5:
#line 330 "parser.y"
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
            printf("Semantic error: time range is backwards\n");
    }
    if ((yyval.event)->getTimeType() == ANY_TIME &&  ((yyval.event)->getType() != PACKET_EVENT ||
        ((yyval.event)->getPacket())->getDirection() != DIRECTION_OUTBOUND)) {
        yylineno = (yyval.event)->getLineNumber();
        printf("Semantic error: event time <star> can only be used with outbound packets\n");
    } else if (((yyval.event)->getTimeType() == ABSOLUTE_RANGE_TIME ||
        (yyval.event)->getTimeType() == RELATIVE_RANGE_TIME) &&
        ((yyval.event)->getType() != PACKET_EVENT ||
        ((yyval.event)->getPacket())->getDirection() != DIRECTION_OUTBOUND)) {
        yylineno = (yyval.event)->getLineNumber();
        printf("Semantic error: event time range can only be used with outbound packets\n");
    }
    delete((yyvsp[(1) - (2)].event));
;}
    break;

  case 6:
#line 360 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(INVALID_EVENT);
    (yyval.event)->setLineNumber((yylsp[(2) - (2)]).first_line);
    (yyval.event)->setEventTime((yyvsp[(2) - (2)].time_usecs));
    (yyval.event)->setTimeType(RELATIVE_TIME);
;}
    break;

  case 7:
#line 366 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(INVALID_EVENT);
    (yyval.event)->setLineNumber((yylsp[(1) - (1)]).first_line);
    (yyval.event)->setEventTime((yyvsp[(1) - (1)].time_usecs));
    (yyval.event)->setTimeType(ABSOLUTE_TIME);
;}
    break;

  case 8:
#line 372 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(INVALID_EVENT);
    (yyval.event)->setLineNumber((yylsp[(1) - (1)]).first_line);
    (yyval.event)->setTimeType(ANY_TIME);
;}
    break;

  case 9:
#line 377 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(INVALID_EVENT);
    (yyval.event)->setLineNumber((yylsp[(1) - (3)]).first_line);
    (yyval.event)->setTimeType(ABSOLUTE_RANGE_TIME);
    (yyval.event)->setEventTime((yyvsp[(1) - (3)].time_usecs));
    (yyval.event)->setEventTimeEnd((yyvsp[(3) - (3)].time_usecs));
;}
    break;

  case 10:
#line 384 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(INVALID_EVENT);
    (yyval.event)->setLineNumber((yylsp[(1) - (5)]).first_line);
    (yyval.event)->setTimeType(RELATIVE_RANGE_TIME);
    (yyval.event)->setEventTime((yyvsp[(2) - (5)].time_usecs));
    (yyval.event)->setEventTimeEnd((yyvsp[(5) - (5)].time_usecs));
;}
    break;

  case 11:
#line 394 "parser.y"
    {
    if ((yyvsp[(1) - (1)].floating) < 0) {
        printf("Semantic error: negative time\n");
    }
    (yyval.time_usecs) = (int64)((yyvsp[(1) - (1)].floating) * 1.0e6); /* convert float secs to s64 microseconds */
;}
    break;

  case 12:
#line 400 "parser.y"
    {
    if ((yyvsp[(1) - (1)].integer) < 0) {
        printf("Semantic error: negative time\n");
    }
    (yyval.time_usecs) = (int64)((yyvsp[(1) - (1)].integer) * 1000000); /* convert int secs to s64 microseconds */
;}
    break;

  case 13:
#line 409 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(PACKET_EVENT);  (yyval.event)->setPacket((yyvsp[(1) - (1)].packet));
;}
    break;

  case 14:
#line 412 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(SYSCALL_EVENT);
    (yyval.event)->setSyscall((yyvsp[(1) - (1)].syscall));
;}
    break;

  case 15:
#line 419 "parser.y"
    {
    (yyval.packet) = (yyvsp[(1) - (1)].packet);
;}
    break;

  case 16:
#line 422 "parser.y"
    {
    (yyval.packet) = (yyvsp[(1) - (1)].packet);
;}
    break;

  case 17:
#line 425 "parser.y"
    {
    (yyval.packet) = (yyvsp[(1) - (1)].packet);
;}
    break;

  case 18:
#line 431 "parser.y"
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

  case 19:
#line 457 "parser.y"
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

  case 20:
#line 476 "parser.y"
    {
    PacketDrillPacket *inner = NULL;;
    enum direction_t direction = (yyvsp[(1) - (4)].packet)->getDirection();
    cPacket* pkt = PacketDrill::buildSCTPPacket(in_config->getWireProtocol(), direction, (yyvsp[(4) - (4)].sctp_chunk_list));
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

  case 21:
#line 492 "parser.y"
    { (yyval.sctp_chunk_list) = new cQueue("sctpChunkList");
                                   (yyval.sctp_chunk_list)->insert((cObject*)(yyvsp[(1) - (1)].sctp_chunk)); ;}
    break;

  case 22:
#line 494 "parser.y"
    { (yyval.sctp_chunk_list) = (yyvsp[(1) - (3)].sctp_chunk_list);
                                   (yyvsp[(1) - (3)].sctp_chunk_list)->insert((yyvsp[(3) - (3)].sctp_chunk)); ;}
    break;

  case 23:
#line 500 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 24:
#line 501 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 25:
#line 502 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 26:
#line 503 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 27:
#line 504 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 28:
#line 505 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 29:
#line 506 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 30:
#line 507 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 31:
#line 508 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 32:
#line 509 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 33:
#line 510 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 34:
#line 511 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 35:
#line 516 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 36:
#line 517 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: flags value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 37:
#line 523 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: flags value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 38:
#line 532 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 39:
#line 533 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: length value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 40:
#line 542 "parser.y"
    { (yyval.byte_list) = NULL; ;}
    break;

  case 41:
#line 543 "parser.y"
    { (yyval.byte_list) = NULL; ;}
    break;

  case 42:
#line 544 "parser.y"
    { (yyval.byte_list) = (yyvsp[(4) - (5)].byte_list); ;}
    break;

  case 43:
#line 548 "parser.y"
    { (yyval.byte_list) = new PacketDrillBytes((yyvsp[(1) - (1)].byte)); ;}
    break;

  case 44:
#line 549 "parser.y"
    { (yyval.byte_list) = (yyvsp[(1) - (3)].byte_list);
                       (yyvsp[(1) - (3)].byte_list)->appendByte((yyvsp[(3) - (3)].byte)); ;}
    break;

  case 45:
#line 554 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(1) - (1)].integer))) {
        printf("Semantic error: byte value out of range\n");
    }
    (yyval.byte) = (yyvsp[(1) - (1)].integer);
;}
    break;

  case 46:
#line 560 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(1) - (1)].integer))) {
        printf("Semantic error: byte value out of range\n");
    }
    (yyval.byte) = (yyvsp[(1) - (1)].integer);
;}
    break;

  case 47:
#line 569 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 48:
#line 570 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: flags value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 49:
#line 576 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: flags value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 50:
#line 582 "parser.y"
    {
    uint64 flags;
    char *c;

    flags = 0;
    for (c = (yyvsp[(3) - (3)].string); *c != '\0'; c++) {
        switch (*c) {
        case 'I':
            if (flags & SCTP_DATA_CHUNK_I_BIT) {
                printf("Semantic error: I-bit specified multiple times\n");
            } else {
                flags |= SCTP_DATA_CHUNK_I_BIT;
            }
            break;
        case 'U':
            if (flags & SCTP_DATA_CHUNK_U_BIT) {
                printf("Semantic error: U-bit specified multiple times\n");
            } else {
                flags |= SCTP_DATA_CHUNK_U_BIT;
            }
            break;
        case 'B':
            if (flags & SCTP_DATA_CHUNK_B_BIT) {
                printf("Semantic error: B-bit specified multiple times\n");
            } else {
                flags |= SCTP_DATA_CHUNK_B_BIT;
            }
            break;
        case 'E':
            if (flags & SCTP_DATA_CHUNK_E_BIT) {
                printf("Semantic error: E-bit specified multiple times\n");
            } else {
                flags |= SCTP_DATA_CHUNK_E_BIT;
            }
            break;
        default:
            printf("Semantic error: Only expecting IUBE as flags\n");
            break;
        }
    }
    (yyval.integer) = flags;
;}
    break;

  case 51:
#line 627 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 52:
#line 628 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: flags value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 53:
#line 634 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: flags value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 54:
#line 640 "parser.y"
    {
    uint64 flags;
    char *c;

    flags = 0;
    for (c = (yyvsp[(3) - (3)].string); *c != '\0'; c++) {
        switch (*c) {
        case 'T':
            if (flags & SCTP_ABORT_CHUNK_T_BIT) {
                printf("Semantic error: T-bit specified multiple times\n");
            } else {
                flags |= SCTP_ABORT_CHUNK_T_BIT;
            }
            break;
        default:
            printf("Semantic error: Only expecting T as flags\n");
            break;
        }
    }
    (yyval.integer) = flags;
;}
    break;

  case 55:
#line 664 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 56:
#line 665 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: flags value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 57:
#line 671 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: flags value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 58:
#line 677 "parser.y"
    {
    uint64 flags;
    char *c;

    flags = 0;
    for (c = (yyvsp[(3) - (3)].string); *c != '\0'; c++) {
        switch (*c) {
        case 'T':
            if (flags & SCTP_SHUTDOWN_COMPLETE_CHUNK_T_BIT) {
                printf("Semantic error: T-bit specified multiple times\n");
            } else {
                flags |= SCTP_SHUTDOWN_COMPLETE_CHUNK_T_BIT;
            }
            break;
        default:
            printf("Semantic error: Only expecting T as flags\n");
            break;
        }
    }
    (yyval.integer) = flags;
;}
    break;

  case 59:
#line 702 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 60:
#line 703 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: tag value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 61:
#line 712 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 62:
#line 713 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: a_rwnd value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 63:
#line 722 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 64:
#line 723 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: os value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 65:
#line 732 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 66:
#line 733 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: is value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 67:
#line 742 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 68:
#line 743 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: tsn value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 69:
#line 752 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 70:
#line 753 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: sid value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 71:
#line 762 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 72:
#line 763 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: ssn value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 73:
#line 773 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 74:
#line 774 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: ppid value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 75:
#line 780 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: ppid value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 76:
#line 789 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 77:
#line 790 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: cum_tsn value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 78:
#line 799 "parser.y"
    { (yyval.sack_block_list) = NULL; ;}
    break;

  case 79:
#line 800 "parser.y"
    { (yyval.sack_block_list) = NULL; ;}
    break;

  case 80:
#line 801 "parser.y"
    { (yyval.sack_block_list) = (yyvsp[(4) - (5)].sack_block_list); ;}
    break;

  case 81:
#line 806 "parser.y"
    { (yyval.sack_block_list) = NULL; ;}
    break;

  case 82:
#line 807 "parser.y"
    { (yyval.sack_block_list) = NULL; ;}
    break;

  case 83:
#line 808 "parser.y"
    { (yyval.sack_block_list) = (yyvsp[(4) - (5)].sack_block_list); ;}
    break;

  case 84:
#line 813 "parser.y"
    {
    if (((yyvsp[(5) - (14)].integer) != -1) &&
        (!is_valid_u16((yyvsp[(5) - (14)].integer)) || ((yyvsp[(5) - (14)].integer) < SCTP_DATA_CHUNK_LENGTH))) {
        printf("Semantic error: length value out of range\n");
    }
    (yyval.sctp_chunk) = PacketDrill::buildDataChunk((yyvsp[(3) - (14)].integer), (yyvsp[(5) - (14)].integer), (yyvsp[(7) - (14)].integer), (yyvsp[(9) - (14)].integer), (yyvsp[(11) - (14)].integer), (yyvsp[(13) - (14)].integer));
;}
    break;

  case 85:
#line 822 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildInitChunk((yyvsp[(3) - (15)].integer), (yyvsp[(5) - (15)].integer), (yyvsp[(7) - (15)].integer), (yyvsp[(9) - (15)].integer), (yyvsp[(11) - (15)].integer), (yyvsp[(13) - (15)].integer), (yyvsp[(14) - (15)].expression_list));
;}
    break;

  case 86:
#line 827 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildInitAckChunk((yyvsp[(3) - (15)].integer), (yyvsp[(5) - (15)].integer), (yyvsp[(7) - (15)].integer), (yyvsp[(9) - (15)].integer), (yyvsp[(11) - (15)].integer), (yyvsp[(13) - (15)].integer), (yyvsp[(14) - (15)].expression_list));
;}
    break;

  case 87:
#line 832 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildSackChunk((yyvsp[(3) - (12)].integer), (yyvsp[(5) - (12)].integer), (yyvsp[(7) - (12)].integer), (yyvsp[(9) - (12)].sack_block_list), (yyvsp[(11) - (12)].sack_block_list));
;}
    break;

  case 88:
#line 837 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildHeartbeatChunk((yyvsp[(3) - (6)].integer), (yyvsp[(5) - (6)].sctp_parameter));
;}
    break;

  case 89:
#line 843 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildHeartbeatAckChunk((yyvsp[(3) - (6)].integer), (yyvsp[(5) - (6)].sctp_parameter));
;}
    break;

  case 90:
#line 849 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildAbortChunk((yyvsp[(3) - (4)].integer));
;}
    break;

  case 91:
#line 854 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildShutdownChunk((yyvsp[(3) - (6)].integer), (yyvsp[(5) - (6)].integer));
;}
    break;

  case 92:
#line 859 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildShutdownAckChunk((yyvsp[(3) - (4)].integer));
;}
    break;

  case 93:
#line 864 "parser.y"
    {
    if (((yyvsp[(5) - (8)].integer) != -1) &&
        (!is_valid_u16((yyvsp[(5) - (8)].integer)) || ((yyvsp[(5) - (8)].integer) < SCTP_COOKIE_ACK_LENGTH))) {
        printf("Semantic error: length value out of range\n");
    }
    if (((yyvsp[(5) - (8)].integer) != -1) && ((yyvsp[(7) - (8)].byte_list) != NULL) &&
        ((yyvsp[(5) - (8)].integer) != SCTP_COOKIE_ACK_LENGTH + (yyvsp[(7) - (8)].byte_list)->getListLength())) {
        printf("Semantic error: length value incompatible with val\n");
    }
    if (((yyvsp[(5) - (8)].integer) == -1) && ((yyvsp[(7) - (8)].byte_list) != NULL)) {
        printf("Semantic error: length needs to be specified\n");
    }
    (yyval.sctp_chunk) = PacketDrill::buildCookieEchoChunk((yyvsp[(3) - (8)].integer), (yyvsp[(5) - (8)].integer), (yyvsp[(7) - (8)].byte_list));
;}
    break;

  case 94:
#line 880 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildCookieAckChunk((yyvsp[(3) - (4)].integer));
;}
    break;

  case 95:
#line 885 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildShutdownCompleteChunk((yyvsp[(3) - (4)].integer));
;}
    break;

  case 96:
#line 890 "parser.y"
    { (yyval.expression_list) = NULL; ;}
    break;

  case 97:
#line 891 "parser.y"
    { (yyval.expression_list) = (yyvsp[(2) - (2)].expression_list); ;}
    break;

  case 98:
#line 895 "parser.y"
    {
    (yyval.expression_list) = new cQueue("sctp_parameter_list");
    (yyval.expression_list)->insert((yyvsp[(1) - (1)].sctp_parameter));
;}
    break;

  case 99:
#line 899 "parser.y"
    {
    (yyval.expression_list) = (yyvsp[(1) - (3)].expression_list);
    (yyval.expression_list)->insert((yyvsp[(3) - (3)].sctp_parameter));
;}
    break;

  case 100:
#line 907 "parser.y"
    { (yyval.sctp_parameter) = (yyvsp[(1) - (1)].sctp_parameter); ;}
    break;

  case 101:
#line 908 "parser.y"
    { (yyval.sctp_parameter) = (yyvsp[(1) - (1)].sctp_parameter); ;}
    break;

  case 102:
#line 913 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(-1, NULL);
;}
    break;

  case 103:
#line 916 "parser.y"
    {
    if (((yyvsp[(3) - (6)].integer) != -1) &&
        (!is_valid_u16((yyvsp[(3) - (6)].integer)) || ((yyvsp[(3) - (6)].integer) < 4))) {
        printf("Semantic error: length value out of range\n");
    }
    if (((yyvsp[(3) - (6)].integer) != -1) && ((yyvsp[(5) - (6)].byte_list) != NULL) &&
        ((yyvsp[(3) - (6)].integer) != 4 + (yyvsp[(5) - (6)].byte_list)->getListLength())) {
        printf("Semantic error: length value incompatible with val\n");
    }
    if (((yyvsp[(3) - (6)].integer) == -1) && ((yyvsp[(5) - (6)].byte_list) != NULL)) {
        printf("Semantic error: length needs to be specified\n");
    }
    (yyval.sctp_parameter) = new PacketDrillSctpParameter((yyvsp[(3) - (6)].integer), (yyvsp[(5) - (6)].byte_list));
;}
    break;

  case 104:
#line 933 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(-1, NULL);
;}
    break;

  case 105:
#line 936 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(-1, NULL);
;}
    break;

  case 106:
#line 939 "parser.y"
    {
    if (((yyvsp[(5) - (10)].integer) < 4) || !is_valid_u32((yyvsp[(5) - (10)].integer))) {
        printf("Semantic error: len value out of range\n");
    }
    (yyval.sctp_parameter) = new PacketDrillSctpParameter((yyvsp[(5) - (10)].integer), NULL);
;}
    break;

  case 107:
#line 949 "parser.y"
    {
    (yyval.packet) = new PacketDrillPacket();
    (yyval.packet)->setDirection((yyvsp[(1) - (1)].direction));
;}
    break;

  case 108:
#line 957 "parser.y"
    {
    (yyval.direction) = DIRECTION_INBOUND;
    current_script_line = yylineno;
;}
    break;

  case 109:
#line 961 "parser.y"
    {
    (yyval.direction) = DIRECTION_OUTBOUND;
    current_script_line = yylineno;
;}
    break;

  case 110:
#line 968 "parser.y"
    {
    (yyval.string) = (yyvsp[(1) - (1)].string);
;}
    break;

  case 111:
#line 971 "parser.y"
    {
    (yyval.string) = strdup(".");
;}
    break;

  case 112:
#line 974 "parser.y"
    {
printf("parse MYWORD\n");
    asprintf(&((yyval.string)), "%s.", (yyvsp[(1) - (2)].string));
printf("after parse MYWORD\n");
    free((yyvsp[(1) - (2)].string));
printf("after free MYWORD\n");
;}
    break;

  case 113:
#line 981 "parser.y"
    {
    (yyval.string) = strdup("");
;}
    break;

  case 114:
#line 987 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(1) - (6)].integer))) {
        printf("TCP start sequence number out of range");
    }
    if (!is_valid_u32((yyvsp[(3) - (6)].integer))) {
        printf("TCP end sequence number out of range");
    }
    if (!is_valid_u16((yyvsp[(5) - (6)].integer))) {
        printf("TCP payload size out of range");
    }
    if ((yyvsp[(3) - (6)].integer) != ((yyvsp[(1) - (6)].integer) +(yyvsp[(5) - (6)].integer))) {
        printf("inconsistent TCP sequence numbers and payload size");
    }
    (yyval.tcp_sequence_info).start_sequence = (yyvsp[(1) - (6)].integer);
    (yyval.tcp_sequence_info).payload_bytes = (yyvsp[(5) - (6)].integer);
    (yyval.tcp_sequence_info).protocol = IPPROTO_TCP;
;}
    break;

  case 115:
#line 1007 "parser.y"
    {
    (yyval.sequence_number) = 0;
;}
    break;

  case 116:
#line 1010 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(2) - (2)].integer))) {
    printf("TCP ack sequence number out of range");
    }
    (yyval.sequence_number) = (yyvsp[(2) - (2)].integer);
;}
    break;

  case 117:
#line 1019 "parser.y"
    {
    (yyval.window) = -1;
;}
    break;

  case 118:
#line 1022 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(2) - (2)].integer))) {
        printf("TCP window value out of range");
    }
    (yyval.window) = (yyvsp[(2) - (2)].integer);
;}
    break;

  case 119:
#line 1031 "parser.y"
    {
    (yyval.tcp_options) = new cQueue("opt_tcp_options");
;}
    break;

  case 120:
#line 1034 "parser.y"
    {
    (yyval.tcp_options) = (yyvsp[(2) - (3)].tcp_options);
;}
    break;

  case 121:
#line 1037 "parser.y"
    {
    (yyval.tcp_options) = NULL; /* FLAG_OPTIONS_NOCHECK */
;}
    break;

  case 122:
#line 1044 "parser.y"
    {
    (yyval.tcp_options) = new cQueue("tcp_option");
    (yyval.tcp_options)->insert((yyvsp[(1) - (1)].tcp_option));
;}
    break;

  case 123:
#line 1048 "parser.y"
    {
    (yyval.tcp_options) = (yyvsp[(1) - (3)].tcp_options);
    (yyval.tcp_options)->insert((yyvsp[(3) - (3)].tcp_option));
;}
    break;

  case 124:
#line 1056 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_NOP, 1);
;}
    break;

  case 125:
#line 1059 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_EOL, 1);
;}
    break;

  case 126:
#line 1062 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_MAXSEG, TCPOLEN_MAXSEG);
    if (!is_valid_u16((yyvsp[(2) - (2)].integer))) {
        printf("mss value out of range");
    }
    (yyval.tcp_option)->setMss((yyvsp[(2) - (2)].integer));
;}
    break;

  case 127:
#line 1069 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_WINDOW, TCPOLEN_WINDOW);
    if (!is_valid_u8((yyvsp[(2) - (2)].integer))) {
        printf("window scale shift count out of range");
    }
    (yyval.tcp_option)->setWindowScale((yyvsp[(2) - (2)].integer));
;}
    break;

  case 128:
#line 1076 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_SACK_PERMITTED, TCPOLEN_SACK_PERMITTED);
;}
    break;

  case 129:
#line 1079 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_SACK, 2+8*(yyvsp[(2) - (2)].sack_block_list)->getLength());
    (yyval.tcp_option)->setBlockList((yyvsp[(2) - (2)].sack_block_list));
;}
    break;

  case 130:
#line 1083 "parser.y"
    {
    uint32 val, ecr;
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_TIMESTAMP, TCPOLEN_TIMESTAMP);
    if (!is_valid_u32((yyvsp[(3) - (5)].integer))) {
        printf("ts val out of range");
    }
    if (!is_valid_u32((yyvsp[(5) - (5)].integer))) {
        printf("ecr val out of range");
    }
    val = (yyvsp[(3) - (5)].integer);
    ecr = (yyvsp[(5) - (5)].integer);
    (yyval.tcp_option)->setVal(val);
    (yyval.tcp_option)->setEcr(ecr);
;}
    break;

  case 131:
#line 1100 "parser.y"
    {
    (yyval.sack_block_list) = new cQueue("sack_block_list");
    (yyval.sack_block_list)->insert((yyvsp[(1) - (1)].sack_block));
;}
    break;

  case 132:
#line 1104 "parser.y"
    {
    (yyval.sack_block_list) = (yyvsp[(1) - (2)].sack_block_list); (yyvsp[(1) - (2)].sack_block_list)->insert((yyvsp[(2) - (2)].sack_block));
;}
    break;

  case 133:
#line 1110 "parser.y"
    { (yyval.sack_block_list) = new cQueue("gap_list");;}
    break;

  case 134:
#line 1111 "parser.y"
    {
    (yyval.sack_block_list) = new cQueue("gap_list");
    (yyval.sack_block_list)->insert((yyvsp[(1) - (1)].sack_block));
;}
    break;

  case 135:
#line 1115 "parser.y"
    {
    (yyval.sack_block_list) = (yyvsp[(1) - (3)].sack_block_list); (yyvsp[(1) - (3)].sack_block_list)->insert((yyvsp[(3) - (3)].sack_block));
;}
    break;

  case 136:
#line 1121 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(1) - (3)].integer))) {
        printf("Semantic error: start value out of range\n");
    }
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: end value out of range\n");
    }
    (yyval.sack_block) = new PacketDrillStruct((yyvsp[(1) - (3)].integer), (yyvsp[(3) - (3)].integer));
;}
    break;

  case 137:
#line 1133 "parser.y"
    { (yyval.sack_block_list) = new cQueue("dup_list");;}
    break;

  case 138:
#line 1134 "parser.y"
    {
    (yyval.sack_block_list) = new cQueue("dup_list");
    (yyval.sack_block_list)->insert((yyvsp[(1) - (1)].sack_block));
;}
    break;

  case 139:
#line 1138 "parser.y"
    {
    (yyval.sack_block_list) = (yyvsp[(1) - (3)].sack_block_list); (yyvsp[(1) - (3)].sack_block_list)->insert((yyvsp[(3) - (3)].sack_block));
;}
    break;

  case 140:
#line 1144 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(1) - (3)].integer))) {
        printf("Semantic error: start value out of range\n");
    }
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: end value out of range\n");
    }
    (yyval.sack_block) = new PacketDrillStruct((yyvsp[(1) - (3)].integer), (yyvsp[(3) - (3)].integer));
;}
    break;

  case 141:
#line 1156 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(1) - (3)].integer))) {
        printf("TCP SACK left sequence number out of range\n");
    }
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        printf("TCP SACK right sequence number out of range");
    }
    PacketDrillStruct *block = new PacketDrillStruct((yyvsp[(1) - (3)].integer), (yyvsp[(3) - (3)].integer));
    if (!is_valid_u32((yyvsp[(1) - (3)].integer))) {
        printf("TCP SACK left sequence number out of range");
    }
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        printf("TCP SACK right sequence number out of range");
    }
    (yyval.sack_block) = block;
;}
    break;

  case 142:
#line 1175 "parser.y"
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

  case 143:
#line 1187 "parser.y"
    {
    (yyval.time_usecs) = -1;
;}
    break;

  case 144:
#line 1190 "parser.y"
    {
    (yyval.time_usecs) = (yyvsp[(2) - (2)].time_usecs);
;}
    break;

  case 145:
#line 1196 "parser.y"
    {
    (yyval.string) = (yyvsp[(1) - (1)].string);
    current_script_line = yylineno;
;}
    break;

  case 146:
#line 1203 "parser.y"
    {
    (yyval.expression_list) = NULL;
;}
    break;

  case 147:
#line 1206 "parser.y"
    {
    (yyval.expression_list) = (yyvsp[(2) - (3)].expression_list);
;}
    break;

  case 148:
#line 1212 "parser.y"
    {
    (yyval.expression_list) = new cQueue("expressionList");
    (yyval.expression_list)->insert((cObject*)(yyvsp[(1) - (1)].expression));
;}
    break;

  case 149:
#line 1216 "parser.y"
    {
    (yyval.expression_list) = (yyvsp[(1) - (3)].expression_list);
    (yyvsp[(1) - (3)].expression_list)->insert((yyvsp[(3) - (3)].expression));
;}
    break;

  case 150:
#line 1223 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS);
;}
    break;

  case 151:
#line 1226 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression); ;}
    break;

  case 152:
#line 1228 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 153:
#line 1231 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_WORD);
    (yyval.expression)->setString((yyvsp[(1) - (1)].string));
;}
    break;

  case 154:
#line 1235 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_STRING);
    (yyval.expression)->setString((yyvsp[(1) - (1)].string));
    (yyval.expression)->setFormat("\"%s\"");
;}
    break;

  case 155:
#line 1240 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_STRING);
    (yyval.expression)->setString((yyvsp[(1) - (2)].string));
    (yyval.expression)->setFormat("\"%s\"...");
;}
    break;

  case 156:
#line 1245 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 157:
#line 1248 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 158:
#line 1251 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 159:
#line 1254 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 160:
#line 1257 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 161:
#line 1260 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 162:
#line 1268 "parser.y"
    {
    (yyval.expression) = new_integer_expression((yyvsp[(1) - (1)].integer), "%ld");
;}
    break;

  case 163:
#line 1274 "parser.y"
    {
    (yyval.expression) = new_integer_expression((yyvsp[(1) - (1)].integer), "%#lx");
;}
    break;

  case 164:
#line 1280 "parser.y"
    {    /* bitwise OR */
    (yyval.expression) = new PacketDrillExpression(EXPR_BINARY);
    struct binary_expression *binary = (struct binary_expression *) malloc(sizeof(struct binary_expression));
    binary->op = strdup("|");
    binary->lhs = (yyvsp[(1) - (3)].expression);
    binary->rhs = (yyvsp[(3) - (3)].expression);
    (yyval.expression)->setBinary(binary);
;}
    break;

  case 165:
#line 1291 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_LIST);
    (yyval.expression)->setList(NULL);
;}
    break;

  case 166:
#line 1295 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_LIST);
    (yyval.expression)->setList((yyvsp[(2) - (3)].expression_list));
;}
    break;

  case 167:
#line 1302 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        printf("srto_initial out of range\n");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 168:
#line 1308 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS);
;}
    break;

  case 169:
#line 1314 "parser.y"
    {
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 170:
#line 1317 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 171:
#line 1321 "parser.y"
    {
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 172:
#line 1324 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 173:
#line 1328 "parser.y"
    {
    (yyval.expression) = new_integer_expression((yyvsp[(1) - (1)].integer), "%u");
;}
    break;

  case 174:
#line 1331 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_WORD);
    (yyval.expression)->setString((yyvsp[(1) - (1)].string));
;}
    break;

  case 175:
#line 1335 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 176:
#line 1339 "parser.y"
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

  case 177:
#line 1348 "parser.y"
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

  case 178:
#line 1360 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: sinit_num_ostreams out of range\n");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 179:
#line 1366 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 180:
#line 1370 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: sinit_max_instreams out of range\n");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 181:
#line 1376 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 182:
#line 1380 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: sinit_max_attempts out of range\n");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 183:
#line 1386 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 184:
#line 1390 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: sinit_max_init_timeo out of range\n");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 185:
#line 1396 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 186:
#line 1401 "parser.y"
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

  case 187:
#line 1413 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_ASSOCVAL);
    struct sctp_assoc_value_expr *assocval = (struct sctp_assoc_value_expr *) malloc(sizeof(struct sctp_assoc_value_expr));
    assocval->assoc_id = (yyvsp[(4) - (9)].expression);
    assocval->assoc_value = (yyvsp[(8) - (9)].expression);
    (yyval.expression)->setAssocval(assocval);
;}
    break;

  case 188:
#line 1420 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_ASSOCVAL);
    struct sctp_assoc_value_expr *assocval = (struct sctp_assoc_value_expr *) malloc(sizeof(struct sctp_assoc_value_expr));
    assocval->assoc_id = new PacketDrillExpression(EXPR_ELLIPSIS);
    assocval->assoc_value = (yyvsp[(4) - (5)].expression);
    (yyval.expression)->setAssocval(assocval);
;}
    break;

  case 189:
#line 1430 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: sack_delay out of range\n");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 190:
#line 1436 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS);
;}
    break;

  case 191:
#line 1441 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: sack_freq out of range\n");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 192:
#line 1447 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 193:
#line 1450 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_SACKINFO);
    struct sctp_sack_info_expr *sackinfo = (struct sctp_sack_info_expr *) malloc(sizeof(struct sctp_sack_info_expr));
    sackinfo->sack_assoc_id = (yyvsp[(4) - (9)].expression);
    sackinfo->sack_delay = (yyvsp[(6) - (9)].expression);
    sackinfo->sack_freq = (yyvsp[(8) - (9)].expression);
    (yyval.expression)->setSackinfo(sackinfo);
;}
    break;

  case 194:
#line 1458 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_SACKINFO);
    struct sctp_sack_info_expr *sackinfo = (struct sctp_sack_info_expr *) malloc(sizeof(struct sctp_sack_info_expr));
    sackinfo->sack_assoc_id = new PacketDrillExpression(EXPR_ELLIPSIS);
    sackinfo->sack_delay = (yyvsp[(2) - (5)].expression);
    sackinfo->sack_freq = (yyvsp[(4) - (5)].expression);
    (yyval.expression)->setSackinfo(sackinfo);
;}
    break;

  case 195:
#line 1469 "parser.y"
    {
    (yyval.errno_info) = NULL;
;}
    break;

  case 196:
#line 1472 "parser.y"
    {
    (yyval.errno_info) = (struct errno_spec*)malloc(sizeof(struct errno_spec));
    (yyval.errno_info)->errno_macro = (yyvsp[(1) - (2)].string);
    (yyval.errno_info)->strerror = (yyvsp[(2) - (2)].string);
;}
    break;

  case 197:
#line 1480 "parser.y"
    {
    (yyval.string) = NULL;
;}
    break;

  case 198:
#line 1483 "parser.y"
    {
    (yyval.string) = (yyvsp[(1) - (1)].string);
;}
    break;

  case 199:
#line 1489 "parser.y"
    {
    (yyval.string) = (yyvsp[(2) - (3)].string);
;}
    break;

  case 200:
#line 1495 "parser.y"
    {
    (yyval.string) = (yyvsp[(1) - (1)].string);
;}
    break;

  case 201:
#line 1498 "parser.y"
    {
    asprintf(&((yyval.string)), "%s %s", (yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string));
    free((yyvsp[(1) - (2)].string));
    free((yyvsp[(2) - (2)].string));
;}
    break;


/* Line 1267 of yacc.c.  */
#line 3878 "parser.cc"
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
  yyerrstatus = 3;    /* Each real token shifted decrements this.  */

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



