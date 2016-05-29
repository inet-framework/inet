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
     MYSUPPORTED_EXTENSIONS = 294,
     TYPES = 295,
     TAG = 296,
     A_RWND = 297,
     OS = 298,
     IS = 299,
     TSN = 300,
     MYSID = 301,
     SSN = 302,
     PPID = 303,
     CUM_TSN = 304,
     GAPS = 305,
     DUPS = 306,
     SRTO_ASSOC_ID = 307,
     SRTO_INITIAL = 308,
     SRTO_MAX = 309,
     SRTO_MIN = 310,
     SINIT_NUM_OSTREAMS = 311,
     SINIT_MAX_INSTREAMS = 312,
     SINIT_MAX_ATTEMPTS = 313,
     SINIT_MAX_INIT_TIMEO = 314,
     MYSACK_DELAY = 315,
     SACK_FREQ = 316,
     ASSOC_VALUE = 317,
     ASSOC_ID = 318,
     SACK_ASSOC_ID = 319,
     MYFLOAT = 320,
     INTEGER = 321,
     HEX_INTEGER = 322,
     MYWORD = 323,
     MYSTRING = 324
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
#define MYSUPPORTED_EXTENSIONS 294
#define TYPES 295
#define TAG 296
#define A_RWND 297
#define OS 298
#define IS 299
#define TSN 300
#define MYSID 301
#define SSN 302
#define PPID 303
#define CUM_TSN 304
#define GAPS 305
#define DUPS 306
#define SRTO_ASSOC_ID 307
#define SRTO_INITIAL 308
#define SRTO_MAX 309
#define SRTO_MIN 310
#define SINIT_NUM_OSTREAMS 311
#define SINIT_MAX_INSTREAMS 312
#define SINIT_MAX_ATTEMPTS 313
#define SINIT_MAX_INIT_TIMEO 314
#define MYSACK_DELAY 315
#define SACK_FREQ 316
#define ASSOC_VALUE 317
#define ASSOC_ID 318
#define SACK_ASSOC_ID 319
#define MYFLOAT 320
#define INTEGER 321
#define HEX_INTEGER 322
#define MYWORD 323
#define MYSTRING 324




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
#line 218 "parser.y"
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
    PacketDrillStruct *sack_block;
    PacketDrillExpression *expression;
    cQueue *expression_list;
    PacketDrillTcpOption *tcp_option;
    PacketDrillSctpParameter *sctp_parameter;
    PacketDrillOption *option;
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
#line 480 "parser.cc"
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
#line 505 "parser.cc"

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
#define YYLAST   546

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  88
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  100
/* YYNRULES -- Number of rules.  */
#define YYNRULES  229
/* YYNRULES -- Number of states.  */
#define YYNSTATES  505

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   324

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      74,    75,    72,    71,    80,    84,    83,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    76,    77,
      81,    70,    82,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    78,     2,    79,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    86,    85,    87,    73,     2,     2,     2,
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
      65,    66,    67,    68,    69
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,     7,     9,    11,    14,    18,    20,
      22,    24,    26,    28,    31,    34,    37,    39,    41,    45,
      51,    53,    55,    57,    59,    61,    63,    65,    72,    78,
      83,    85,    89,    91,    93,    95,    97,    99,   101,   103,
     105,   107,   109,   111,   113,   117,   121,   125,   129,   133,
     137,   143,   149,   151,   155,   157,   161,   163,   165,   167,
     169,   171,   173,   175,   177,   179,   181,   183,   185,   187,
     189,   191,   193,   197,   201,   205,   209,   213,   217,   221,
     225,   229,   233,   237,   241,   245,   249,   253,   257,   261,
     265,   269,   273,   277,   281,   285,   289,   293,   297,   301,
     305,   309,   313,   317,   321,   327,   333,   337,   343,   349,
     364,   380,   396,   409,   416,   423,   428,   435,   440,   449,
     454,   459,   462,   465,   467,   471,   473,   475,   477,   482,
     489,   496,   505,   510,   521,   532,   534,   536,   538,   540,
     542,   545,   547,   554,   555,   558,   559,   562,   563,   567,
     571,   573,   577,   579,   581,   584,   587,   589,   592,   598,
     600,   603,   604,   606,   610,   614,   615,   617,   621,   625,
     629,   637,   638,   641,   643,   646,   650,   652,   656,   658,
     660,   662,   664,   666,   669,   671,   673,   675,   677,   679,
     681,   683,   685,   689,   692,   696,   700,   704,   708,   712,
     716,   720,   722,   724,   726,   738,   746,   750,   754,   758,
     762,   766,   770,   774,   778,   788,   798,   804,   808,   812,
     816,   820,   830,   836,   837,   840,   841,   843,   847,   849
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      89,     0,    -1,    90,    95,    -1,    -1,    91,    -1,    92,
      -1,    91,    92,    -1,    93,    70,    94,    -1,    16,    -1,
      66,    -1,    68,    -1,    69,    -1,    96,    -1,    95,    96,
      -1,    97,    99,    -1,    71,    98,    -1,    98,    -1,    72,
      -1,    98,    73,    98,    -1,    71,    98,    73,    71,    98,
      -1,    65,    -1,    66,    -1,   100,    -1,   160,    -1,   101,
      -1,   102,    -1,   103,    -1,   145,   147,   148,   149,   150,
     151,    -1,   145,     4,    74,    66,    75,    -1,   145,    35,
      76,   104,    -1,   105,    -1,   104,    77,   105,    -1,   127,
      -1,   128,    -1,   129,    -1,   130,    -1,   131,    -1,   132,
      -1,   133,    -1,   134,    -1,   135,    -1,   136,    -1,   137,
      -1,   138,    -1,    37,    70,     3,    -1,    37,    70,    67,
      -1,    37,    70,    66,    -1,    38,    70,     3,    -1,    38,
      70,    66,    -1,    14,    70,     3,    -1,    14,    70,    78,
       3,    79,    -1,    14,    70,    78,   109,    79,    -1,   111,
      -1,   109,    80,   111,    -1,   112,    -1,   110,    80,   112,
      -1,    67,    -1,    66,    -1,    67,    -1,    66,    -1,    18,
      -1,    19,    -1,    20,    -1,    32,    -1,    21,    -1,    22,
      -1,    23,    -1,    24,    -1,    25,    -1,    27,    -1,    28,
      -1,    29,    -1,    37,    70,     3,    -1,    37,    70,    67,
      -1,    37,    70,    66,    -1,    37,    70,    68,    -1,    37,
      70,     3,    -1,    37,    70,    67,    -1,    37,    70,    66,
      -1,    37,    70,    68,    -1,    37,    70,     3,    -1,    37,
      70,    67,    -1,    37,    70,    66,    -1,    37,    70,    68,
      -1,    41,    70,     3,    -1,    41,    70,    66,    -1,    42,
      70,     3,    -1,    42,    70,    66,    -1,    43,    70,     3,
      -1,    43,    70,    66,    -1,    44,    70,     3,    -1,    44,
      70,    66,    -1,    45,    70,     3,    -1,    45,    70,    66,
      -1,    46,    70,     3,    -1,    46,    70,    66,    -1,    47,
      70,     3,    -1,    47,    70,    66,    -1,    48,    70,     3,
      -1,    48,    70,    66,    -1,    48,    70,    67,    -1,    49,
      70,     3,    -1,    49,    70,    66,    -1,    50,    70,     3,
      -1,    50,    70,    78,     3,    79,    -1,    50,    70,    78,
     155,    79,    -1,    51,    70,     3,    -1,    51,    70,    78,
       3,    79,    -1,    51,    70,    78,   157,    79,    -1,    18,
      78,   113,    80,   107,    80,   120,    80,   121,    80,   122,
      80,   123,    79,    -1,    19,    78,   106,    80,   116,    80,
     117,    80,   118,    80,   119,    80,   120,   139,    79,    -1,
      20,    78,   106,    80,   116,    80,   117,    80,   118,    80,
     119,    80,   120,   139,    79,    -1,    32,    78,   106,    80,
     124,    80,   117,    80,   125,    80,   126,    79,    -1,    21,
      78,   106,    80,   142,    79,    -1,    22,    78,   106,    80,
     142,    79,    -1,    23,    78,   114,    79,    -1,    24,    78,
     106,    80,   124,    79,    -1,    25,    78,   106,    79,    -1,
      27,    78,   106,    80,   107,    80,   108,    79,    -1,    28,
      78,   106,    79,    -1,    29,    78,   115,    79,    -1,    80,
       3,    -1,    80,   140,    -1,   141,    -1,   140,    80,   141,
      -1,   142,    -1,   144,    -1,   143,    -1,    30,    78,     3,
      79,    -1,    30,    78,   107,    80,   108,    79,    -1,    39,
      78,    40,    70,     3,    79,    -1,    39,    78,    40,    70,
      78,   110,    79,    79,    -1,    33,    78,     3,    79,    -1,
      33,    78,    38,    70,     3,    80,    14,    70,     3,    79,
      -1,    33,    78,    38,    70,    66,    80,    14,    70,     3,
      79,    -1,   146,    -1,    81,    -1,    82,    -1,    68,    -1,
      83,    -1,    68,    83,    -1,    84,    -1,    66,    76,    66,
      74,    66,    75,    -1,    -1,     5,    66,    -1,    -1,     6,
      66,    -1,    -1,    81,   152,    82,    -1,    81,     3,    82,
      -1,   153,    -1,   152,    80,   153,    -1,     9,    -1,    12,
      -1,     8,    66,    -1,     7,    66,    -1,    15,    -1,    13,
     154,    -1,    10,    14,    66,    11,    66,    -1,   159,    -1,
     154,   159,    -1,    -1,   156,    -1,   155,    80,   156,    -1,
      66,    76,    66,    -1,    -1,   158,    -1,   157,    80,   158,
      -1,    66,    76,    66,    -1,    66,    76,    66,    -1,   161,
     162,   163,    70,   165,   184,   185,    -1,    -1,     3,    98,
      -1,    68,    -1,    74,    75,    -1,    74,   164,    75,    -1,
     165,    -1,   164,    80,   165,    -1,     3,    -1,   166,    -1,
     167,    -1,    68,    -1,    69,    -1,    69,     3,    -1,   168,
      -1,   169,    -1,   179,    -1,   180,    -1,   174,    -1,   183,
      -1,    66,    -1,    67,    -1,   165,    85,   165,    -1,    78,
      79,    -1,    78,   164,    79,    -1,    53,    70,    66,    -1,
      53,    70,     3,    -1,    54,    70,    66,    -1,    54,    70,
       3,    -1,    55,    70,    66,    -1,    55,    70,     3,    -1,
      66,    -1,    68,    -1,     3,    -1,    86,    52,    70,   173,
      80,   170,    80,   171,    80,   172,    87,    -1,    86,   170,
      80,   171,    80,   172,    87,    -1,    56,    70,    66,    -1,
      56,    70,     3,    -1,    57,    70,    66,    -1,    57,    70,
       3,    -1,    58,    70,    66,    -1,    58,    70,     3,    -1,
      59,    70,    66,    -1,    59,    70,     3,    -1,    86,   175,
      80,   176,    80,   177,    80,   178,    87,    -1,    86,    63,
      70,   173,    80,    62,    70,   165,    87,    -1,    86,    62,
      70,   165,    87,    -1,    60,    70,    66,    -1,    60,    70,
       3,    -1,    61,    70,    66,    -1,    61,    70,     3,    -1,
      86,    64,    70,   173,    80,   181,    80,   182,    87,    -1,
      86,   181,    80,   182,    87,    -1,    -1,    68,   186,    -1,
      -1,   186,    -1,    74,   187,    75,    -1,    68,    -1,   187,
      68,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   318,   318,   324,   326,   333,   337,   344,   349,   353,
     354,   355,   360,   364,   371,   401,   407,   413,   418,   425,
     435,   441,   450,   453,   460,   463,   466,   472,   498,   517,
     533,   535,   541,   542,   543,   544,   545,   546,   547,   548,
     549,   550,   551,   552,   557,   558,   564,   573,   574,   583,
     584,   585,   589,   590,   595,   596,   601,   607,   616,   622,
     628,   631,   634,   637,   640,   643,   646,   649,   652,   655,
     658,   661,   667,   668,   674,   680,   725,   726,   732,   738,
     762,   763,   769,   775,   800,   801,   810,   811,   820,   821,
     830,   831,   840,   841,   850,   851,   860,   861,   871,   872,
     878,   887,   888,   897,   898,   899,   904,   905,   906,   911,
     920,   925,   930,   935,   941,   947,   952,   957,   962,   978,
     983,   988,   989,   993,   997,  1005,  1006,  1007,  1012,  1015,
    1031,  1034,  1039,  1042,  1045,  1055,  1063,  1067,  1074,  1077,
    1080,  1087,  1093,  1113,  1116,  1125,  1128,  1137,  1140,  1143,
    1150,  1154,  1162,  1165,  1168,  1175,  1182,  1185,  1189,  1206,
    1210,  1216,  1217,  1221,  1227,  1239,  1240,  1244,  1250,  1262,
    1281,  1293,  1296,  1302,  1309,  1312,  1318,  1322,  1329,  1332,
    1334,  1337,  1341,  1346,  1351,  1354,  1357,  1360,  1363,  1366,
    1374,  1380,  1386,  1397,  1401,  1408,  1414,  1420,  1423,  1427,
    1430,  1434,  1437,  1441,  1445,  1454,  1466,  1472,  1476,  1482,
    1486,  1492,  1496,  1502,  1506,  1519,  1526,  1536,  1542,  1547,
    1553,  1556,  1564,  1575,  1578,  1586,  1589,  1595,  1601,  1604
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
  "STATE_COOKIE", "PARAMETER", "MYSCTP", "TYPE", "FLAGS", "LEN",
  "MYSUPPORTED_EXTENSIONS", "TYPES", "TAG", "A_RWND", "OS", "IS", "TSN",
  "MYSID", "SSN", "PPID", "CUM_TSN", "GAPS", "DUPS", "SRTO_ASSOC_ID",
  "SRTO_INITIAL", "SRTO_MAX", "SRTO_MIN", "SINIT_NUM_OSTREAMS",
  "SINIT_MAX_INSTREAMS", "SINIT_MAX_ATTEMPTS", "SINIT_MAX_INIT_TIMEO",
  "MYSACK_DELAY", "SACK_FREQ", "ASSOC_VALUE", "ASSOC_ID", "SACK_ASSOC_ID",
  "MYFLOAT", "INTEGER", "HEX_INTEGER", "MYWORD", "MYSTRING", "'='", "'+'",
  "'*'", "'~'", "'('", "')'", "':'", "';'", "'['", "']'", "','", "'<'",
  "'>'", "'.'", "'-'", "'|'", "'{'", "'}'", "$accept", "script",
  "opt_options", "options", "option", "option_flag", "option_value",
  "events", "event", "event_time", "time", "action", "packet_spec",
  "tcp_packet_spec", "udp_packet_spec", "sctp_packet_spec",
  "sctp_chunk_list", "sctp_chunk", "opt_flags", "opt_len", "opt_val",
  "byte_list", "chunk_types_list", "byte", "chunk_type", "opt_data_flags",
  "opt_abort_flags", "opt_shutdown_complete_flags", "opt_tag",
  "opt_a_rwnd", "opt_os", "opt_is", "opt_tsn", "opt_sid", "opt_ssn",
  "opt_ppid", "opt_cum_tsn", "opt_gaps", "opt_dups",
  "sctp_data_chunk_spec", "sctp_init_chunk_spec",
  "sctp_init_ack_chunk_spec", "sctp_sack_chunk_spec",
  "sctp_heartbeat_chunk_spec", "sctp_heartbeat_ack_chunk_spec",
  "sctp_abort_chunk_spec", "sctp_shutdown_chunk_spec",
  "sctp_shutdown_ack_chunk_spec", "sctp_cookie_echo_chunk_spec",
  "sctp_cookie_ack_chunk_spec", "sctp_shutdown_complete_chunk_spec",
  "opt_parameter_list", "sctp_parameter_list", "sctp_parameter",
  "sctp_heartbeat_information_parameter",
  "sctp_supported_extensions_parameter", "sctp_state_cookie_parameter",
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
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
      61,    43,    42,   126,    40,    41,    58,    59,    91,    93,
      44,    60,    62,    46,    45,   124,   123,   125
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    88,    89,    90,    90,    91,    91,    92,    93,    94,
      94,    94,    95,    95,    96,    97,    97,    97,    97,    97,
      98,    98,    99,    99,   100,   100,   100,   101,   102,   103,
     104,   104,   105,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   106,   106,   106,   107,   107,   108,
     108,   108,   109,   109,   110,   110,   111,   111,   112,   112,
     112,   112,   112,   112,   112,   112,   112,   112,   112,   112,
     112,   112,   113,   113,   113,   113,   114,   114,   114,   114,
     115,   115,   115,   115,   116,   116,   117,   117,   118,   118,
     119,   119,   120,   120,   121,   121,   122,   122,   123,   123,
     123,   124,   124,   125,   125,   125,   126,   126,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   139,   140,   140,   141,   141,   141,   142,   142,
     143,   143,   144,   144,   144,   145,   146,   146,   147,   147,
     147,   147,   148,   149,   149,   150,   150,   151,   151,   151,
     152,   152,   153,   153,   153,   153,   153,   153,   153,   154,
     154,   155,   155,   155,   156,   157,   157,   157,   158,   159,
     160,   161,   161,   162,   163,   163,   164,   164,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     166,   167,   168,   169,   169,   170,   170,   171,   171,   172,
     172,   173,   173,   173,   174,   174,   175,   175,   176,   176,
     177,   177,   178,   178,   179,   180,   180,   181,   181,   182,
     182,   183,   183,   184,   184,   185,   185,   186,   187,   187
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     0,     1,     1,     2,     3,     1,     1,
       1,     1,     1,     2,     2,     2,     1,     1,     3,     5,
       1,     1,     1,     1,     1,     1,     1,     6,     5,     4,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     3,     3,     3,     3,     3,
       5,     5,     1,     3,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     5,     5,     3,     5,     5,    14,
      15,    15,    12,     6,     6,     4,     6,     4,     8,     4,
       4,     2,     2,     1,     3,     1,     1,     1,     4,     6,
       6,     8,     4,    10,    10,     1,     1,     1,     1,     1,
       2,     1,     6,     0,     2,     0,     2,     0,     3,     3,
       1,     3,     1,     1,     2,     2,     1,     2,     5,     1,
       2,     0,     1,     3,     3,     0,     1,     3,     3,     3,
       7,     0,     2,     1,     2,     3,     1,     3,     1,     1,
       1,     1,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     2,     3,     3,     3,     3,     3,     3,
       3,     1,     1,     1,    11,     7,     3,     3,     3,     3,
       3,     3,     3,     3,     9,     9,     5,     3,     3,     3,
       3,     9,     5,     0,     2,     0,     1,     3,     1,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     8,     0,     0,     4,     5,     0,     1,    20,    21,
       0,    17,     2,    12,   171,    16,     6,     0,    15,    13,
       0,   136,   137,    14,    22,    24,    25,    26,     0,   135,
      23,     0,     0,     9,    10,    11,     7,     0,   172,     0,
       0,   138,   139,   141,     0,   173,     0,    18,     0,     0,
       0,   140,     0,   143,     0,     0,    19,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      29,    30,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,     0,     0,   145,   178,   190,   191,
     181,   182,   174,     0,     0,     0,   176,   179,   180,   184,
     185,   188,   186,   187,   189,     0,    28,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   144,     0,   147,   183,   193,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   175,     0,     0,
     223,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    31,     0,   146,
       0,    27,   194,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   177,   192,     0,   225,     0,     0,     0,
       0,     0,     0,     0,     0,   115,     0,   117,     0,   119,
       0,   120,     0,     0,     0,     0,     0,   152,     0,   153,
       0,   156,     0,   150,   203,   201,   202,     0,   196,   195,
     207,   206,   218,   217,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   224,   170,   226,    72,    74,    73,
      75,     0,     0,    44,    46,    45,     0,     0,     0,     0,
       0,     0,    76,    78,    77,    79,     0,     0,     0,    80,
      82,    81,    83,     0,   142,   149,   155,   154,     0,     0,
     157,   159,     0,   148,     0,   216,     0,     0,     0,     0,
       0,     0,     0,   222,   228,     0,     0,     0,     0,     0,
       0,     0,   113,   114,     0,   116,     0,     0,     0,     0,
     160,   151,     0,     0,     0,   198,   197,     0,     0,   209,
     208,     0,     0,   220,   219,   229,   227,    47,    48,     0,
       0,    84,    85,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   169,     0,     0,     0,     0,   205,
       0,     0,     0,     0,     0,     0,     0,   128,     0,     0,
     118,     0,   158,     0,     0,     0,   200,   199,   211,   210,
       0,     0,    92,    93,     0,     0,    86,    87,     0,     0,
       0,     0,    49,     0,     0,     0,     0,   215,   221,     0,
     214,     0,     0,     0,     0,     0,   129,     0,    57,    56,
       0,    52,     0,     0,     0,   213,   212,    94,    95,     0,
       0,    88,    89,     0,     0,     0,    50,    51,     0,   103,
     161,     0,     0,   204,     0,     0,     0,     0,     0,    53,
       0,     0,     0,   162,     0,   112,    96,    97,     0,     0,
      90,    91,     0,     0,   104,     0,   105,     0,   106,   165,
       0,   109,     0,     0,     0,   164,   163,     0,     0,     0,
     166,    98,    99,   100,   121,     0,     0,   122,   123,   125,
     127,   126,   110,   111,   107,     0,   108,     0,     0,     0,
       0,   168,   167,     0,     0,     0,   124,   132,     0,     0,
       0,     0,     0,     0,     0,     0,   130,    60,    61,    62,
      64,    65,    66,    67,    68,    69,    70,    71,    63,    59,
      58,     0,    54,     0,     0,     0,     0,     0,     0,   131,
      55,     0,     0,   133,   134
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,     4,     5,     6,    36,    12,    13,    14,
      15,    23,    24,    25,    26,    27,    70,    71,   144,   232,
     321,   380,   491,   381,   492,   142,   149,   155,   237,   314,
     359,   394,   310,   355,   390,   419,   247,   365,   402,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,   433,   447,   448,   449,   450,   451,    28,    29,    44,
      53,    86,   123,   161,   202,   203,   260,   412,   413,   439,
     440,   261,    30,    31,    46,    55,    95,    96,    97,    98,
      99,   100,   134,   218,   298,   207,   101,   135,   220,   302,
     351,   102,   103,   136,   222,   104,   176,   225,   224,   275
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -188
static const yytype_int16 yypact[] =
{
      -8,  -188,   161,   -35,    -8,  -188,   104,  -188,  -188,  -188,
     -45,  -188,   -35,  -188,     2,   105,  -188,   -27,   111,  -188,
     -45,  -188,  -188,  -188,  -188,  -188,  -188,  -188,    12,  -188,
    -188,   136,   -45,  -188,  -188,  -188,  -188,   141,  -188,   124,
     131,   134,  -188,  -188,   168,  -188,   162,  -188,   -45,   169,
     130,  -188,   163,   232,    -1,   170,  -188,   166,   160,   164,
     165,   167,   171,   172,   173,   174,   175,   176,   177,   178,
     180,  -188,  -188,  -188,  -188,  -188,  -188,  -188,  -188,  -188,
    -188,  -188,  -188,  -188,   181,   182,   238,  -188,  -188,  -188,
    -188,   243,  -188,     3,   123,   -57,   179,  -188,  -188,  -188,
    -188,  -188,  -188,  -188,  -188,    24,  -188,   221,   222,   222,
     222,   222,   223,   222,   222,   222,   222,   224,   222,   130,
     188,  -188,   197,   184,  -188,  -188,    66,   196,   198,   199,
     200,   201,   202,   203,   187,   194,   195,  -188,    24,    24,
     -53,   206,   204,   207,   205,   208,   209,   210,   211,   212,
     213,   215,   216,   218,   217,   219,   220,  -188,   214,  -188,
     193,  -188,  -188,     7,    10,    22,    35,    24,     7,     7,
     225,   226,   231,   179,   179,   227,   227,    32,   240,    45,
     241,   241,   256,   256,    37,  -188,   246,  -188,   240,  -188,
      40,  -188,   246,   228,   229,   233,   236,  -188,   290,  -188,
     239,  -188,   -56,  -188,  -188,  -188,  -188,   230,  -188,  -188,
    -188,  -188,  -188,  -188,   -34,   234,   235,   237,   242,   247,
     244,   248,   245,   251,  -188,  -188,  -188,  -188,  -188,  -188,
    -188,   250,   249,  -188,  -188,  -188,   253,   254,   255,   252,
     257,   258,  -188,  -188,  -188,  -188,   261,   259,   260,  -188,
    -188,  -188,  -188,   262,  -188,  -188,  -188,  -188,   267,   263,
     239,  -188,   157,  -188,   268,  -188,   264,   265,    43,   272,
      51,   270,    52,  -188,  -188,    88,    53,   271,    54,   266,
     266,     6,  -188,  -188,    55,  -188,   292,   266,   298,   275,
    -188,  -188,   269,   273,   274,  -188,  -188,   276,   277,  -188,
    -188,   278,   279,  -188,  -188,  -188,  -188,  -188,  -188,   280,
     281,  -188,  -188,   282,   283,   285,   287,   288,  -188,  -188,
     286,   291,   289,   294,  -188,   225,    24,   231,    56,  -188,
      57,   296,    58,   299,    60,   301,   301,  -188,   292,     0,
    -188,   297,  -188,   293,   112,   284,  -188,  -188,  -188,  -188,
     302,   295,  -188,  -188,   304,   300,  -188,  -188,   305,   303,
     306,   308,  -188,    47,   307,   309,   272,  -188,  -188,    61,
    -188,    94,   310,   122,   314,   314,  -188,   311,  -188,  -188,
     110,  -188,     1,   316,   312,  -188,  -188,  -188,  -188,   315,
     313,  -188,  -188,   318,   317,   320,  -188,  -188,   153,  -188,
     125,   321,   319,  -188,   126,   328,   127,   271,   271,  -188,
     322,   326,   143,  -188,     8,  -188,  -188,  -188,   324,   325,
    -188,  -188,   323,   323,  -188,   329,  -188,   330,  -188,   128,
      49,  -188,   138,   327,   331,  -188,  -188,   332,   333,   145,
    -188,  -188,  -188,  -188,  -188,   334,   335,   336,  -188,  -188,
    -188,  -188,  -188,  -188,  -188,   339,  -188,   341,    11,   338,
     -11,  -188,  -188,   340,   344,   345,  -188,  -188,   129,     9,
     337,   342,   346,   115,   348,   365,  -188,  -188,  -188,  -188,
    -188,  -188,  -188,  -188,  -188,  -188,  -188,  -188,  -188,  -188,
    -188,   147,  -188,   350,   351,   347,   115,   378,   381,  -188,
    -188,   349,   352,  -188,  -188
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -188,  -188,  -188,  -188,   388,  -188,  -188,  -188,   396,  -188,
      -3,  -188,  -188,  -188,  -188,  -188,  -188,   343,   100,  -187,
     -26,  -188,  -188,   -85,  -145,  -188,  -188,  -188,   353,  -107,
      17,    59,  -179,  -188,  -188,  -188,   354,  -188,  -188,  -188,
    -188,  -188,  -188,  -188,  -188,  -188,  -188,  -188,  -188,  -188,
    -188,    -5,  -188,   -37,    48,  -188,  -188,  -188,  -188,  -188,
    -188,  -188,  -188,  -188,  -188,   183,  -188,  -188,     5,  -188,
     -33,   186,  -188,  -188,  -188,  -188,   355,  -105,  -188,  -188,
    -188,  -188,   185,   102,    63,    64,  -188,  -188,  -188,  -188,
    -188,  -188,  -188,   189,   103,  -188,  -188,  -188,   356,  -188
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
     140,   248,    87,   362,   399,    20,    87,    18,     1,   316,
     204,   428,   472,   208,   463,   175,    39,    38,   137,   239,
       8,     9,   445,   138,   262,   210,   263,    87,   446,    47,
       8,     9,   139,   173,   174,   227,    10,    11,   212,    33,
     242,    34,    35,   249,   231,    56,   295,    40,   233,   464,
     377,   139,   441,   265,   299,   303,   307,   311,   318,   346,
     348,   352,   214,   356,   385,    88,    89,    90,    91,    88,
      89,    90,    91,   205,    92,   206,   209,    93,   363,   400,
      41,    93,   125,    21,    22,    94,   429,   473,   211,    94,
      88,    89,    90,    91,   317,    42,    43,   387,   228,   229,
     230,   213,    93,   243,   244,   245,   250,   251,   252,   296,
      94,   234,   235,   378,   379,   442,   443,   300,   304,   308,
     312,   319,   347,   349,   353,   391,   357,   386,   410,   416,
     420,   437,   470,   477,   478,   479,   480,   481,   482,   483,
     484,   444,   485,   486,   487,   162,   138,   488,    58,    59,
      60,    61,    62,    63,    64,    65,   305,    66,    67,    68,
     388,     7,    69,   306,   195,   196,   197,   198,   239,   199,
     200,   445,   201,   315,    17,   127,   128,   446,    32,   129,
     322,   489,   490,   130,    37,   131,   132,   133,   392,   397,
     398,   411,   417,   421,   438,   471,   194,   139,    49,   367,
     195,   196,   197,   198,    45,   199,   200,    50,   201,   145,
     146,   147,    48,   150,   151,   152,   153,    51,   156,   378,
     379,   344,   426,   427,   456,   457,   495,   496,   422,   423,
     240,   241,   215,   216,    52,    57,    54,    85,   107,    84,
     105,   106,   108,   109,   122,   110,   124,   120,   121,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   141,   143,
     148,   154,   158,   159,   139,   160,   163,   170,   164,   165,
     166,   167,   168,   169,   171,   172,   177,   179,   231,   217,
     193,   184,   236,   219,   178,   180,   239,   190,   181,   182,
     183,   185,   221,   186,   187,   246,   188,   189,   191,   256,
     192,   223,   257,   254,   258,   259,   320,   268,   313,   323,
     264,   255,   361,   409,   266,   267,   309,   270,   272,   274,
     276,   128,   269,   278,   271,   130,   293,   297,   301,   277,
     281,   284,   273,   288,   279,   280,   282,   283,   285,   289,
     286,   324,   287,   326,   358,   354,   328,   364,   330,   325,
     332,   500,   334,   360,   327,   350,   339,   389,   393,   331,
     342,   333,   493,   335,   329,   336,   337,   401,   338,   341,
     340,   368,   369,   366,   371,   373,   418,   382,   465,   494,
     372,   501,   370,   374,   502,   404,   375,   376,   406,   383,
     396,   414,    16,   405,   430,   435,   411,   407,   415,   403,
     408,   424,   425,   432,   431,   461,   452,   438,    19,   455,
     453,   454,   458,   459,   468,   469,   460,   474,   434,   467,
     497,   498,   475,   466,   462,   476,   499,   343,   503,   384,
     345,   504,   436,     0,   395,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   291,   290,     0,   126,   292,
       0,     0,     0,     0,     0,     0,   294,     0,     0,     0,
       0,     0,   157,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   226,     0,   238,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   253
};

static const yytype_int16 yycheck[] =
{
     105,   188,     3,     3,     3,     3,     3,    10,    16,     3,
       3,     3,     3,     3,     3,    68,     4,    20,    75,    30,
      65,    66,    33,    80,    80,     3,    82,     3,    39,    32,
      65,    66,    85,   138,   139,     3,    71,    72,     3,    66,
       3,    68,    69,     3,    38,    48,     3,    35,     3,    38,
       3,    85,     3,    87,     3,     3,     3,     3,     3,     3,
       3,     3,   167,     3,     3,    66,    67,    68,    69,    66,
      67,    68,    69,    66,    75,    68,    66,    78,    78,    78,
      68,    78,    79,    81,    82,    86,    78,    78,    66,    86,
      66,    67,    68,    69,   281,    83,    84,     3,    66,    67,
      68,    66,    78,    66,    67,    68,    66,    67,    68,    66,
      86,    66,    67,    66,    67,    66,    67,    66,    66,    66,
      66,    66,    66,    66,    66,     3,    66,    66,     3,     3,
       3,     3,     3,    18,    19,    20,    21,    22,    23,    24,
      25,     3,    27,    28,    29,    79,    80,    32,    18,    19,
      20,    21,    22,    23,    24,    25,    68,    27,    28,    29,
      66,     0,    32,    75,     7,     8,     9,    10,    30,    12,
      13,    33,    15,   280,    70,    52,    53,    39,    73,    56,
     287,    66,    67,    60,    73,    62,    63,    64,    66,    79,
      80,    66,    66,    66,    66,    66,     3,    85,    74,    87,
       7,     8,     9,    10,    68,    12,    13,    76,    15,   109,
     110,   111,    71,   113,   114,   115,   116,    83,   118,    66,
      67,   326,    79,    80,    79,    80,    79,    80,   407,   408,
     182,   183,   168,   169,    66,    66,    74,     5,    78,    76,
      70,    75,    78,    78,     6,    78,     3,    66,    66,    78,
      78,    78,    78,    78,    78,    78,    78,    77,    37,    37,
      37,    37,    74,    66,    85,    81,    70,    80,    70,    70,
      70,    70,    70,    70,    80,    80,    70,    70,    38,    54,
      66,    70,    41,    57,    80,    80,    30,    70,    80,    80,
      80,    79,    61,    80,    79,    49,    80,    79,    79,    66,
      80,    74,    66,    75,    14,    66,    14,    70,    42,    11,
      80,    82,   338,   398,    80,    80,    45,    70,    70,    68,
      70,    53,    80,    70,    80,    60,    62,    55,    58,    80,
      78,    70,    87,    66,    80,    80,    79,    79,    79,    76,
      80,    66,    80,    70,    43,    46,    70,    50,    70,    80,
      70,   496,    70,   336,    80,    59,    70,    47,    44,    80,
      66,    80,    14,    80,    87,    80,    79,    51,    80,    80,
      79,    87,    70,    80,    70,    70,    48,    70,    40,    14,
      80,     3,    87,    80,     3,    70,    80,    79,    70,    80,
      79,    70,     4,    80,    70,    66,    66,    80,    79,    87,
      80,    79,    76,    80,    79,    66,    79,    66,    12,    76,
      79,    79,    78,    78,    70,    70,    80,    80,   423,    79,
      70,    70,    80,   460,   457,    79,    79,   325,    79,   366,
     327,    79,   427,    -1,   375,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   262,   260,    -1,    93,   264,
      -1,    -1,    -1,    -1,    -1,    -1,   267,    -1,    -1,    -1,
      -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   176,    -1,   181,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   192
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    16,    89,    90,    91,    92,    93,     0,    65,    66,
      71,    72,    95,    96,    97,    98,    92,    70,    98,    96,
       3,    81,    82,    99,   100,   101,   102,   103,   145,   146,
     160,   161,    73,    66,    68,    69,    94,    73,    98,     4,
      35,    68,    83,    84,   147,    68,   162,    98,    71,    74,
      76,    83,    66,   148,    74,   163,    98,    66,    18,    19,
      20,    21,    22,    23,    24,    25,    27,    28,    29,    32,
     104,   105,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,    76,     5,   149,     3,    66,    67,
      68,    69,    75,    78,    86,   164,   165,   166,   167,   168,
     169,   174,   179,   180,   183,    70,    75,    78,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    78,    78,    77,
      66,    66,     6,   150,     3,    79,   164,    52,    53,    56,
      60,    62,    63,    64,   170,   175,   181,    75,    80,    85,
     165,    37,   113,    37,   106,   106,   106,   106,    37,   114,
     106,   106,   106,   106,    37,   115,   106,   105,    74,    66,
      81,   151,    79,    70,    70,    70,    70,    70,    70,    70,
      80,    80,    80,   165,   165,    68,   184,    70,    80,    70,
      80,    80,    80,    80,    70,    79,    80,    79,    80,    79,
      70,    79,    80,    66,     3,     7,     8,     9,    10,    12,
      13,    15,   152,   153,     3,    66,    68,   173,     3,    66,
       3,    66,     3,    66,   165,   173,   173,    54,   171,    57,
     176,    61,   182,    74,   186,   185,   186,     3,    66,    67,
      68,    38,   107,     3,    66,    67,    41,   116,   116,    30,
     142,   142,     3,    66,    67,    68,    49,   124,   107,     3,
      66,    67,    68,   124,    75,    82,    66,    66,    14,    66,
     154,   159,    80,    82,    80,    87,    80,    80,    70,    80,
      70,    80,    70,    87,    68,   187,    70,    80,    70,    80,
      80,    78,    79,    79,    70,    79,    80,    80,    66,    76,
     159,   153,   170,    62,   181,     3,    66,    55,   172,     3,
      66,    58,   177,     3,    66,    68,    75,     3,    66,    45,
     120,     3,    66,    42,   117,   117,     3,   107,     3,    66,
      14,   108,   117,    11,    66,    80,    70,    80,    70,    87,
      70,    80,    70,    80,    70,    80,    80,    79,    80,    70,
      79,    80,    66,   171,   165,   182,     3,    66,     3,    66,
      59,   178,     3,    66,    46,   121,     3,    66,    43,   118,
     118,   108,     3,    78,    50,   125,    80,    87,    87,    70,
      87,    70,    80,    70,    80,    80,    79,     3,    66,    67,
     109,   111,    70,    80,   172,     3,    66,     3,    66,    47,
     122,     3,    66,    44,   119,   119,    79,    79,    80,     3,
      78,    51,   126,    87,    70,    80,    70,    80,    80,   111,
       3,    66,   155,   156,    70,    79,     3,    66,    48,   123,
       3,    66,   120,   120,    79,    76,    79,    80,     3,    78,
      70,    79,    80,   139,   139,    66,   156,     3,    66,   157,
     158,     3,    66,    67,     3,    33,    39,   140,   141,   142,
     143,   144,    79,    79,    79,    76,    79,    80,    78,    78,
      80,    66,   158,     3,    38,    40,   141,    79,    70,    70,
       3,    66,     3,    78,    80,    80,    79,    18,    19,    20,
      21,    22,    23,    24,    25,    27,    28,    29,    32,    66,
      67,   110,   112,    14,    14,    79,    80,    70,    70,    79,
     112,     3,     3,    79,    79
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
#line 318 "parser.y"
    {
    (yyval.string) = NULL;    /* The parser output is in out_script */
;}
    break;

  case 3:
#line 324 "parser.y"
    { (yyval.option) = NULL;
    parse_and_finalize_config(invocation);;}
    break;

  case 4:
#line 326 "parser.y"
    {
    (yyval.option) = (yyvsp[(1) - (1)].option);
    parse_and_finalize_config(invocation);
;}
    break;

  case 5:
#line 333 "parser.y"
    {
    out_script->addOption((yyvsp[(1) - (1)].option));
    (yyval.option) = (yyvsp[(1) - (1)].option);    /* return the tail so we can append to it */
;}
    break;

  case 6:
#line 337 "parser.y"
    {
    out_script->addOption((yyvsp[(2) - (2)].option));
    (yyval.option) = (yyvsp[(2) - (2)].option);    /* return the tail so we can append to it */
;}
    break;

  case 7:
#line 344 "parser.y"
    {
    (yyval.option) = new PacketDrillOption((yyvsp[(1) - (3)].string), (yyvsp[(3) - (3)].string));
;}
    break;

  case 8:
#line 349 "parser.y"
    { (yyval.string) = (yyvsp[(1) - (1)].reserved); ;}
    break;

  case 9:
#line 353 "parser.y"
    { (yyval.string) = strdup(yytext); ;}
    break;

  case 10:
#line 354 "parser.y"
    { (yyval.string) = (yyvsp[(1) - (1)].string); ;}
    break;

  case 11:
#line 355 "parser.y"
    { (yyval.string) = (yyvsp[(1) - (1)].string); ;}
    break;

  case 12:
#line 360 "parser.y"
    {
    out_script->addEvent((yyvsp[(1) - (1)].event));    /* save pointer to event list as output of parser */
    (yyval.event) = (yyvsp[(1) - (1)].event);    /* return the tail so that we can append to it */
;}
    break;

  case 13:
#line 364 "parser.y"
    {
    out_script->addEvent((yyvsp[(2) - (2)].event));
    (yyval.event) = (yyvsp[(2) - (2)].event);    /* return the tail so that we can append to it */
;}
    break;

  case 14:
#line 371 "parser.y"
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

  case 15:
#line 401 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(INVALID_EVENT);
    (yyval.event)->setLineNumber((yylsp[(2) - (2)]).first_line);
    (yyval.event)->setEventTime((yyvsp[(2) - (2)].time_usecs));
    (yyval.event)->setTimeType(RELATIVE_TIME);
;}
    break;

  case 16:
#line 407 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(INVALID_EVENT);
    (yyval.event)->setLineNumber((yylsp[(1) - (1)]).first_line);
    (yyval.event)->setEventTime((yyvsp[(1) - (1)].time_usecs));
    (yyval.event)->setTimeType(ABSOLUTE_TIME);
;}
    break;

  case 17:
#line 413 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(INVALID_EVENT);
    (yyval.event)->setLineNumber((yylsp[(1) - (1)]).first_line);
    (yyval.event)->setTimeType(ANY_TIME);
;}
    break;

  case 18:
#line 418 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(INVALID_EVENT);
    (yyval.event)->setLineNumber((yylsp[(1) - (3)]).first_line);
    (yyval.event)->setTimeType(ABSOLUTE_RANGE_TIME);
    (yyval.event)->setEventTime((yyvsp[(1) - (3)].time_usecs));
    (yyval.event)->setEventTimeEnd((yyvsp[(3) - (3)].time_usecs));
;}
    break;

  case 19:
#line 425 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(INVALID_EVENT);
    (yyval.event)->setLineNumber((yylsp[(1) - (5)]).first_line);
    (yyval.event)->setTimeType(RELATIVE_RANGE_TIME);
    (yyval.event)->setEventTime((yyvsp[(2) - (5)].time_usecs));
    (yyval.event)->setEventTimeEnd((yyvsp[(5) - (5)].time_usecs));
;}
    break;

  case 20:
#line 435 "parser.y"
    {
    if ((yyvsp[(1) - (1)].floating) < 0) {
        printf("Semantic error: negative time\n");
    }
    (yyval.time_usecs) = (int64)((yyvsp[(1) - (1)].floating) * 1.0e6); /* convert float secs to s64 microseconds */
;}
    break;

  case 21:
#line 441 "parser.y"
    {
    if ((yyvsp[(1) - (1)].integer) < 0) {
        printf("Semantic error: negative time\n");
    }
    (yyval.time_usecs) = (int64)((yyvsp[(1) - (1)].integer) * 1000000); /* convert int secs to s64 microseconds */
;}
    break;

  case 22:
#line 450 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(PACKET_EVENT);  (yyval.event)->setPacket((yyvsp[(1) - (1)].packet));
;}
    break;

  case 23:
#line 453 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(SYSCALL_EVENT);
    (yyval.event)->setSyscall((yyvsp[(1) - (1)].syscall));
;}
    break;

  case 24:
#line 460 "parser.y"
    {
    (yyval.packet) = (yyvsp[(1) - (1)].packet);
;}
    break;

  case 25:
#line 463 "parser.y"
    {
    (yyval.packet) = (yyvsp[(1) - (1)].packet);
;}
    break;

  case 26:
#line 466 "parser.y"
    {
    (yyval.packet) = (yyvsp[(1) - (1)].packet);
;}
    break;

  case 27:
#line 472 "parser.y"
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

  case 28:
#line 498 "parser.y"
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

  case 29:
#line 517 "parser.y"
    {
    PacketDrillPacket *inner = NULL;
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

  case 30:
#line 533 "parser.y"
    { (yyval.sctp_chunk_list) = new cQueue("sctpChunkList");
                                   (yyval.sctp_chunk_list)->insert((cObject*)(yyvsp[(1) - (1)].sctp_chunk)); ;}
    break;

  case 31:
#line 535 "parser.y"
    { (yyval.sctp_chunk_list) = (yyvsp[(1) - (3)].sctp_chunk_list);
                                   (yyvsp[(1) - (3)].sctp_chunk_list)->insert((yyvsp[(3) - (3)].sctp_chunk)); ;}
    break;

  case 32:
#line 541 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 33:
#line 542 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 34:
#line 543 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 35:
#line 544 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 36:
#line 545 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 37:
#line 546 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 38:
#line 547 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 39:
#line 548 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 40:
#line 549 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 41:
#line 550 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 42:
#line 551 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 43:
#line 552 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 44:
#line 557 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 45:
#line 558 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: flags value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 46:
#line 564 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: flags value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 47:
#line 573 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 48:
#line 574 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: length value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 49:
#line 583 "parser.y"
    { (yyval.byte_list) = NULL; ;}
    break;

  case 50:
#line 584 "parser.y"
    { (yyval.byte_list) = NULL; ;}
    break;

  case 51:
#line 585 "parser.y"
    { (yyval.byte_list) = (yyvsp[(4) - (5)].byte_list); ;}
    break;

  case 52:
#line 589 "parser.y"
    { (yyval.byte_list) = new PacketDrillBytes((yyvsp[(1) - (1)].byte)); ;}
    break;

  case 53:
#line 590 "parser.y"
    { (yyval.byte_list) = (yyvsp[(1) - (3)].byte_list);
                       (yyvsp[(1) - (3)].byte_list)->appendByte((yyvsp[(3) - (3)].byte)); ;}
    break;

  case 54:
#line 595 "parser.y"
    { (yyval.byte_list) = new PacketDrillBytes((yyvsp[(1) - (1)].integer)); printf("new PacketDrillBytes created\n");;}
    break;

  case 55:
#line 596 "parser.y"
    { (yyval.byte_list) = (yyvsp[(1) - (3)].byte_list);
                       (yyvsp[(1) - (3)].byte_list)->appendByte((yyvsp[(3) - (3)].integer)); ;}
    break;

  case 56:
#line 601 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(1) - (1)].integer))) {
        printf("Semantic error: byte value out of range\n");
    }
    (yyval.byte) = (yyvsp[(1) - (1)].integer);
;}
    break;

  case 57:
#line 607 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(1) - (1)].integer))) {
        printf("Semantic error: byte value out of range\n");
    }
    (yyval.byte) = (yyvsp[(1) - (1)].integer);
;}
    break;

  case 58:
#line 616 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(1) - (1)].integer))) {
        printf("Semantic error: type value out of range\n");
    }
    (yyval.integer) = (yyvsp[(1) - (1)].integer);
;}
    break;

  case 59:
#line 622 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(1) - (1)].integer))) {
        printf("Semantic error: type value out of range\n");
    }
    (yyval.integer) = (yyvsp[(1) - (1)].integer);
;}
    break;

  case 60:
#line 628 "parser.y"
    {
    (yyval.integer) = SCTP_DATA_CHUNK_TYPE;
;}
    break;

  case 61:
#line 631 "parser.y"
    {
    (yyval.integer) = SCTP_INIT_CHUNK_TYPE;
;}
    break;

  case 62:
#line 634 "parser.y"
    {
    (yyval.integer) = SCTP_INIT_ACK_CHUNK_TYPE;
;}
    break;

  case 63:
#line 637 "parser.y"
    {
    (yyval.integer) = SCTP_SACK_CHUNK_TYPE;
;}
    break;

  case 64:
#line 640 "parser.y"
    {
    (yyval.integer) = SCTP_HEARTBEAT_CHUNK_TYPE;
;}
    break;

  case 65:
#line 643 "parser.y"
    {
    (yyval.integer) = SCTP_HEARTBEAT_ACK_CHUNK_TYPE;
;}
    break;

  case 66:
#line 646 "parser.y"
    {
    (yyval.integer) = SCTP_ABORT_CHUNK_TYPE;
;}
    break;

  case 67:
#line 649 "parser.y"
    {
    (yyval.integer) = SCTP_SHUTDOWN_CHUNK_TYPE;
;}
    break;

  case 68:
#line 652 "parser.y"
    {
    (yyval.integer) = SCTP_SHUTDOWN_ACK_CHUNK_TYPE;
;}
    break;

  case 69:
#line 655 "parser.y"
    {
    (yyval.integer) = SCTP_COOKIE_ECHO_CHUNK_TYPE;
;}
    break;

  case 70:
#line 658 "parser.y"
    {
    (yyval.integer) = SCTP_COOKIE_ACK_CHUNK_TYPE;
;}
    break;

  case 71:
#line 661 "parser.y"
    {
    (yyval.integer) = SCTP_SHUTDOWN_COMPLETE_CHUNK_TYPE;
;}
    break;

  case 72:
#line 667 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 73:
#line 668 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: flags value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 74:
#line 674 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: flags value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 75:
#line 680 "parser.y"
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

  case 76:
#line 725 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 77:
#line 726 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: flags value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 78:
#line 732 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: flags value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 79:
#line 738 "parser.y"
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

  case 80:
#line 762 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 81:
#line 763 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: flags value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 82:
#line 769 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: flags value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 83:
#line 775 "parser.y"
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

  case 84:
#line 800 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 85:
#line 801 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: tag value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 86:
#line 810 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 87:
#line 811 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: a_rwnd value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 88:
#line 820 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 89:
#line 821 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: os value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 90:
#line 830 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 91:
#line 831 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: is value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 92:
#line 840 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 93:
#line 841 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: tsn value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 94:
#line 850 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 95:
#line 851 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: sid value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 96:
#line 860 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 97:
#line 861 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: ssn value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 98:
#line 871 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 99:
#line 872 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: ppid value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 100:
#line 878 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: ppid value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 101:
#line 887 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 102:
#line 888 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: cum_tsn value out of range\n");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 103:
#line 897 "parser.y"
    { (yyval.sack_block_list) = NULL; ;}
    break;

  case 104:
#line 898 "parser.y"
    { (yyval.sack_block_list) = NULL; ;}
    break;

  case 105:
#line 899 "parser.y"
    { (yyval.sack_block_list) = (yyvsp[(4) - (5)].sack_block_list); ;}
    break;

  case 106:
#line 904 "parser.y"
    { (yyval.sack_block_list) = NULL; ;}
    break;

  case 107:
#line 905 "parser.y"
    { (yyval.sack_block_list) = NULL; ;}
    break;

  case 108:
#line 906 "parser.y"
    { (yyval.sack_block_list) = (yyvsp[(4) - (5)].sack_block_list); ;}
    break;

  case 109:
#line 911 "parser.y"
    {
    if (((yyvsp[(5) - (14)].integer) != -1) &&
        (!is_valid_u16((yyvsp[(5) - (14)].integer)) || ((yyvsp[(5) - (14)].integer) < SCTP_DATA_CHUNK_LENGTH))) {
        printf("Semantic error: length value out of range\n");
    }
    (yyval.sctp_chunk) = PacketDrill::buildDataChunk((yyvsp[(3) - (14)].integer), (yyvsp[(5) - (14)].integer), (yyvsp[(7) - (14)].integer), (yyvsp[(9) - (14)].integer), (yyvsp[(11) - (14)].integer), (yyvsp[(13) - (14)].integer));
;}
    break;

  case 110:
#line 920 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildInitChunk((yyvsp[(3) - (15)].integer), (yyvsp[(5) - (15)].integer), (yyvsp[(7) - (15)].integer), (yyvsp[(9) - (15)].integer), (yyvsp[(11) - (15)].integer), (yyvsp[(13) - (15)].integer), (yyvsp[(14) - (15)].expression_list));
;}
    break;

  case 111:
#line 925 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildInitAckChunk((yyvsp[(3) - (15)].integer), (yyvsp[(5) - (15)].integer), (yyvsp[(7) - (15)].integer), (yyvsp[(9) - (15)].integer), (yyvsp[(11) - (15)].integer), (yyvsp[(13) - (15)].integer), (yyvsp[(14) - (15)].expression_list));
;}
    break;

  case 112:
#line 930 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildSackChunk((yyvsp[(3) - (12)].integer), (yyvsp[(5) - (12)].integer), (yyvsp[(7) - (12)].integer), (yyvsp[(9) - (12)].sack_block_list), (yyvsp[(11) - (12)].sack_block_list));
;}
    break;

  case 113:
#line 935 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildHeartbeatChunk((yyvsp[(3) - (6)].integer), (yyvsp[(5) - (6)].sctp_parameter));
;}
    break;

  case 114:
#line 941 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildHeartbeatAckChunk((yyvsp[(3) - (6)].integer), (yyvsp[(5) - (6)].sctp_parameter));
;}
    break;

  case 115:
#line 947 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildAbortChunk((yyvsp[(3) - (4)].integer));
;}
    break;

  case 116:
#line 952 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildShutdownChunk((yyvsp[(3) - (6)].integer), (yyvsp[(5) - (6)].integer));
;}
    break;

  case 117:
#line 957 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildShutdownAckChunk((yyvsp[(3) - (4)].integer));
;}
    break;

  case 118:
#line 962 "parser.y"
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

  case 119:
#line 978 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildCookieAckChunk((yyvsp[(3) - (4)].integer));
;}
    break;

  case 120:
#line 983 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildShutdownCompleteChunk((yyvsp[(3) - (4)].integer));
;}
    break;

  case 121:
#line 988 "parser.y"
    { (yyval.expression_list) = NULL; ;}
    break;

  case 122:
#line 989 "parser.y"
    { (yyval.expression_list) = (yyvsp[(2) - (2)].expression_list); ;}
    break;

  case 123:
#line 993 "parser.y"
    {
    (yyval.expression_list) = new cQueue("sctp_parameter_list");
    (yyval.expression_list)->insert((yyvsp[(1) - (1)].sctp_parameter));
;}
    break;

  case 124:
#line 997 "parser.y"
    {
    (yyval.expression_list) = (yyvsp[(1) - (3)].expression_list);
    (yyval.expression_list)->insert((yyvsp[(3) - (3)].sctp_parameter));
;}
    break;

  case 125:
#line 1005 "parser.y"
    { (yyval.sctp_parameter) = (yyvsp[(1) - (1)].sctp_parameter); ;}
    break;

  case 126:
#line 1006 "parser.y"
    { (yyval.sctp_parameter) = (yyvsp[(1) - (1)].sctp_parameter); ;}
    break;

  case 127:
#line 1007 "parser.y"
    { (yyval.sctp_parameter) = (yyvsp[(1) - (1)].sctp_parameter); ;}
    break;

  case 128:
#line 1012 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(HEARTBEAT_INFORMATION, -1, NULL);
;}
    break;

  case 129:
#line 1015 "parser.y"
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
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(HEARTBEAT_INFORMATION, (yyvsp[(3) - (6)].integer), (yyvsp[(5) - (6)].byte_list));
;}
    break;

  case 130:
#line 1031 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(SUPPORTED_EXTENSIONS, -1, NULL);
;}
    break;

  case 131:
#line 1034 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(SUPPORTED_EXTENSIONS, (yyvsp[(6) - (8)].byte_list)->getListLength(), (yyvsp[(6) - (8)].byte_list));
;}
    break;

  case 132:
#line 1039 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(STATE_COOKIE, -1, NULL);
;}
    break;

  case 133:
#line 1042 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(STATE_COOKIE, -1, NULL);
;}
    break;

  case 134:
#line 1045 "parser.y"
    {
    if (((yyvsp[(5) - (10)].integer) < 4) || !is_valid_u32((yyvsp[(5) - (10)].integer))) {
        printf("Semantic error: len value out of range\n");
    }
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(STATE_COOKIE, (yyvsp[(5) - (10)].integer), NULL);
;}
    break;

  case 135:
#line 1055 "parser.y"
    {
    (yyval.packet) = new PacketDrillPacket();
    (yyval.packet)->setDirection((yyvsp[(1) - (1)].direction));
;}
    break;

  case 136:
#line 1063 "parser.y"
    {
    (yyval.direction) = DIRECTION_INBOUND;
    current_script_line = yylineno;
;}
    break;

  case 137:
#line 1067 "parser.y"
    {
    (yyval.direction) = DIRECTION_OUTBOUND;
    current_script_line = yylineno;
;}
    break;

  case 138:
#line 1074 "parser.y"
    {
    (yyval.string) = (yyvsp[(1) - (1)].string);
;}
    break;

  case 139:
#line 1077 "parser.y"
    {
    (yyval.string) = strdup(".");
;}
    break;

  case 140:
#line 1080 "parser.y"
    {
printf("parse MYWORD\n");
    asprintf(&((yyval.string)), "%s.", (yyvsp[(1) - (2)].string));
printf("after parse MYWORD\n");
    free((yyvsp[(1) - (2)].string));
printf("after free MYWORD\n");
;}
    break;

  case 141:
#line 1087 "parser.y"
    {
    (yyval.string) = strdup("");
;}
    break;

  case 142:
#line 1093 "parser.y"
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

  case 143:
#line 1113 "parser.y"
    {
    (yyval.sequence_number) = 0;
;}
    break;

  case 144:
#line 1116 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(2) - (2)].integer))) {
    printf("TCP ack sequence number out of range");
    }
    (yyval.sequence_number) = (yyvsp[(2) - (2)].integer);
;}
    break;

  case 145:
#line 1125 "parser.y"
    {
    (yyval.window) = -1;
;}
    break;

  case 146:
#line 1128 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(2) - (2)].integer))) {
        printf("TCP window value out of range");
    }
    (yyval.window) = (yyvsp[(2) - (2)].integer);
;}
    break;

  case 147:
#line 1137 "parser.y"
    {
    (yyval.tcp_options) = new cQueue("opt_tcp_options");
;}
    break;

  case 148:
#line 1140 "parser.y"
    {
    (yyval.tcp_options) = (yyvsp[(2) - (3)].tcp_options);
;}
    break;

  case 149:
#line 1143 "parser.y"
    {
    (yyval.tcp_options) = NULL; /* FLAG_OPTIONS_NOCHECK */
;}
    break;

  case 150:
#line 1150 "parser.y"
    {
    (yyval.tcp_options) = new cQueue("tcp_option");
    (yyval.tcp_options)->insert((yyvsp[(1) - (1)].tcp_option));
;}
    break;

  case 151:
#line 1154 "parser.y"
    {
    (yyval.tcp_options) = (yyvsp[(1) - (3)].tcp_options);
    (yyval.tcp_options)->insert((yyvsp[(3) - (3)].tcp_option));
;}
    break;

  case 152:
#line 1162 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_NOP, 1);
;}
    break;

  case 153:
#line 1165 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_EOL, 1);
;}
    break;

  case 154:
#line 1168 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_MAXSEG, TCPOLEN_MAXSEG);
    if (!is_valid_u16((yyvsp[(2) - (2)].integer))) {
        printf("mss value out of range");
    }
    (yyval.tcp_option)->setMss((yyvsp[(2) - (2)].integer));
;}
    break;

  case 155:
#line 1175 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_WINDOW, TCPOLEN_WINDOW);
    if (!is_valid_u8((yyvsp[(2) - (2)].integer))) {
        printf("window scale shift count out of range");
    }
    (yyval.tcp_option)->setWindowScale((yyvsp[(2) - (2)].integer));
;}
    break;

  case 156:
#line 1182 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_SACK_PERMITTED, TCPOLEN_SACK_PERMITTED);
;}
    break;

  case 157:
#line 1185 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_SACK, 2+8*(yyvsp[(2) - (2)].sack_block_list)->getLength());
    (yyval.tcp_option)->setBlockList((yyvsp[(2) - (2)].sack_block_list));
;}
    break;

  case 158:
#line 1189 "parser.y"
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

  case 159:
#line 1206 "parser.y"
    {
    (yyval.sack_block_list) = new cQueue("sack_block_list");
    (yyval.sack_block_list)->insert((yyvsp[(1) - (1)].sack_block));
;}
    break;

  case 160:
#line 1210 "parser.y"
    {
    (yyval.sack_block_list) = (yyvsp[(1) - (2)].sack_block_list); (yyvsp[(1) - (2)].sack_block_list)->insert((yyvsp[(2) - (2)].sack_block));
;}
    break;

  case 161:
#line 1216 "parser.y"
    { (yyval.sack_block_list) = new cQueue("gap_list");;}
    break;

  case 162:
#line 1217 "parser.y"
    {
    (yyval.sack_block_list) = new cQueue("gap_list");
    (yyval.sack_block_list)->insert((yyvsp[(1) - (1)].sack_block));
;}
    break;

  case 163:
#line 1221 "parser.y"
    {
    (yyval.sack_block_list) = (yyvsp[(1) - (3)].sack_block_list); (yyvsp[(1) - (3)].sack_block_list)->insert((yyvsp[(3) - (3)].sack_block));
;}
    break;

  case 164:
#line 1227 "parser.y"
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

  case 165:
#line 1239 "parser.y"
    { (yyval.sack_block_list) = new cQueue("dup_list");;}
    break;

  case 166:
#line 1240 "parser.y"
    {
    (yyval.sack_block_list) = new cQueue("dup_list");
    (yyval.sack_block_list)->insert((yyvsp[(1) - (1)].sack_block));
;}
    break;

  case 167:
#line 1244 "parser.y"
    {
    (yyval.sack_block_list) = (yyvsp[(1) - (3)].sack_block_list); (yyvsp[(1) - (3)].sack_block_list)->insert((yyvsp[(3) - (3)].sack_block));
;}
    break;

  case 168:
#line 1250 "parser.y"
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

  case 169:
#line 1262 "parser.y"
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

  case 170:
#line 1281 "parser.y"
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

  case 171:
#line 1293 "parser.y"
    {
    (yyval.time_usecs) = -1;
;}
    break;

  case 172:
#line 1296 "parser.y"
    {
    (yyval.time_usecs) = (yyvsp[(2) - (2)].time_usecs);
;}
    break;

  case 173:
#line 1302 "parser.y"
    {
    (yyval.string) = (yyvsp[(1) - (1)].string);
    current_script_line = yylineno;
;}
    break;

  case 174:
#line 1309 "parser.y"
    {
    (yyval.expression_list) = NULL;
;}
    break;

  case 175:
#line 1312 "parser.y"
    {
    (yyval.expression_list) = (yyvsp[(2) - (3)].expression_list);
;}
    break;

  case 176:
#line 1318 "parser.y"
    {
    (yyval.expression_list) = new cQueue("expressionList");
    (yyval.expression_list)->insert((cObject*)(yyvsp[(1) - (1)].expression));
;}
    break;

  case 177:
#line 1322 "parser.y"
    {
    (yyval.expression_list) = (yyvsp[(1) - (3)].expression_list);
    (yyvsp[(1) - (3)].expression_list)->insert((yyvsp[(3) - (3)].expression));
;}
    break;

  case 178:
#line 1329 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS);
;}
    break;

  case 179:
#line 1332 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression); ;}
    break;

  case 180:
#line 1334 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 181:
#line 1337 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_WORD);
    (yyval.expression)->setString((yyvsp[(1) - (1)].string));
;}
    break;

  case 182:
#line 1341 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_STRING);
    (yyval.expression)->setString((yyvsp[(1) - (1)].string));
    (yyval.expression)->setFormat("\"%s\"");
;}
    break;

  case 183:
#line 1346 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_STRING);
    (yyval.expression)->setString((yyvsp[(1) - (2)].string));
    (yyval.expression)->setFormat("\"%s\"...");
;}
    break;

  case 184:
#line 1351 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 185:
#line 1354 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 186:
#line 1357 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 187:
#line 1360 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 188:
#line 1363 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 189:
#line 1366 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 190:
#line 1374 "parser.y"
    {
    (yyval.expression) = new_integer_expression((yyvsp[(1) - (1)].integer), "%ld");
;}
    break;

  case 191:
#line 1380 "parser.y"
    {
    (yyval.expression) = new_integer_expression((yyvsp[(1) - (1)].integer), "%#lx");
;}
    break;

  case 192:
#line 1386 "parser.y"
    {    /* bitwise OR */
    (yyval.expression) = new PacketDrillExpression(EXPR_BINARY);
    struct binary_expression *binary = (struct binary_expression *) malloc(sizeof(struct binary_expression));
    binary->op = strdup("|");
    binary->lhs = (yyvsp[(1) - (3)].expression);
    binary->rhs = (yyvsp[(3) - (3)].expression);
    (yyval.expression)->setBinary(binary);
;}
    break;

  case 193:
#line 1397 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_LIST);
    (yyval.expression)->setList(NULL);
;}
    break;

  case 194:
#line 1401 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_LIST);
    (yyval.expression)->setList((yyvsp[(2) - (3)].expression_list));
;}
    break;

  case 195:
#line 1408 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        printf("srto_initial out of range\n");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 196:
#line 1414 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS);
;}
    break;

  case 197:
#line 1420 "parser.y"
    {
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 198:
#line 1423 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 199:
#line 1427 "parser.y"
    {
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 200:
#line 1430 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 201:
#line 1434 "parser.y"
    {
    (yyval.expression) = new_integer_expression((yyvsp[(1) - (1)].integer), "%u");
;}
    break;

  case 202:
#line 1437 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_WORD);
    (yyval.expression)->setString((yyvsp[(1) - (1)].string));
;}
    break;

  case 203:
#line 1441 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 204:
#line 1445 "parser.y"
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

  case 205:
#line 1454 "parser.y"
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

  case 206:
#line 1466 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: sinit_num_ostreams out of range\n");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 207:
#line 1472 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 208:
#line 1476 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: sinit_max_instreams out of range\n");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 209:
#line 1482 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 210:
#line 1486 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: sinit_max_attempts out of range\n");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 211:
#line 1492 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 212:
#line 1496 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: sinit_max_init_timeo out of range\n");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 213:
#line 1502 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 214:
#line 1507 "parser.y"
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

  case 215:
#line 1519 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_ASSOCVAL);
    struct sctp_assoc_value_expr *assocval = (struct sctp_assoc_value_expr *) malloc(sizeof(struct sctp_assoc_value_expr));
    assocval->assoc_id = (yyvsp[(4) - (9)].expression);
    assocval->assoc_value = (yyvsp[(8) - (9)].expression);
    (yyval.expression)->setAssocval(assocval);
;}
    break;

  case 216:
#line 1526 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_ASSOCVAL);
    struct sctp_assoc_value_expr *assocval = (struct sctp_assoc_value_expr *) malloc(sizeof(struct sctp_assoc_value_expr));
    assocval->assoc_id = new PacketDrillExpression(EXPR_ELLIPSIS);
    assocval->assoc_value = (yyvsp[(4) - (5)].expression);
    (yyval.expression)->setAssocval(assocval);
;}
    break;

  case 217:
#line 1536 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: sack_delay out of range\n");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 218:
#line 1542 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS);
;}
    break;

  case 219:
#line 1547 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        printf("Semantic error: sack_freq out of range\n");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 220:
#line 1553 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 221:
#line 1556 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_SACKINFO);
    struct sctp_sack_info_expr *sackinfo = (struct sctp_sack_info_expr *) malloc(sizeof(struct sctp_sack_info_expr));
    sackinfo->sack_assoc_id = (yyvsp[(4) - (9)].expression);
    sackinfo->sack_delay = (yyvsp[(6) - (9)].expression);
    sackinfo->sack_freq = (yyvsp[(8) - (9)].expression);
    (yyval.expression)->setSackinfo(sackinfo);
;}
    break;

  case 222:
#line 1564 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_SACKINFO);
    struct sctp_sack_info_expr *sackinfo = (struct sctp_sack_info_expr *) malloc(sizeof(struct sctp_sack_info_expr));
    sackinfo->sack_assoc_id = new PacketDrillExpression(EXPR_ELLIPSIS);
    sackinfo->sack_delay = (yyvsp[(2) - (5)].expression);
    sackinfo->sack_freq = (yyvsp[(4) - (5)].expression);
    (yyval.expression)->setSackinfo(sackinfo);
;}
    break;

  case 223:
#line 1575 "parser.y"
    {
    (yyval.errno_info) = NULL;
;}
    break;

  case 224:
#line 1578 "parser.y"
    {
    (yyval.errno_info) = (struct errno_spec*)malloc(sizeof(struct errno_spec));
    (yyval.errno_info)->errno_macro = (yyvsp[(1) - (2)].string);
    (yyval.errno_info)->strerror = (yyvsp[(2) - (2)].string);
;}
    break;

  case 225:
#line 1586 "parser.y"
    {
    (yyval.string) = NULL;
;}
    break;

  case 226:
#line 1589 "parser.y"
    {
    (yyval.string) = (yyvsp[(1) - (1)].string);
;}
    break;

  case 227:
#line 1595 "parser.y"
    {
    (yyval.string) = (yyvsp[(2) - (3)].string);
;}
    break;

  case 228:
#line 1601 "parser.y"
    {
    (yyval.string) = (yyvsp[(1) - (1)].string);
;}
    break;

  case 229:
#line 1604 "parser.y"
    {
    asprintf(&((yyval.string)), "%s %s", (yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string));
    free((yyvsp[(1) - (2)].string));
    free((yyvsp[(2) - (2)].string));
;}
    break;


/* Line 1267 of yacc.c.  */
#line 4120 "parser.cc"
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



