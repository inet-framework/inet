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
/* Line 1529 of yacc.c.  */
#line 225 "parser.h"
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
