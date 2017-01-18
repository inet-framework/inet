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
/* Line 1529 of yacc.c.  */
#line 221 "parser.h"
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
