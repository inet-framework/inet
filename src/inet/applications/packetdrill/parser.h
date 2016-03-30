/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

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
/* Line 1529 of yacc.c.  */
#line 217 "parser.h"
    YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

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

extern YYLTYPE yylloc;
