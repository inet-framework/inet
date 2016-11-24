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
/* Line 1529 of yacc.c.  */
#line 374 "parser.h"
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
