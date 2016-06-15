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
     ACK = 262,
     WIN = 263,
     WSCALE = 264,
     MSS = 265,
     NOP = 266,
     TIMESTAMP = 267,
     ECR = 268,
     EOL = 269,
     TCPSACK = 270,
     VAL = 271,
     SACKOK = 272,
     OPTION = 273,
     CHUNK = 274,
     MYDATA = 275,
     MYINIT = 276,
     MYINIT_ACK = 277,
     MYHEARTBEAT = 278,
     MYHEARTBEAT_ACK = 279,
     MYABORT = 280,
     MYSHUTDOWN = 281,
     MYSHUTDOWN_ACK = 282,
     MYERROR = 283,
     MYCOOKIE_ECHO = 284,
     MYCOOKIE_ACK = 285,
     MYSHUTDOWN_COMPLETE = 286,
     HEARTBEAT_INFORMATION = 287,
     CAUSE_INFO = 288,
     MYSACK = 289,
     STATE_COOKIE = 290,
     PARAMETER = 291,
     MYSCTP = 292,
     TYPE = 293,
     FLAGS = 294,
     LEN = 295,
     MYSUPPORTED_EXTENSIONS = 296,
     TYPES = 297,
     TAG = 298,
     A_RWND = 299,
     OS = 300,
     IS = 301,
     TSN = 302,
     MYSID = 303,
     SSN = 304,
     PPID = 305,
     CUM_TSN = 306,
     GAPS = 307,
     DUPS = 308,
     SRTO_ASSOC_ID = 309,
     SRTO_INITIAL = 310,
     SRTO_MAX = 311,
     SRTO_MIN = 312,
     SINIT_NUM_OSTREAMS = 313,
     SINIT_MAX_INSTREAMS = 314,
     SINIT_MAX_ATTEMPTS = 315,
     SINIT_MAX_INIT_TIMEO = 316,
     MYSACK_DELAY = 317,
     SACK_FREQ = 318,
     ASSOC_VALUE = 319,
     ASSOC_ID = 320,
     SACK_ASSOC_ID = 321,
     MYFLOAT = 322,
     INTEGER = 323,
     HEX_INTEGER = 324,
     MYWORD = 325,
     MYSTRING = 326
   };
#endif
/* Tokens.  */
#define ELLIPSIS 258
#define UDP 259
#define _HTONS_ 260
#define _HTONL_ 261
#define ACK 262
#define WIN 263
#define WSCALE 264
#define MSS 265
#define NOP 266
#define TIMESTAMP 267
#define ECR 268
#define EOL 269
#define TCPSACK 270
#define VAL 271
#define SACKOK 272
#define OPTION 273
#define CHUNK 274
#define MYDATA 275
#define MYINIT 276
#define MYINIT_ACK 277
#define MYHEARTBEAT 278
#define MYHEARTBEAT_ACK 279
#define MYABORT 280
#define MYSHUTDOWN 281
#define MYSHUTDOWN_ACK 282
#define MYERROR 283
#define MYCOOKIE_ECHO 284
#define MYCOOKIE_ACK 285
#define MYSHUTDOWN_COMPLETE 286
#define HEARTBEAT_INFORMATION 287
#define CAUSE_INFO 288
#define MYSACK 289
#define STATE_COOKIE 290
#define PARAMETER 291
#define MYSCTP 292
#define TYPE 293
#define FLAGS 294
#define LEN 295
#define MYSUPPORTED_EXTENSIONS 296
#define TYPES 297
#define TAG 298
#define A_RWND 299
#define OS 300
#define IS 301
#define TSN 302
#define MYSID 303
#define SSN 304
#define PPID 305
#define CUM_TSN 306
#define GAPS 307
#define DUPS 308
#define SRTO_ASSOC_ID 309
#define SRTO_INITIAL 310
#define SRTO_MAX 311
#define SRTO_MIN 312
#define SINIT_NUM_OSTREAMS 313
#define SINIT_MAX_INSTREAMS 314
#define SINIT_MAX_ATTEMPTS 315
#define SINIT_MAX_INIT_TIMEO 316
#define MYSACK_DELAY 317
#define SACK_FREQ 318
#define ASSOC_VALUE 319
#define ASSOC_ID 320
#define SACK_ASSOC_ID 321
#define MYFLOAT 322
#define INTEGER 323
#define HEX_INTEGER 324
#define MYWORD 325
#define MYSTRING 326




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
#line 223 "parser.y"
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
#line 489 "parser.cc"
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
#line 514 "parser.cc"

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
#define YYLAST   551

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  90
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  100
/* YYNRULES -- Number of rules.  */
#define YYNRULES  233
/* YYNRULES -- Number of states.  */
#define YYNSTATES  515

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   326

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      76,    77,    74,    73,    82,    86,    85,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    78,    79,
      83,    72,    84,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    80,     2,    81,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    88,    87,    89,    75,     2,     2,     2,
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
      65,    66,    67,    68,    69,    70,    71
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
     454,   459,   462,   463,   466,   468,   472,   474,   476,   478,
     483,   490,   497,   506,   511,   522,   533,   535,   537,   539,
     541,   543,   546,   548,   555,   556,   559,   560,   563,   564,
     568,   572,   574,   578,   580,   582,   585,   588,   590,   593,
     599,   601,   604,   605,   607,   611,   615,   616,   618,   622,
     626,   630,   638,   639,   642,   644,   647,   651,   653,   657,
     659,   661,   663,   668,   673,   678,   680,   682,   685,   687,
     689,   691,   693,   695,   697,   699,   701,   705,   708,   712,
     716,   720,   724,   728,   732,   736,   738,   740,   742,   754,
     762,   766,   770,   774,   778,   782,   786,   790,   794,   804,
     814,   820,   824,   828,   832,   836,   846,   852,   853,   856,
     857,   859,   863,   865
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      91,     0,    -1,    92,    97,    -1,    -1,    93,    -1,    94,
      -1,    93,    94,    -1,    95,    72,    96,    -1,    18,    -1,
      68,    -1,    70,    -1,    71,    -1,    98,    -1,    97,    98,
      -1,    99,   101,    -1,    73,   100,    -1,   100,    -1,    74,
      -1,   100,    75,   100,    -1,    73,   100,    75,    73,   100,
      -1,    67,    -1,    68,    -1,   102,    -1,   162,    -1,   103,
      -1,   104,    -1,   105,    -1,   147,   149,   150,   151,   152,
     153,    -1,   147,     4,    76,    68,    77,    -1,   147,    37,
      78,   106,    -1,   107,    -1,   106,    79,   107,    -1,   129,
      -1,   130,    -1,   131,    -1,   132,    -1,   133,    -1,   134,
      -1,   135,    -1,   136,    -1,   137,    -1,   138,    -1,   139,
      -1,   140,    -1,    39,    72,     3,    -1,    39,    72,    69,
      -1,    39,    72,    68,    -1,    40,    72,     3,    -1,    40,
      72,    68,    -1,    16,    72,     3,    -1,    16,    72,    80,
       3,    81,    -1,    16,    72,    80,   111,    81,    -1,   113,
      -1,   111,    82,   113,    -1,   114,    -1,   112,    82,   114,
      -1,    69,    -1,    68,    -1,    69,    -1,    68,    -1,    20,
      -1,    21,    -1,    22,    -1,    34,    -1,    23,    -1,    24,
      -1,    25,    -1,    26,    -1,    27,    -1,    29,    -1,    30,
      -1,    31,    -1,    39,    72,     3,    -1,    39,    72,    69,
      -1,    39,    72,    68,    -1,    39,    72,    70,    -1,    39,
      72,     3,    -1,    39,    72,    69,    -1,    39,    72,    68,
      -1,    39,    72,    70,    -1,    39,    72,     3,    -1,    39,
      72,    69,    -1,    39,    72,    68,    -1,    39,    72,    70,
      -1,    43,    72,     3,    -1,    43,    72,    68,    -1,    44,
      72,     3,    -1,    44,    72,    68,    -1,    45,    72,     3,
      -1,    45,    72,    68,    -1,    46,    72,     3,    -1,    46,
      72,    68,    -1,    47,    72,     3,    -1,    47,    72,    68,
      -1,    48,    72,     3,    -1,    48,    72,    68,    -1,    49,
      72,     3,    -1,    49,    72,    68,    -1,    50,    72,     3,
      -1,    50,    72,    68,    -1,    50,    72,    69,    -1,    51,
      72,     3,    -1,    51,    72,    68,    -1,    52,    72,     3,
      -1,    52,    72,    80,     3,    81,    -1,    52,    72,    80,
     157,    81,    -1,    53,    72,     3,    -1,    53,    72,    80,
       3,    81,    -1,    53,    72,    80,   159,    81,    -1,    20,
      80,   115,    82,   109,    82,   122,    82,   123,    82,   124,
      82,   125,    81,    -1,    21,    80,   108,    82,   118,    82,
     119,    82,   120,    82,   121,    82,   122,   141,    81,    -1,
      22,    80,   108,    82,   118,    82,   119,    82,   120,    82,
     121,    82,   122,   141,    81,    -1,    34,    80,   108,    82,
     126,    82,   119,    82,   127,    82,   128,    81,    -1,    23,
      80,   108,    82,   144,    81,    -1,    24,    80,   108,    82,
     144,    81,    -1,    25,    80,   116,    81,    -1,    26,    80,
     108,    82,   126,    81,    -1,    27,    80,   108,    81,    -1,
      29,    80,   108,    82,   109,    82,   110,    81,    -1,    30,
      80,   108,    81,    -1,    31,    80,   117,    81,    -1,    82,
       3,    -1,    -1,    82,   142,    -1,   143,    -1,   142,    82,
     143,    -1,   144,    -1,   146,    -1,   145,    -1,    32,    80,
       3,    81,    -1,    32,    80,   109,    82,   110,    81,    -1,
      41,    80,    42,    72,     3,    81,    -1,    41,    80,    42,
      72,    80,   112,    81,    81,    -1,    35,    80,     3,    81,
      -1,    35,    80,    40,    72,     3,    82,    16,    72,     3,
      81,    -1,    35,    80,    40,    72,    68,    82,    16,    72,
       3,    81,    -1,   148,    -1,    83,    -1,    84,    -1,    70,
      -1,    85,    -1,    70,    85,    -1,    86,    -1,    68,    78,
      68,    76,    68,    77,    -1,    -1,     7,    68,    -1,    -1,
       8,    68,    -1,    -1,    83,   154,    84,    -1,    83,     3,
      84,    -1,   155,    -1,   154,    82,   155,    -1,    11,    -1,
      14,    -1,    10,    68,    -1,     9,    68,    -1,    17,    -1,
      15,   156,    -1,    12,    16,    68,    13,    68,    -1,   161,
      -1,   156,   161,    -1,    -1,   158,    -1,   157,    82,   158,
      -1,    68,    78,    68,    -1,    -1,   160,    -1,   159,    82,
     160,    -1,    68,    78,    68,    -1,    68,    78,    68,    -1,
     163,   164,   165,    72,   167,   186,   187,    -1,    -1,     3,
     100,    -1,    70,    -1,    76,    77,    -1,    76,   166,    77,
      -1,   167,    -1,   166,    82,   167,    -1,     3,    -1,   168,
      -1,   169,    -1,     6,    76,    68,    77,    -1,     6,    76,
      69,    77,    -1,     5,    76,    68,    77,    -1,    70,    -1,
      71,    -1,    71,     3,    -1,   170,    -1,   171,    -1,   181,
      -1,   182,    -1,   176,    -1,   185,    -1,    68,    -1,    69,
      -1,   167,    87,   167,    -1,    80,    81,    -1,    80,   166,
      81,    -1,    55,    72,    68,    -1,    55,    72,     3,    -1,
      56,    72,    68,    -1,    56,    72,     3,    -1,    57,    72,
      68,    -1,    57,    72,     3,    -1,    68,    -1,    70,    -1,
       3,    -1,    88,    54,    72,   175,    82,   172,    82,   173,
      82,   174,    89,    -1,    88,   172,    82,   173,    82,   174,
      89,    -1,    58,    72,    68,    -1,    58,    72,     3,    -1,
      59,    72,    68,    -1,    59,    72,     3,    -1,    60,    72,
      68,    -1,    60,    72,     3,    -1,    61,    72,    68,    -1,
      61,    72,     3,    -1,    88,   177,    82,   178,    82,   179,
      82,   180,    89,    -1,    88,    65,    72,   175,    82,    64,
      72,   167,    89,    -1,    88,    64,    72,   167,    89,    -1,
      62,    72,    68,    -1,    62,    72,     3,    -1,    63,    72,
      68,    -1,    63,    72,     3,    -1,    88,    66,    72,   175,
      82,   183,    82,   184,    89,    -1,    88,   183,    82,   184,
      89,    -1,    -1,    70,   188,    -1,    -1,   188,    -1,    76,
     189,    77,    -1,    70,    -1,   189,    70,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   323,   323,   329,   331,   338,   342,   349,   354,   358,
     359,   360,   365,   369,   376,   406,   412,   418,   423,   430,
     440,   446,   455,   462,   469,   472,   475,   481,   507,   526,
     546,   548,   554,   555,   556,   557,   558,   559,   560,   561,
     562,   563,   564,   565,   570,   571,   577,   586,   587,   596,
     597,   598,   602,   603,   608,   609,   614,   620,   629,   635,
     641,   644,   647,   650,   653,   656,   659,   662,   665,   668,
     671,   674,   680,   681,   687,   693,   737,   738,   744,   750,
     773,   774,   780,   786,   810,   811,   820,   821,   830,   831,
     840,   841,   850,   851,   860,   861,   870,   871,   881,   882,
     888,   897,   898,   907,   908,   909,   914,   915,   916,   921,
     930,   935,   940,   945,   951,   957,   962,   967,   972,   988,
     993,   998,   999,  1000,  1004,  1008,  1016,  1017,  1018,  1023,
    1026,  1042,  1045,  1050,  1053,  1056,  1066,  1074,  1078,  1085,
    1088,  1091,  1098,  1104,  1124,  1127,  1136,  1139,  1148,  1151,
    1154,  1161,  1165,  1173,  1176,  1179,  1186,  1193,  1196,  1200,
    1217,  1221,  1227,  1228,  1232,  1238,  1250,  1251,  1255,  1261,
    1273,  1292,  1304,  1307,  1313,  1320,  1323,  1329,  1333,  1340,
    1343,  1345,  1348,  1354,  1360,  1366,  1370,  1375,  1380,  1383,
    1386,  1389,  1392,  1395,  1403,  1409,  1415,  1426,  1430,  1437,
    1443,  1449,  1452,  1456,  1459,  1463,  1466,  1470,  1474,  1483,
    1495,  1501,  1505,  1511,  1515,  1521,  1525,  1531,  1535,  1548,
    1555,  1565,  1571,  1576,  1582,  1585,  1593,  1604,  1607,  1615,
    1618,  1624,  1630,  1633
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ELLIPSIS", "UDP", "_HTONS_", "_HTONL_",
  "ACK", "WIN", "WSCALE", "MSS", "NOP", "TIMESTAMP", "ECR", "EOL",
  "TCPSACK", "VAL", "SACKOK", "OPTION", "CHUNK", "MYDATA", "MYINIT",
  "MYINIT_ACK", "MYHEARTBEAT", "MYHEARTBEAT_ACK", "MYABORT", "MYSHUTDOWN",
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
     325,   326,    61,    43,    42,   126,    40,    41,    58,    59,
      91,    93,    44,    60,    62,    46,    45,   124,   123,   125
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    90,    91,    92,    92,    93,    93,    94,    95,    96,
      96,    96,    97,    97,    98,    99,    99,    99,    99,    99,
     100,   100,   101,   101,   102,   102,   102,   103,   104,   105,
     106,   106,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   108,   108,   108,   109,   109,   110,
     110,   110,   111,   111,   112,   112,   113,   113,   114,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   115,   115,   115,   115,   116,   116,   116,   116,
     117,   117,   117,   117,   118,   118,   119,   119,   120,   120,
     121,   121,   122,   122,   123,   123,   124,   124,   125,   125,
     125,   126,   126,   127,   127,   127,   128,   128,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   141,   141,   142,   142,   143,   143,   143,   144,
     144,   145,   145,   146,   146,   146,   147,   148,   148,   149,
     149,   149,   149,   150,   151,   151,   152,   152,   153,   153,
     153,   154,   154,   155,   155,   155,   155,   155,   155,   155,
     156,   156,   157,   157,   157,   158,   159,   159,   159,   160,
     161,   162,   163,   163,   164,   165,   165,   166,   166,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   167,   168,   169,   170,   171,   171,   172,
     172,   173,   173,   174,   174,   175,   175,   175,   176,   176,
     177,   177,   178,   178,   179,   179,   180,   180,   181,   182,
     182,   183,   183,   184,   184,   185,   185,   186,   186,   187,
     187,   188,   189,   189
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
       4,     2,     0,     2,     1,     3,     1,     1,     1,     4,
       6,     6,     8,     4,    10,    10,     1,     1,     1,     1,
       1,     2,     1,     6,     0,     2,     0,     2,     0,     3,
       3,     1,     3,     1,     1,     2,     2,     1,     2,     5,
       1,     2,     0,     1,     3,     3,     0,     1,     3,     3,
       3,     7,     0,     2,     1,     2,     3,     1,     3,     1,
       1,     1,     4,     4,     4,     1,     1,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     2,     3,     3,
       3,     3,     3,     3,     3,     1,     1,     1,    11,     7,
       3,     3,     3,     3,     3,     3,     3,     3,     9,     9,
       5,     3,     3,     3,     3,     9,     5,     0,     2,     0,
       1,     3,     1,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     8,     0,     0,     4,     5,     0,     1,    20,    21,
       0,    17,     2,    12,   172,    16,     6,     0,    15,    13,
       0,   137,   138,    14,    22,    24,    25,    26,     0,   136,
      23,     0,     0,     9,    10,    11,     7,     0,   173,     0,
       0,   139,   140,   142,     0,   174,     0,    18,     0,     0,
       0,   141,     0,   144,     0,     0,    19,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      29,    30,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,     0,     0,   146,   179,     0,     0,
     194,   195,   185,   186,   175,     0,     0,     0,   177,   180,
     181,   188,   189,   192,   190,   191,   193,     0,    28,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   145,     0,   148,     0,     0,   187,   197,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   176,     0,     0,   227,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    31,     0,   147,     0,    27,     0,     0,     0,   198,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     178,   196,     0,   229,     0,     0,     0,     0,     0,     0,
       0,     0,   115,     0,   117,     0,   119,     0,   120,     0,
       0,     0,     0,     0,   153,     0,   154,     0,   157,     0,
     151,   184,   182,   183,   207,   205,   206,     0,   200,   199,
     211,   210,   222,   221,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   228,   171,   230,    72,    74,    73,
      75,     0,     0,    44,    46,    45,     0,     0,     0,     0,
       0,     0,    76,    78,    77,    79,     0,     0,     0,    80,
      82,    81,    83,     0,   143,   150,   156,   155,     0,     0,
     158,   160,     0,   149,     0,   220,     0,     0,     0,     0,
       0,     0,     0,   226,   232,     0,     0,     0,     0,     0,
       0,     0,   113,   114,     0,   116,     0,     0,     0,     0,
     161,   152,     0,     0,     0,   202,   201,     0,     0,   213,
     212,     0,     0,   224,   223,   233,   231,    47,    48,     0,
       0,    84,    85,     0,     0,     0,     0,     0,   101,   102,
       0,     0,     0,     0,   170,     0,     0,     0,     0,   209,
       0,     0,     0,     0,     0,     0,     0,   129,     0,     0,
     118,     0,   159,     0,     0,     0,   204,   203,   215,   214,
       0,     0,    92,    93,     0,     0,    86,    87,     0,     0,
       0,     0,    49,     0,     0,     0,     0,   219,   225,     0,
     218,     0,     0,     0,     0,     0,   130,     0,    57,    56,
       0,    52,     0,     0,     0,   217,   216,    94,    95,     0,
       0,    88,    89,     0,     0,     0,    50,    51,     0,   103,
     162,     0,     0,   208,     0,     0,     0,     0,     0,    53,
       0,     0,     0,   163,     0,   112,    96,    97,     0,     0,
      90,    91,   122,   122,   104,     0,   105,     0,   106,   166,
       0,   109,     0,     0,     0,   165,   164,     0,     0,     0,
     167,    98,    99,   100,   121,     0,     0,   123,   124,   126,
     128,   127,   110,   111,   107,     0,   108,     0,     0,     0,
       0,   169,   168,     0,     0,     0,   125,   133,     0,     0,
       0,     0,     0,     0,     0,     0,   131,    60,    61,    62,
      64,    65,    66,    67,    68,    69,    70,    71,    63,    59,
      58,     0,    54,     0,     0,     0,     0,     0,     0,   132,
      55,     0,     0,   134,   135
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,     4,     5,     6,    36,    12,    13,    14,
      15,    23,    24,    25,    26,    27,    70,    71,   148,   242,
     331,   390,   501,   391,   502,   146,   153,   159,   247,   324,
     369,   404,   320,   365,   400,   429,   257,   375,   412,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,   443,   457,   458,   459,   460,   461,    28,    29,    44,
      53,    86,   125,   165,   209,   210,   270,   422,   423,   449,
     450,   271,    30,    31,    46,    55,    97,    98,    99,   100,
     101,   102,   138,   228,   308,   217,   103,   139,   230,   312,
     361,   104,   105,   140,   232,   106,   183,   235,   234,   285
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -205
static const yytype_int16 yypact[] =
{
       6,  -205,    27,   -34,     6,  -205,   -30,  -205,  -205,  -205,
     123,  -205,   -34,  -205,    17,   -37,  -205,    87,   -27,  -205,
     123,  -205,  -205,  -205,  -205,  -205,  -205,  -205,    13,  -205,
    -205,    84,   123,  -205,  -205,  -205,  -205,    93,  -205,   101,
      82,    95,  -205,  -205,   128,  -205,   109,  -205,   123,   151,
     181,  -205,    91,   202,     0,   152,  -205,   157,   148,   169,
     170,   172,   175,   176,   177,   178,   179,   180,   182,   183,
     158,  -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,
    -205,  -205,  -205,  -205,   193,   196,   243,  -205,   189,   190,
    -205,  -205,  -205,   264,  -205,     4,   117,   -59,   184,  -205,
    -205,  -205,  -205,  -205,  -205,  -205,  -205,    25,  -205,   229,
     230,   230,   230,   230,   231,   230,   230,   230,   230,   233,
     230,   181,   197,  -205,   206,   192,   208,   124,  -205,  -205,
     113,   205,   207,   209,   210,   211,   212,   213,   198,   204,
     214,  -205,    25,    25,   -55,   215,   216,   217,   218,   219,
     220,   222,   223,   224,   225,   227,   228,   232,   234,   235,
     236,  -205,   226,  -205,   153,  -205,   201,   237,   238,  -205,
       8,    23,    44,    50,    25,     8,     8,   241,   240,   246,
     184,   184,   244,   244,    34,   248,    18,   247,   247,   259,
     259,    38,  -205,   242,  -205,   248,  -205,    41,  -205,   242,
     245,   239,   249,   251,  -205,   276,  -205,   253,  -205,    94,
    -205,  -205,  -205,  -205,  -205,  -205,  -205,   250,  -205,  -205,
    -205,  -205,  -205,  -205,    49,   252,   254,   255,   256,   257,
     258,   261,   260,   265,  -205,  -205,  -205,  -205,  -205,  -205,
    -205,   267,   262,  -205,  -205,  -205,   269,   263,   266,   270,
     271,   272,  -205,  -205,  -205,  -205,   274,   273,   275,  -205,
    -205,  -205,  -205,   277,  -205,  -205,  -205,  -205,   279,   278,
     253,  -205,   221,  -205,   282,  -205,   287,   268,    51,   285,
      52,   283,    53,  -205,  -205,   -48,    54,   281,    55,   280,
     280,     5,  -205,  -205,    56,  -205,   295,   280,   290,   292,
    -205,  -205,   284,   286,   288,  -205,  -205,   289,   291,  -205,
    -205,   293,   294,  -205,  -205,  -205,  -205,  -205,  -205,   296,
     297,  -205,  -205,   299,   300,   301,   303,   304,  -205,  -205,
     302,   306,   307,   305,  -205,   241,    25,   246,    57,  -205,
      58,   308,    59,   314,    60,   310,   310,  -205,   295,    -1,
    -205,   311,  -205,   309,    97,   312,  -205,  -205,  -205,  -205,
     313,   315,  -205,  -205,   316,   317,  -205,  -205,   318,   320,
     321,   319,  -205,    46,   322,   323,   285,  -205,  -205,    61,
    -205,    62,   326,    63,   331,   331,  -205,   325,  -205,  -205,
     145,  -205,     1,   328,   324,  -205,  -205,  -205,  -205,   335,
     327,  -205,  -205,   336,   329,   330,  -205,  -205,   171,  -205,
      79,   338,   333,  -205,   129,   342,   130,   281,   281,  -205,
     334,   339,   160,  -205,     9,  -205,  -205,  -205,   344,   337,
    -205,  -205,   340,   340,  -205,   351,  -205,   352,  -205,   131,
      48,  -205,    11,   343,   345,  -205,  -205,   346,   347,   162,
    -205,  -205,  -205,  -205,  -205,   298,   341,   348,  -205,  -205,
    -205,  -205,  -205,  -205,  -205,   355,  -205,   360,   134,   353,
     -16,  -205,  -205,   350,   357,   361,  -205,  -205,   132,    10,
     354,   356,   358,   119,   377,   380,  -205,  -205,  -205,  -205,
    -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,
    -205,   164,  -205,   362,   363,   359,   119,   364,   369,  -205,
    -205,   365,   366,  -205,  -205
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -205,  -205,  -205,  -205,   393,  -205,  -205,  -205,   386,  -205,
     141,  -205,  -205,  -205,  -205,  -205,  -205,   191,   105,  -194,
     -23,  -205,  -205,   -82,  -175,  -205,  -205,  -205,   332,  -138,
      86,   -21,  -204,  -205,  -205,  -205,   349,  -205,  -205,  -205,
    -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,
    -205,    12,  -205,   -33,    64,  -205,  -205,  -205,  -205,  -205,
    -205,  -205,  -205,  -205,  -205,   185,  -205,  -205,     7,  -205,
     -26,   173,  -205,  -205,  -205,  -205,   367,  -107,  -205,  -205,
    -205,  -205,   168,   114,    74,    72,  -205,  -205,  -205,  -205,
    -205,  -205,  -205,   174,   111,  -205,  -205,  -205,   368,  -205
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
     144,   258,   372,    87,   409,    88,    89,    87,   326,    88,
      89,   214,   438,   482,   454,   182,   249,    39,   141,   455,
      20,   243,   315,   142,     1,   456,   218,     7,    87,   316,
      88,    89,   143,     8,     9,   180,   181,   237,    32,    10,
      11,   252,    17,   249,   259,   241,   455,   220,    37,   387,
      40,   451,   456,   222,   305,   309,   313,   317,   321,   328,
     356,   358,   362,   366,   395,   397,   401,   224,    90,    91,
      92,    93,    90,    91,    92,    93,   215,    94,   216,   373,
      95,   410,   420,    41,    95,   129,   244,   245,    96,   439,
     483,   219,    96,    90,    91,    92,    93,   327,    42,    43,
      21,    22,   238,   239,   240,    95,   253,   254,   255,   260,
     261,   262,   221,    96,   388,   389,   452,   453,   223,   306,
     310,   314,   318,   322,   329,   357,   359,   363,   367,   396,
     398,   402,   426,   430,   447,   480,   143,   473,   275,   487,
     488,   489,   490,   491,   492,   493,   494,   421,   495,   496,
     497,    18,   325,   498,    45,    33,   201,    34,    35,   332,
      50,    38,   202,   203,   204,   205,    48,   206,   207,    84,
     208,   131,   132,    47,   474,   133,   272,    49,   273,   134,
      51,   135,   136,   137,   143,    54,   377,   499,   500,    56,
       8,     9,   167,   168,   169,   142,    52,   427,   431,   448,
     481,    58,    59,    60,    61,    62,    63,    64,    65,    85,
      66,    67,    68,   432,   433,    69,   149,   150,   151,    57,
     154,   155,   156,   157,   107,   160,   407,   408,   109,   354,
     202,   203,   204,   205,   108,   206,   207,   121,   208,   388,
     389,   436,   437,   466,   467,   505,   506,   225,   226,   110,
     111,   124,   112,   250,   251,   113,   114,   115,   116,   117,
     118,   122,   119,   120,   123,   126,   127,   128,   145,   147,
     152,   143,   158,   162,   163,   164,   166,   170,   211,   171,
     177,   172,   173,   174,   175,   176,   178,   184,   241,   186,
     246,   249,   268,   256,   200,   191,   179,   227,   185,   229,
     187,   188,   189,   333,   190,   192,   197,   193,   194,   231,
     195,   330,   161,   196,   212,   213,   198,   266,   199,   267,
     233,   269,   264,   265,   323,   371,   419,   278,   319,   280,
     134,   510,   274,   282,   276,   284,   277,   132,   279,   286,
     281,   288,   307,   311,   287,   289,   294,   298,   290,   283,
     291,   303,   292,   293,   295,   368,   299,   296,   336,   297,
     334,   338,   364,   374,   405,   340,   335,   511,   342,   360,
     337,   344,   512,   352,   349,   399,   341,   403,   468,   343,
     339,   411,   345,   346,   347,   379,   348,   350,   381,   351,
     383,   376,   428,   503,   392,   475,   504,    16,    19,   382,
     386,   378,   384,   385,   380,   393,   406,   414,   416,   415,
     424,   417,   418,   413,   425,   434,   440,   435,   441,   445,
     421,   469,   442,   471,   462,   465,   463,   464,   448,   478,
     470,   477,   370,   479,   507,   508,   484,   476,   485,   486,
     509,   472,   302,   300,   446,   444,   513,   514,   355,   353,
     394,   304,     0,     0,     0,     0,     0,   301,     0,     0,
       0,     0,   130,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     248,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   263,     0,
       0,   236
};

static const yytype_int16 yycheck[] =
{
     107,   195,     3,     3,     3,     5,     6,     3,     3,     5,
       6,     3,     3,     3,     3,    70,    32,     4,    77,    35,
       3,     3,    70,    82,    18,    41,     3,     0,     3,    77,
       5,     6,    87,    67,    68,   142,   143,     3,    75,    73,
      74,     3,    72,    32,     3,    40,    35,     3,    75,     3,
      37,     3,    41,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,   174,    68,    69,
      70,    71,    68,    69,    70,    71,    68,    77,    70,    80,
      80,    80,     3,    70,    80,    81,    68,    69,    88,    80,
      80,    68,    88,    68,    69,    70,    71,   291,    85,    86,
      83,    84,    68,    69,    70,    80,    68,    69,    70,    68,
      69,    70,    68,    88,    68,    69,    68,    69,    68,    68,
      68,    68,    68,    68,    68,    68,    68,    68,    68,    68,
      68,    68,     3,     3,     3,     3,    87,     3,    89,    20,
      21,    22,    23,    24,    25,    26,    27,    68,    29,    30,
      31,    10,   290,    34,    70,    68,     3,    70,    71,   297,
      78,    20,     9,    10,    11,    12,    73,    14,    15,    78,
      17,    54,    55,    32,    40,    58,    82,    76,    84,    62,
      85,    64,    65,    66,    87,    76,    89,    68,    69,    48,
      67,    68,    68,    69,    81,    82,    68,    68,    68,    68,
      68,    20,    21,    22,    23,    24,    25,    26,    27,     7,
      29,    30,    31,   417,   418,    34,   111,   112,   113,    68,
     115,   116,   117,   118,    72,   120,    81,    82,    80,   336,
       9,    10,    11,    12,    77,    14,    15,    79,    17,    68,
      69,    81,    82,    81,    82,    81,    82,   175,   176,    80,
      80,     8,    80,   189,   190,    80,    80,    80,    80,    80,
      80,    68,    80,    80,    68,    76,    76,     3,    39,    39,
      39,    87,    39,    76,    68,    83,    68,    72,    77,    72,
      82,    72,    72,    72,    72,    72,    82,    72,    40,    72,
      43,    32,    16,    51,    68,    72,    82,    56,    82,    59,
      82,    82,    82,    13,    82,    81,    72,    82,    81,    63,
      82,    16,   121,    81,    77,    77,    81,    68,    82,    68,
      76,    68,    77,    84,    44,   348,   408,    72,    47,    72,
      62,   506,    82,    72,    82,    70,    82,    55,    82,    72,
      82,    72,    57,    60,    82,    82,    72,    68,    82,    89,
      80,    64,    81,    81,    81,    45,    78,    82,    72,    82,
      68,    72,    48,    52,   385,    72,    82,     3,    72,    61,
      82,    72,     3,    68,    72,    49,    82,    46,    80,    82,
      89,    53,    82,    82,    81,    72,    82,    81,    72,    82,
      72,    82,    50,    16,    72,    42,    16,     4,    12,    82,
      81,    89,    82,    82,    89,    82,    81,    72,    72,    82,
      72,    82,    82,    89,    81,    81,    72,    78,    81,    68,
      68,    80,    82,    68,    81,    78,    81,    81,    68,    72,
      82,    81,   346,    72,    72,    72,    82,   470,    82,    81,
      81,   467,   274,   270,   437,   433,    81,    81,   337,   335,
     376,   277,    -1,    -1,    -1,    -1,    -1,   272,    -1,    -1,
      -1,    -1,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   199,    -1,
      -1,   183
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    18,    91,    92,    93,    94,    95,     0,    67,    68,
      73,    74,    97,    98,    99,   100,    94,    72,   100,    98,
       3,    83,    84,   101,   102,   103,   104,   105,   147,   148,
     162,   163,    75,    68,    70,    71,    96,    75,   100,     4,
      37,    70,    85,    86,   149,    70,   164,   100,    73,    76,
      78,    85,    68,   150,    76,   165,   100,    68,    20,    21,
      22,    23,    24,    25,    26,    27,    29,    30,    31,    34,
     106,   107,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,    78,     7,   151,     3,     5,     6,
      68,    69,    70,    71,    77,    80,    88,   166,   167,   168,
     169,   170,   171,   176,   181,   182,   185,    72,    77,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    79,    68,    68,     8,   152,    76,    76,     3,    81,
     166,    54,    55,    58,    62,    64,    65,    66,   172,   177,
     183,    77,    82,    87,   167,    39,   115,    39,   108,   108,
     108,   108,    39,   116,   108,   108,   108,   108,    39,   117,
     108,   107,    76,    68,    83,   153,    68,    68,    69,    81,
      72,    72,    72,    72,    72,    72,    72,    82,    82,    82,
     167,   167,    70,   186,    72,    82,    72,    82,    82,    82,
      82,    72,    81,    82,    81,    82,    81,    72,    81,    82,
      68,     3,     9,    10,    11,    12,    14,    15,    17,   154,
     155,    77,    77,    77,     3,    68,    70,   175,     3,    68,
       3,    68,     3,    68,   167,   175,   175,    56,   173,    59,
     178,    63,   184,    76,   188,   187,   188,     3,    68,    69,
      70,    40,   109,     3,    68,    69,    43,   118,   118,    32,
     144,   144,     3,    68,    69,    70,    51,   126,   109,     3,
      68,    69,    70,   126,    77,    84,    68,    68,    16,    68,
     156,   161,    82,    84,    82,    89,    82,    82,    72,    82,
      72,    82,    72,    89,    70,   189,    72,    82,    72,    82,
      82,    80,    81,    81,    72,    81,    82,    82,    68,    78,
     161,   155,   172,    64,   183,     3,    68,    57,   174,     3,
      68,    60,   179,     3,    68,    70,    77,     3,    68,    47,
     122,     3,    68,    44,   119,   119,     3,   109,     3,    68,
      16,   110,   119,    13,    68,    82,    72,    82,    72,    89,
      72,    82,    72,    82,    72,    82,    82,    81,    82,    72,
      81,    82,    68,   173,   167,   184,     3,    68,     3,    68,
      61,   180,     3,    68,    48,   123,     3,    68,    45,   120,
     120,   110,     3,    80,    52,   127,    82,    89,    89,    72,
      89,    72,    82,    72,    82,    82,    81,     3,    68,    69,
     111,   113,    72,    82,   174,     3,    68,     3,    68,    49,
     124,     3,    68,    46,   121,   121,    81,    81,    82,     3,
      80,    53,   128,    89,    72,    82,    72,    82,    82,   113,
       3,    68,   157,   158,    72,    81,     3,    68,    50,   125,
       3,    68,   122,   122,    81,    78,    81,    82,     3,    80,
      72,    81,    82,   141,   141,    68,   158,     3,    68,   159,
     160,     3,    68,    69,     3,    35,    41,   142,   143,   144,
     145,   146,    81,    81,    81,    78,    81,    82,    80,    80,
      82,    68,   160,     3,    40,    42,   143,    81,    72,    72,
       3,    68,     3,    80,    82,    82,    81,    20,    21,    22,
      23,    24,    25,    26,    27,    29,    30,    31,    34,    68,
      69,   112,   114,    16,    16,    81,    82,    72,    72,    81,
     114,     3,     3,    81,    81
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
#line 323 "parser.y"
    {
    (yyval.string) = NULL;    /* The parser output is in out_script */
;}
    break;

  case 3:
#line 329 "parser.y"
    { (yyval.option) = NULL;
    parse_and_finalize_config(invocation);;}
    break;

  case 4:
#line 331 "parser.y"
    {
    (yyval.option) = (yyvsp[(1) - (1)].option);
    parse_and_finalize_config(invocation);
;}
    break;

  case 5:
#line 338 "parser.y"
    {
    out_script->addOption((yyvsp[(1) - (1)].option));
    (yyval.option) = (yyvsp[(1) - (1)].option);    /* return the tail so we can append to it */
;}
    break;

  case 6:
#line 342 "parser.y"
    {
    out_script->addOption((yyvsp[(2) - (2)].option));
    (yyval.option) = (yyvsp[(2) - (2)].option);    /* return the tail so we can append to it */
;}
    break;

  case 7:
#line 349 "parser.y"
    {
    (yyval.option) = new PacketDrillOption((yyvsp[(1) - (3)].string), (yyvsp[(3) - (3)].string));
;}
    break;

  case 8:
#line 354 "parser.y"
    { (yyval.string) = (yyvsp[(1) - (1)].reserved); ;}
    break;

  case 9:
#line 358 "parser.y"
    { (yyval.string) = strdup(yytext); ;}
    break;

  case 10:
#line 359 "parser.y"
    { (yyval.string) = (yyvsp[(1) - (1)].string); ;}
    break;

  case 11:
#line 360 "parser.y"
    { (yyval.string) = (yyvsp[(1) - (1)].string); ;}
    break;

  case 12:
#line 365 "parser.y"
    {
    out_script->addEvent((yyvsp[(1) - (1)].event));    /* save pointer to event list as output of parser */
    (yyval.event) = (yyvsp[(1) - (1)].event);    /* return the tail so that we can append to it */
;}
    break;

  case 13:
#line 369 "parser.y"
    {
    out_script->addEvent((yyvsp[(2) - (2)].event));
    (yyval.event) = (yyvsp[(2) - (2)].event);    /* return the tail so that we can append to it */
;}
    break;

  case 14:
#line 376 "parser.y"
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
#line 406 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(INVALID_EVENT);
    (yyval.event)->setLineNumber((yylsp[(2) - (2)]).first_line);
    (yyval.event)->setEventTime((yyvsp[(2) - (2)].time_usecs));
    (yyval.event)->setTimeType(RELATIVE_TIME);
;}
    break;

  case 16:
#line 412 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(INVALID_EVENT);
    (yyval.event)->setLineNumber((yylsp[(1) - (1)]).first_line);
    (yyval.event)->setEventTime((yyvsp[(1) - (1)].time_usecs));
    (yyval.event)->setTimeType(ABSOLUTE_TIME);
;}
    break;

  case 17:
#line 418 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(INVALID_EVENT);
    (yyval.event)->setLineNumber((yylsp[(1) - (1)]).first_line);
    (yyval.event)->setTimeType(ANY_TIME);
;}
    break;

  case 18:
#line 423 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(INVALID_EVENT);
    (yyval.event)->setLineNumber((yylsp[(1) - (3)]).first_line);
    (yyval.event)->setTimeType(ABSOLUTE_RANGE_TIME);
    (yyval.event)->setEventTime((yyvsp[(1) - (3)].time_usecs));
    (yyval.event)->setEventTimeEnd((yyvsp[(3) - (3)].time_usecs));
;}
    break;

  case 19:
#line 430 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(INVALID_EVENT);
    (yyval.event)->setLineNumber((yylsp[(1) - (5)]).first_line);
    (yyval.event)->setTimeType(RELATIVE_RANGE_TIME);
    (yyval.event)->setEventTime((yyvsp[(2) - (5)].time_usecs));
    (yyval.event)->setEventTimeEnd((yyvsp[(5) - (5)].time_usecs));
;}
    break;

  case 20:
#line 440 "parser.y"
    {
    if ((yyvsp[(1) - (1)].floating) < 0) {
        semantic_error("negative time");
    }
    (yyval.time_usecs) = (int64)((yyvsp[(1) - (1)].floating) * 1.0e6); /* convert float secs to s64 microseconds */
;}
    break;

  case 21:
#line 446 "parser.y"
    {
    if ((yyvsp[(1) - (1)].integer) < 0) {
        semantic_error("negative time");
    }
    (yyval.time_usecs) = (int64)((yyvsp[(1) - (1)].integer) * 1000000); /* convert int secs to s64 microseconds */
;}
    break;

  case 22:
#line 455 "parser.y"
    {
    if ((yyvsp[(1) - (1)].packet)) {
        (yyval.event) = new PacketDrillEvent(PACKET_EVENT);  (yyval.event)->setPacket((yyvsp[(1) - (1)].packet));
    } else {
        (yyval.event) = NULL;
    }
;}
    break;

  case 23:
#line 462 "parser.y"
    {
    (yyval.event) = new PacketDrillEvent(SYSCALL_EVENT);
    (yyval.event)->setSyscall((yyvsp[(1) - (1)].syscall));
;}
    break;

  case 24:
#line 469 "parser.y"
    {
    (yyval.packet) = (yyvsp[(1) - (1)].packet);
;}
    break;

  case 25:
#line 472 "parser.y"
    {
    (yyval.packet) = (yyvsp[(1) - (1)].packet);
;}
    break;

  case 26:
#line 475 "parser.y"
    {
    (yyval.packet) = (yyvsp[(1) - (1)].packet);
;}
    break;

  case 27:
#line 481 "parser.y"
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
#line 507 "parser.y"
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
#line 526 "parser.y"
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

  case 30:
#line 546 "parser.y"
    { (yyval.sctp_chunk_list) = new cQueue("sctpChunkList");
                                   (yyval.sctp_chunk_list)->insert((cObject*)(yyvsp[(1) - (1)].sctp_chunk)); ;}
    break;

  case 31:
#line 548 "parser.y"
    { (yyval.sctp_chunk_list) = (yyvsp[(1) - (3)].sctp_chunk_list);
                                   (yyvsp[(1) - (3)].sctp_chunk_list)->insert((yyvsp[(3) - (3)].sctp_chunk)); ;}
    break;

  case 32:
#line 554 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 33:
#line 555 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 34:
#line 556 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 35:
#line 557 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 36:
#line 558 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 37:
#line 559 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 38:
#line 560 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 39:
#line 561 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 40:
#line 562 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 41:
#line 563 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 42:
#line 564 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 43:
#line 565 "parser.y"
    { (yyval.sctp_chunk) = (yyvsp[(1) - (1)].sctp_chunk); ;}
    break;

  case 44:
#line 570 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 45:
#line 571 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        semantic_error("flags value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 46:
#line 577 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        semantic_error("flags value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 47:
#line 586 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 48:
#line 587 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("length value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 49:
#line 596 "parser.y"
    { (yyval.byte_list) = NULL; ;}
    break;

  case 50:
#line 597 "parser.y"
    { (yyval.byte_list) = NULL; ;}
    break;

  case 51:
#line 598 "parser.y"
    { (yyval.byte_list) = (yyvsp[(4) - (5)].byte_list); ;}
    break;

  case 52:
#line 602 "parser.y"
    { (yyval.byte_list) = new PacketDrillBytes((yyvsp[(1) - (1)].byte)); ;}
    break;

  case 53:
#line 603 "parser.y"
    { (yyval.byte_list) = (yyvsp[(1) - (3)].byte_list);
                       (yyvsp[(1) - (3)].byte_list)->appendByte((yyvsp[(3) - (3)].byte)); ;}
    break;

  case 54:
#line 608 "parser.y"
    { (yyval.byte_list) = new PacketDrillBytes((yyvsp[(1) - (1)].integer)); printf("new PacketDrillBytes created\n");;}
    break;

  case 55:
#line 609 "parser.y"
    { (yyval.byte_list) = (yyvsp[(1) - (3)].byte_list);
                       (yyvsp[(1) - (3)].byte_list)->appendByte((yyvsp[(3) - (3)].integer)); ;}
    break;

  case 56:
#line 614 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(1) - (1)].integer))) {
        semantic_error("byte value out of range");
    }
    (yyval.byte) = (yyvsp[(1) - (1)].integer);
;}
    break;

  case 57:
#line 620 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(1) - (1)].integer))) {
        semantic_error("byte value out of range");
    }
    (yyval.byte) = (yyvsp[(1) - (1)].integer);
;}
    break;

  case 58:
#line 629 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(1) - (1)].integer))) {
        semantic_error("type value out of range");
    }
    (yyval.integer) = (yyvsp[(1) - (1)].integer);
;}
    break;

  case 59:
#line 635 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(1) - (1)].integer))) {
        semantic_error("type value out of range");
    }
    (yyval.integer) = (yyvsp[(1) - (1)].integer);
;}
    break;

  case 60:
#line 641 "parser.y"
    {
    (yyval.integer) = SCTP_DATA_CHUNK_TYPE;
;}
    break;

  case 61:
#line 644 "parser.y"
    {
    (yyval.integer) = SCTP_INIT_CHUNK_TYPE;
;}
    break;

  case 62:
#line 647 "parser.y"
    {
    (yyval.integer) = SCTP_INIT_ACK_CHUNK_TYPE;
;}
    break;

  case 63:
#line 650 "parser.y"
    {
    (yyval.integer) = SCTP_SACK_CHUNK_TYPE;
;}
    break;

  case 64:
#line 653 "parser.y"
    {
    (yyval.integer) = SCTP_HEARTBEAT_CHUNK_TYPE;
;}
    break;

  case 65:
#line 656 "parser.y"
    {
    (yyval.integer) = SCTP_HEARTBEAT_ACK_CHUNK_TYPE;
;}
    break;

  case 66:
#line 659 "parser.y"
    {
    (yyval.integer) = SCTP_ABORT_CHUNK_TYPE;
;}
    break;

  case 67:
#line 662 "parser.y"
    {
    (yyval.integer) = SCTP_SHUTDOWN_CHUNK_TYPE;
;}
    break;

  case 68:
#line 665 "parser.y"
    {
    (yyval.integer) = SCTP_SHUTDOWN_ACK_CHUNK_TYPE;
;}
    break;

  case 69:
#line 668 "parser.y"
    {
    (yyval.integer) = SCTP_COOKIE_ECHO_CHUNK_TYPE;
;}
    break;

  case 70:
#line 671 "parser.y"
    {
    (yyval.integer) = SCTP_COOKIE_ACK_CHUNK_TYPE;
;}
    break;

  case 71:
#line 674 "parser.y"
    {
    (yyval.integer) = SCTP_SHUTDOWN_COMPLETE_CHUNK_TYPE;
;}
    break;

  case 72:
#line 680 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 73:
#line 681 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        semantic_error("flags value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 74:
#line 687 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        semantic_error("flags value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 75:
#line 693 "parser.y"
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

  case 76:
#line 737 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 77:
#line 738 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        semantic_error("flags value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 78:
#line 744 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        semantic_error("flags value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 79:
#line 750 "parser.y"
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

  case 80:
#line 773 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 81:
#line 774 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        semantic_error("flags value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 82:
#line 780 "parser.y"
    {
    if (!is_valid_u8((yyvsp[(3) - (3)].integer))) {
        semantic_error("flags value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 83:
#line 786 "parser.y"
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

  case 84:
#line 810 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 85:
#line 811 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("tag value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 86:
#line 820 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 87:
#line 821 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("a_rwnd value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 88:
#line 830 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 89:
#line 831 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("os value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 90:
#line 840 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 91:
#line 841 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("is value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 92:
#line 850 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 93:
#line 851 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("tsn value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 94:
#line 860 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 95:
#line 861 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("sid value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 96:
#line 870 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 97:
#line 871 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("ssn value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 98:
#line 881 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 99:
#line 882 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("ppid value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 100:
#line 888 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("ppid value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 101:
#line 897 "parser.y"
    { (yyval.integer) = -1; ;}
    break;

  case 102:
#line 898 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("cum_tsn value out of range");
    }
    (yyval.integer) = (yyvsp[(3) - (3)].integer);
;}
    break;

  case 103:
#line 907 "parser.y"
    { (yyval.sack_block_list) = NULL; ;}
    break;

  case 104:
#line 908 "parser.y"
    { (yyval.sack_block_list) = NULL; ;}
    break;

  case 105:
#line 909 "parser.y"
    { (yyval.sack_block_list) = (yyvsp[(4) - (5)].sack_block_list); ;}
    break;

  case 106:
#line 914 "parser.y"
    { (yyval.sack_block_list) = NULL; ;}
    break;

  case 107:
#line 915 "parser.y"
    { (yyval.sack_block_list) = NULL; ;}
    break;

  case 108:
#line 916 "parser.y"
    { (yyval.sack_block_list) = (yyvsp[(4) - (5)].sack_block_list); ;}
    break;

  case 109:
#line 921 "parser.y"
    {
    if (((yyvsp[(5) - (14)].integer) != -1) &&
        (!is_valid_u16((yyvsp[(5) - (14)].integer)) || ((yyvsp[(5) - (14)].integer) < SCTP_DATA_CHUNK_LENGTH))) {
        semantic_error("length value out of range");
    }
    (yyval.sctp_chunk) = PacketDrill::buildDataChunk((yyvsp[(3) - (14)].integer), (yyvsp[(5) - (14)].integer), (yyvsp[(7) - (14)].integer), (yyvsp[(9) - (14)].integer), (yyvsp[(11) - (14)].integer), (yyvsp[(13) - (14)].integer));
;}
    break;

  case 110:
#line 930 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildInitChunk((yyvsp[(3) - (15)].integer), (yyvsp[(5) - (15)].integer), (yyvsp[(7) - (15)].integer), (yyvsp[(9) - (15)].integer), (yyvsp[(11) - (15)].integer), (yyvsp[(13) - (15)].integer), (yyvsp[(14) - (15)].expression_list));
;}
    break;

  case 111:
#line 935 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildInitAckChunk((yyvsp[(3) - (15)].integer), (yyvsp[(5) - (15)].integer), (yyvsp[(7) - (15)].integer), (yyvsp[(9) - (15)].integer), (yyvsp[(11) - (15)].integer), (yyvsp[(13) - (15)].integer), (yyvsp[(14) - (15)].expression_list));
;}
    break;

  case 112:
#line 940 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildSackChunk((yyvsp[(3) - (12)].integer), (yyvsp[(5) - (12)].integer), (yyvsp[(7) - (12)].integer), (yyvsp[(9) - (12)].sack_block_list), (yyvsp[(11) - (12)].sack_block_list));
;}
    break;

  case 113:
#line 945 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildHeartbeatChunk((yyvsp[(3) - (6)].integer), (yyvsp[(5) - (6)].sctp_parameter));
;}
    break;

  case 114:
#line 951 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildHeartbeatAckChunk((yyvsp[(3) - (6)].integer), (yyvsp[(5) - (6)].sctp_parameter));
;}
    break;

  case 115:
#line 957 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildAbortChunk((yyvsp[(3) - (4)].integer));
;}
    break;

  case 116:
#line 962 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildShutdownChunk((yyvsp[(3) - (6)].integer), (yyvsp[(5) - (6)].integer));
;}
    break;

  case 117:
#line 967 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildShutdownAckChunk((yyvsp[(3) - (4)].integer));
;}
    break;

  case 118:
#line 972 "parser.y"
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

  case 119:
#line 988 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildCookieAckChunk((yyvsp[(3) - (4)].integer));
;}
    break;

  case 120:
#line 993 "parser.y"
    {
    (yyval.sctp_chunk) = PacketDrill::buildShutdownCompleteChunk((yyvsp[(3) - (4)].integer));
;}
    break;

  case 121:
#line 998 "parser.y"
    { (yyval.expression_list) = NULL; ;}
    break;

  case 122:
#line 999 "parser.y"
    { (yyval.expression_list) = new cQueue("empty"); ;}
    break;

  case 123:
#line 1000 "parser.y"
    { (yyval.expression_list) = (yyvsp[(2) - (2)].expression_list); ;}
    break;

  case 124:
#line 1004 "parser.y"
    {
    (yyval.expression_list) = new cQueue("sctp_parameter_list");
    (yyval.expression_list)->insert((yyvsp[(1) - (1)].sctp_parameter));
;}
    break;

  case 125:
#line 1008 "parser.y"
    {
    (yyval.expression_list) = (yyvsp[(1) - (3)].expression_list);
    (yyval.expression_list)->insert((yyvsp[(3) - (3)].sctp_parameter));
;}
    break;

  case 126:
#line 1016 "parser.y"
    { (yyval.sctp_parameter) = (yyvsp[(1) - (1)].sctp_parameter); ;}
    break;

  case 127:
#line 1017 "parser.y"
    { (yyval.sctp_parameter) = (yyvsp[(1) - (1)].sctp_parameter); ;}
    break;

  case 128:
#line 1018 "parser.y"
    { (yyval.sctp_parameter) = (yyvsp[(1) - (1)].sctp_parameter); ;}
    break;

  case 129:
#line 1023 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(HEARTBEAT_INFORMATION, -1, NULL);
;}
    break;

  case 130:
#line 1026 "parser.y"
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

  case 131:
#line 1042 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(SUPPORTED_EXTENSIONS, -1, NULL);
;}
    break;

  case 132:
#line 1045 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(SUPPORTED_EXTENSIONS, (yyvsp[(6) - (8)].byte_list)->getListLength(), (yyvsp[(6) - (8)].byte_list));
;}
    break;

  case 133:
#line 1050 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(STATE_COOKIE, -1, NULL);
;}
    break;

  case 134:
#line 1053 "parser.y"
    {
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(STATE_COOKIE, -1, NULL);
;}
    break;

  case 135:
#line 1056 "parser.y"
    {
    if (((yyvsp[(5) - (10)].integer) < 4) || !is_valid_u32((yyvsp[(5) - (10)].integer))) {
        semantic_error("len value out of range");
    }
    (yyval.sctp_parameter) = new PacketDrillSctpParameter(STATE_COOKIE, (yyvsp[(5) - (10)].integer), NULL);
;}
    break;

  case 136:
#line 1066 "parser.y"
    {
    (yyval.packet) = new PacketDrillPacket();
    (yyval.packet)->setDirection((yyvsp[(1) - (1)].direction));
;}
    break;

  case 137:
#line 1074 "parser.y"
    {
    (yyval.direction) = DIRECTION_INBOUND;
    current_script_line = yylineno;
;}
    break;

  case 138:
#line 1078 "parser.y"
    {
    (yyval.direction) = DIRECTION_OUTBOUND;
    current_script_line = yylineno;
;}
    break;

  case 139:
#line 1085 "parser.y"
    {
    (yyval.string) = (yyvsp[(1) - (1)].string);
;}
    break;

  case 140:
#line 1088 "parser.y"
    {
    (yyval.string) = strdup(".");
;}
    break;

  case 141:
#line 1091 "parser.y"
    {
printf("parse MYWORD\n");
    asprintf(&((yyval.string)), "%s.", (yyvsp[(1) - (2)].string));
printf("after parse MYWORD\n");
    free((yyvsp[(1) - (2)].string));
printf("after free MYWORD\n");
;}
    break;

  case 142:
#line 1098 "parser.y"
    {
    (yyval.string) = strdup("");
;}
    break;

  case 143:
#line 1104 "parser.y"
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

  case 144:
#line 1124 "parser.y"
    {
    (yyval.sequence_number) = 0;
;}
    break;

  case 145:
#line 1127 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(2) - (2)].integer))) {
    printf("TCP ack sequence number out of range");
    }
    (yyval.sequence_number) = (yyvsp[(2) - (2)].integer);
;}
    break;

  case 146:
#line 1136 "parser.y"
    {
    (yyval.window) = -1;
;}
    break;

  case 147:
#line 1139 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(2) - (2)].integer))) {
        printf("TCP window value out of range");
    }
    (yyval.window) = (yyvsp[(2) - (2)].integer);
;}
    break;

  case 148:
#line 1148 "parser.y"
    {
    (yyval.tcp_options) = new cQueue("opt_tcp_options");
;}
    break;

  case 149:
#line 1151 "parser.y"
    {
    (yyval.tcp_options) = (yyvsp[(2) - (3)].tcp_options);
;}
    break;

  case 150:
#line 1154 "parser.y"
    {
    (yyval.tcp_options) = NULL; /* FLAG_OPTIONS_NOCHECK */
;}
    break;

  case 151:
#line 1161 "parser.y"
    {
    (yyval.tcp_options) = new cQueue("tcp_option");
    (yyval.tcp_options)->insert((yyvsp[(1) - (1)].tcp_option));
;}
    break;

  case 152:
#line 1165 "parser.y"
    {
    (yyval.tcp_options) = (yyvsp[(1) - (3)].tcp_options);
    (yyval.tcp_options)->insert((yyvsp[(3) - (3)].tcp_option));
;}
    break;

  case 153:
#line 1173 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_NOP, 1);
;}
    break;

  case 154:
#line 1176 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_EOL, 1);
;}
    break;

  case 155:
#line 1179 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_MAXSEG, TCPOLEN_MAXSEG);
    if (!is_valid_u16((yyvsp[(2) - (2)].integer))) {
        printf("mss value out of range");
    }
    (yyval.tcp_option)->setMss((yyvsp[(2) - (2)].integer));
;}
    break;

  case 156:
#line 1186 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_WINDOW, TCPOLEN_WINDOW);
    if (!is_valid_u8((yyvsp[(2) - (2)].integer))) {
        printf("window scale shift count out of range");
    }
    (yyval.tcp_option)->setWindowScale((yyvsp[(2) - (2)].integer));
;}
    break;

  case 157:
#line 1193 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_SACK_PERMITTED, TCPOLEN_SACK_PERMITTED);
;}
    break;

  case 158:
#line 1196 "parser.y"
    {
    (yyval.tcp_option) = new PacketDrillTcpOption(TCPOPT_SACK, 2+8*(yyvsp[(2) - (2)].sack_block_list)->getLength());
    (yyval.tcp_option)->setBlockList((yyvsp[(2) - (2)].sack_block_list));
;}
    break;

  case 159:
#line 1200 "parser.y"
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

  case 160:
#line 1217 "parser.y"
    {
    (yyval.sack_block_list) = new cQueue("sack_block_list");
    (yyval.sack_block_list)->insert((yyvsp[(1) - (1)].sack_block));
;}
    break;

  case 161:
#line 1221 "parser.y"
    {
    (yyval.sack_block_list) = (yyvsp[(1) - (2)].sack_block_list); (yyvsp[(1) - (2)].sack_block_list)->insert((yyvsp[(2) - (2)].sack_block));
;}
    break;

  case 162:
#line 1227 "parser.y"
    { (yyval.sack_block_list) = new cQueue("gap_list");;}
    break;

  case 163:
#line 1228 "parser.y"
    {
    (yyval.sack_block_list) = new cQueue("gap_list");
    (yyval.sack_block_list)->insert((yyvsp[(1) - (1)].sack_block));
;}
    break;

  case 164:
#line 1232 "parser.y"
    {
    (yyval.sack_block_list) = (yyvsp[(1) - (3)].sack_block_list); (yyvsp[(1) - (3)].sack_block_list)->insert((yyvsp[(3) - (3)].sack_block));
;}
    break;

  case 165:
#line 1238 "parser.y"
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

  case 166:
#line 1250 "parser.y"
    { (yyval.sack_block_list) = new cQueue("dup_list");;}
    break;

  case 167:
#line 1251 "parser.y"
    {
    (yyval.sack_block_list) = new cQueue("dup_list");
    (yyval.sack_block_list)->insert((yyvsp[(1) - (1)].sack_block));
;}
    break;

  case 168:
#line 1255 "parser.y"
    {
    (yyval.sack_block_list) = (yyvsp[(1) - (3)].sack_block_list); (yyvsp[(1) - (3)].sack_block_list)->insert((yyvsp[(3) - (3)].sack_block));
;}
    break;

  case 169:
#line 1261 "parser.y"
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

  case 170:
#line 1273 "parser.y"
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

  case 171:
#line 1292 "parser.y"
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

  case 172:
#line 1304 "parser.y"
    {
    (yyval.time_usecs) = -1;
;}
    break;

  case 173:
#line 1307 "parser.y"
    {
    (yyval.time_usecs) = (yyvsp[(2) - (2)].time_usecs);
;}
    break;

  case 174:
#line 1313 "parser.y"
    {
    (yyval.string) = (yyvsp[(1) - (1)].string);
    current_script_line = yylineno;
;}
    break;

  case 175:
#line 1320 "parser.y"
    {
    (yyval.expression_list) = NULL;
;}
    break;

  case 176:
#line 1323 "parser.y"
    {
    (yyval.expression_list) = (yyvsp[(2) - (3)].expression_list);
;}
    break;

  case 177:
#line 1329 "parser.y"
    {
    (yyval.expression_list) = new cQueue("expressionList");
    (yyval.expression_list)->insert((cObject*)(yyvsp[(1) - (1)].expression));
;}
    break;

  case 178:
#line 1333 "parser.y"
    {
    (yyval.expression_list) = (yyvsp[(1) - (3)].expression_list);
    (yyvsp[(1) - (3)].expression_list)->insert((yyvsp[(3) - (3)].expression));
;}
    break;

  case 179:
#line 1340 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS);
;}
    break;

  case 180:
#line 1343 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression); ;}
    break;

  case 181:
#line 1345 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 182:
#line 1348 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (4)].integer))) {
        semantic_error("number out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (4)].integer), "%lu");
;}
    break;

  case 183:
#line 1354 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (4)].integer))) {
        semantic_error("number out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (4)].integer), "%lu");
;}
    break;

  case 184:
#line 1360 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (4)].integer))) {
        semantic_error("number out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (4)].integer), "%lu");
;}
    break;

  case 185:
#line 1366 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_WORD);
    (yyval.expression)->setString((yyvsp[(1) - (1)].string));
;}
    break;

  case 186:
#line 1370 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_STRING);
    (yyval.expression)->setString((yyvsp[(1) - (1)].string));
    (yyval.expression)->setFormat("\"%s\"");
;}
    break;

  case 187:
#line 1375 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_STRING);
    (yyval.expression)->setString((yyvsp[(1) - (2)].string));
    (yyval.expression)->setFormat("\"%s\"...");
;}
    break;

  case 188:
#line 1380 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 189:
#line 1383 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 190:
#line 1386 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 191:
#line 1389 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 192:
#line 1392 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 193:
#line 1395 "parser.y"
    {
    (yyval.expression) = (yyvsp[(1) - (1)].expression);
;}
    break;

  case 194:
#line 1403 "parser.y"
    {
    (yyval.expression) = new_integer_expression((yyvsp[(1) - (1)].integer), "%ld");
;}
    break;

  case 195:
#line 1409 "parser.y"
    {
    (yyval.expression) = new_integer_expression((yyvsp[(1) - (1)].integer), "%#lx");
;}
    break;

  case 196:
#line 1415 "parser.y"
    {    /* bitwise OR */
    (yyval.expression) = new PacketDrillExpression(EXPR_BINARY);
    struct binary_expression *binary = (struct binary_expression *) malloc(sizeof(struct binary_expression));
    binary->op = strdup("|");
    binary->lhs = (yyvsp[(1) - (3)].expression);
    binary->rhs = (yyvsp[(3) - (3)].expression);
    (yyval.expression)->setBinary(binary);
;}
    break;

  case 197:
#line 1426 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_LIST);
    (yyval.expression)->setList(NULL);
;}
    break;

  case 198:
#line 1430 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_LIST);
    (yyval.expression)->setList((yyvsp[(2) - (3)].expression_list));
;}
    break;

  case 199:
#line 1437 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        printf("srto_initial out of range\n");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 200:
#line 1443 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS);
;}
    break;

  case 201:
#line 1449 "parser.y"
    {
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 202:
#line 1452 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 203:
#line 1456 "parser.y"
    {
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 204:
#line 1459 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 205:
#line 1463 "parser.y"
    {
    (yyval.expression) = new_integer_expression((yyvsp[(1) - (1)].integer), "%u");
;}
    break;

  case 206:
#line 1466 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_WORD);
    (yyval.expression)->setString((yyvsp[(1) - (1)].string));
;}
    break;

  case 207:
#line 1470 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 208:
#line 1474 "parser.y"
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

  case 209:
#line 1483 "parser.y"
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

  case 210:
#line 1495 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("sinit_num_ostreams out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 211:
#line 1501 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 212:
#line 1505 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("sinit_max_instreams out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 213:
#line 1511 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 214:
#line 1515 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("sinit_max_attempts out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 215:
#line 1521 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 216:
#line 1525 "parser.y"
    {
    if (!is_valid_u16((yyvsp[(3) - (3)].integer))) {
        semantic_error("sinit_max_init_timeo out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%hu");
;}
    break;

  case 217:
#line 1531 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 218:
#line 1536 "parser.y"
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

  case 219:
#line 1548 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_ASSOCVAL);
    struct sctp_assoc_value_expr *assocval = (struct sctp_assoc_value_expr *) malloc(sizeof(struct sctp_assoc_value_expr));
    assocval->assoc_id = (yyvsp[(4) - (9)].expression);
    assocval->assoc_value = (yyvsp[(8) - (9)].expression);
    (yyval.expression)->setAssocval(assocval);
;}
    break;

  case 220:
#line 1555 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_ASSOCVAL);
    struct sctp_assoc_value_expr *assocval = (struct sctp_assoc_value_expr *) malloc(sizeof(struct sctp_assoc_value_expr));
    assocval->assoc_id = new PacketDrillExpression(EXPR_ELLIPSIS);
    assocval->assoc_value = (yyvsp[(4) - (5)].expression);
    (yyval.expression)->setAssocval(assocval);
;}
    break;

  case 221:
#line 1565 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("sack_delay out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 222:
#line 1571 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS);
;}
    break;

  case 223:
#line 1576 "parser.y"
    {
    if (!is_valid_u32((yyvsp[(3) - (3)].integer))) {
        semantic_error("sack_freq out of range");
    }
    (yyval.expression) = new_integer_expression((yyvsp[(3) - (3)].integer), "%u");
;}
    break;

  case 224:
#line 1582 "parser.y"
    { (yyval.expression) = new PacketDrillExpression(EXPR_ELLIPSIS); ;}
    break;

  case 225:
#line 1585 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_SACKINFO);
    struct sctp_sack_info_expr *sackinfo = (struct sctp_sack_info_expr *) malloc(sizeof(struct sctp_sack_info_expr));
    sackinfo->sack_assoc_id = (yyvsp[(4) - (9)].expression);
    sackinfo->sack_delay = (yyvsp[(6) - (9)].expression);
    sackinfo->sack_freq = (yyvsp[(8) - (9)].expression);
    (yyval.expression)->setSackinfo(sackinfo);
;}
    break;

  case 226:
#line 1593 "parser.y"
    {
    (yyval.expression) = new PacketDrillExpression(EXPR_SCTP_SACKINFO);
    struct sctp_sack_info_expr *sackinfo = (struct sctp_sack_info_expr *) malloc(sizeof(struct sctp_sack_info_expr));
    sackinfo->sack_assoc_id = new PacketDrillExpression(EXPR_ELLIPSIS);
    sackinfo->sack_delay = (yyvsp[(2) - (5)].expression);
    sackinfo->sack_freq = (yyvsp[(4) - (5)].expression);
    (yyval.expression)->setSackinfo(sackinfo);
;}
    break;

  case 227:
#line 1604 "parser.y"
    {
    (yyval.errno_info) = NULL;
;}
    break;

  case 228:
#line 1607 "parser.y"
    {
    (yyval.errno_info) = (struct errno_spec*)malloc(sizeof(struct errno_spec));
    (yyval.errno_info)->errno_macro = (yyvsp[(1) - (2)].string);
    (yyval.errno_info)->strerror = (yyvsp[(2) - (2)].string);
;}
    break;

  case 229:
#line 1615 "parser.y"
    {
    (yyval.string) = NULL;
;}
    break;

  case 230:
#line 1618 "parser.y"
    {
    (yyval.string) = (yyvsp[(1) - (1)].string);
;}
    break;

  case 231:
#line 1624 "parser.y"
    {
    (yyval.string) = (yyvsp[(2) - (3)].string);
;}
    break;

  case 232:
#line 1630 "parser.y"
    {
    (yyval.string) = (yyvsp[(1) - (1)].string);
;}
    break;

  case 233:
#line 1633 "parser.y"
    {
    asprintf(&((yyval.string)), "%s %s", (yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string));
    free((yyvsp[(1) - (2)].string));
    free((yyvsp[(2) - (2)].string));
;}
    break;


/* Line 1267 of yacc.c.  */
#line 4179 "parser.cc"
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



