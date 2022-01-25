/* A Bison parser, made by GNU Bison 3.7.6.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSER_H_INCLUDED
# define YY_YY_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    ELLIPSIS = 258,                /* ELLIPSIS  */
    UDP = 259,                     /* UDP  */
    _HTONS_ = 260,                 /* _HTONS_  */
    _HTONL_ = 261,                 /* _HTONL_  */
    BACK_QUOTED = 262,             /* BACK_QUOTED  */
    SA_FAMILY = 263,               /* SA_FAMILY  */
    SIN_PORT = 264,                /* SIN_PORT  */
    SIN_ADDR = 265,                /* SIN_ADDR  */
    ACK = 266,                     /* ACK  */
    WIN = 267,                     /* WIN  */
    WSCALE = 268,                  /* WSCALE  */
    MSS = 269,                     /* MSS  */
    NOP = 270,                     /* NOP  */
    TIMESTAMP = 271,               /* TIMESTAMP  */
    ECR = 272,                     /* ECR  */
    EOL = 273,                     /* EOL  */
    TCPSACK = 274,                 /* TCPSACK  */
    VAL = 275,                     /* VAL  */
    SACKOK = 276,                  /* SACKOK  */
    OPTION = 277,                  /* OPTION  */
    IPV4_TYPE = 278,               /* IPV4_TYPE  */
    IPV6_TYPE = 279,               /* IPV6_TYPE  */
    INET_ADDR = 280,               /* INET_ADDR  */
    SPP_ASSOC_ID = 281,            /* SPP_ASSOC_ID  */
    SPP_ADDRESS = 282,             /* SPP_ADDRESS  */
    SPP_HBINTERVAL = 283,          /* SPP_HBINTERVAL  */
    SPP_PATHMAXRXT = 284,          /* SPP_PATHMAXRXT  */
    SPP_PATHMTU = 285,             /* SPP_PATHMTU  */
    SPP_FLAGS = 286,               /* SPP_FLAGS  */
    SPP_IPV6_FLOWLABEL_ = 287,     /* SPP_IPV6_FLOWLABEL_  */
    SPP_DSCP_ = 288,               /* SPP_DSCP_  */
    SINFO_STREAM = 289,            /* SINFO_STREAM  */
    SINFO_SSN = 290,               /* SINFO_SSN  */
    SINFO_FLAGS = 291,             /* SINFO_FLAGS  */
    SINFO_PPID = 292,              /* SINFO_PPID  */
    SINFO_CONTEXT = 293,           /* SINFO_CONTEXT  */
    SINFO_ASSOC_ID = 294,          /* SINFO_ASSOC_ID  */
    SINFO_TIMETOLIVE = 295,        /* SINFO_TIMETOLIVE  */
    SINFO_TSN = 296,               /* SINFO_TSN  */
    SINFO_CUMTSN = 297,            /* SINFO_CUMTSN  */
    SINFO_PR_VALUE = 298,          /* SINFO_PR_VALUE  */
    CHUNK = 299,                   /* CHUNK  */
    MYDATA = 300,                  /* MYDATA  */
    MYINIT = 301,                  /* MYINIT  */
    MYINIT_ACK = 302,              /* MYINIT_ACK  */
    MYHEARTBEAT = 303,             /* MYHEARTBEAT  */
    MYHEARTBEAT_ACK = 304,         /* MYHEARTBEAT_ACK  */
    MYABORT = 305,                 /* MYABORT  */
    MYSHUTDOWN = 306,              /* MYSHUTDOWN  */
    MYSHUTDOWN_ACK = 307,          /* MYSHUTDOWN_ACK  */
    MYERROR = 308,                 /* MYERROR  */
    MYCOOKIE_ECHO = 309,           /* MYCOOKIE_ECHO  */
    MYCOOKIE_ACK = 310,            /* MYCOOKIE_ACK  */
    MYSHUTDOWN_COMPLETE = 311,     /* MYSHUTDOWN_COMPLETE  */
    PAD = 312,                     /* PAD  */
    ERROR = 313,                   /* ERROR  */
    HEARTBEAT_INFORMATION = 314,   /* HEARTBEAT_INFORMATION  */
    CAUSE_INFO = 315,              /* CAUSE_INFO  */
    MYSACK = 316,                  /* MYSACK  */
    STATE_COOKIE = 317,            /* STATE_COOKIE  */
    PARAMETER = 318,               /* PARAMETER  */
    MYSCTP = 319,                  /* MYSCTP  */
    TYPE = 320,                    /* TYPE  */
    FLAGS = 321,                   /* FLAGS  */
    LEN = 322,                     /* LEN  */
    MYSUPPORTED_EXTENSIONS = 323,  /* MYSUPPORTED_EXTENSIONS  */
    MYSUPPORTED_ADDRESS_TYPES = 324, /* MYSUPPORTED_ADDRESS_TYPES  */
    TYPES = 325,                   /* TYPES  */
    CWR = 326,                     /* CWR  */
    ECNE = 327,                    /* ECNE  */
    TAG = 328,                     /* TAG  */
    A_RWND = 329,                  /* A_RWND  */
    OS = 330,                      /* OS  */
    IS = 331,                      /* IS  */
    TSN = 332,                     /* TSN  */
    MYSID = 333,                   /* MYSID  */
    SSN = 334,                     /* SSN  */
    PPID = 335,                    /* PPID  */
    CUM_TSN = 336,                 /* CUM_TSN  */
    GAPS = 337,                    /* GAPS  */
    DUPS = 338,                    /* DUPS  */
    MID = 339,                     /* MID  */
    FSN = 340,                     /* FSN  */
    SRTO_ASSOC_ID = 341,           /* SRTO_ASSOC_ID  */
    SRTO_INITIAL = 342,            /* SRTO_INITIAL  */
    SRTO_MAX = 343,                /* SRTO_MAX  */
    SRTO_MIN = 344,                /* SRTO_MIN  */
    SINIT_NUM_OSTREAMS = 345,      /* SINIT_NUM_OSTREAMS  */
    SINIT_MAX_INSTREAMS = 346,     /* SINIT_MAX_INSTREAMS  */
    SINIT_MAX_ATTEMPTS = 347,      /* SINIT_MAX_ATTEMPTS  */
    SINIT_MAX_INIT_TIMEO = 348,    /* SINIT_MAX_INIT_TIMEO  */
    MYSACK_DELAY = 349,            /* MYSACK_DELAY  */
    SACK_FREQ = 350,               /* SACK_FREQ  */
    ASSOC_VALUE = 351,             /* ASSOC_VALUE  */
    ASSOC_ID = 352,                /* ASSOC_ID  */
    SACK_ASSOC_ID = 353,           /* SACK_ASSOC_ID  */
    RECONFIG = 354,                /* RECONFIG  */
    OUTGOING_SSN_RESET = 355,      /* OUTGOING_SSN_RESET  */
    REQ_SN = 356,                  /* REQ_SN  */
    RESP_SN = 357,                 /* RESP_SN  */
    LAST_TSN = 358,                /* LAST_TSN  */
    SIDS = 359,                    /* SIDS  */
    INCOMING_SSN_RESET = 360,      /* INCOMING_SSN_RESET  */
    RECONFIG_RESPONSE = 361,       /* RECONFIG_RESPONSE  */
    RESULT = 362,                  /* RESULT  */
    SENDER_NEXT_TSN = 363,         /* SENDER_NEXT_TSN  */
    RECEIVER_NEXT_TSN = 364,       /* RECEIVER_NEXT_TSN  */
    SSN_TSN_RESET = 365,           /* SSN_TSN_RESET  */
    ADD_INCOMING_STREAMS = 366,    /* ADD_INCOMING_STREAMS  */
    NUMBER_OF_NEW_STREAMS = 367,   /* NUMBER_OF_NEW_STREAMS  */
    ADD_OUTGOING_STREAMS = 368,    /* ADD_OUTGOING_STREAMS  */
    RECONFIG_REQUEST_GENERIC = 369, /* RECONFIG_REQUEST_GENERIC  */
    SRS_ASSOC_ID = 370,            /* SRS_ASSOC_ID  */
    SRS_FLAGS = 371,               /* SRS_FLAGS  */
    SRS_NUMBER_STREAMS = 372,      /* SRS_NUMBER_STREAMS  */
    SRS_STREAM_LIST = 373,         /* SRS_STREAM_LIST  */
    SSTAT_ASSOC_ID = 374,          /* SSTAT_ASSOC_ID  */
    SSTAT_STATE = 375,             /* SSTAT_STATE  */
    SSTAT_RWND = 376,              /* SSTAT_RWND  */
    SSTAT_UNACKDATA = 377,         /* SSTAT_UNACKDATA  */
    SSTAT_PENDDATA = 378,          /* SSTAT_PENDDATA  */
    SSTAT_INSTRMS = 379,           /* SSTAT_INSTRMS  */
    SSTAT_OUTSTRMS = 380,          /* SSTAT_OUTSTRMS  */
    SSTAT_FRAGMENTATION_POINT = 381, /* SSTAT_FRAGMENTATION_POINT  */
    SSTAT_PRIMARY = 382,           /* SSTAT_PRIMARY  */
    SASOC_ASOCMAXRXT = 383,        /* SASOC_ASOCMAXRXT  */
    SASOC_ASSOC_ID = 384,          /* SASOC_ASSOC_ID  */
    SASOC_NUMBER_PEER_DESTINATIONS = 385, /* SASOC_NUMBER_PEER_DESTINATIONS  */
    SASOC_PEER_RWND = 386,         /* SASOC_PEER_RWND  */
    SASOC_LOCAL_RWND = 387,        /* SASOC_LOCAL_RWND  */
    SASOC_COOKIE_LIFE = 388,       /* SASOC_COOKIE_LIFE  */
    SAS_ASSOC_ID = 389,            /* SAS_ASSOC_ID  */
    SAS_INSTRMS = 390,             /* SAS_INSTRMS  */
    SAS_OUTSTRMS = 391,            /* SAS_OUTSTRMS  */
    MYINVALID_STREAM_IDENTIFIER = 392, /* MYINVALID_STREAM_IDENTIFIER  */
    ISID = 393,                    /* ISID  */
    MYFLOAT = 394,                 /* MYFLOAT  */
    INTEGER = 395,                 /* INTEGER  */
    HEX_INTEGER = 396,             /* HEX_INTEGER  */
    MYWORD = 397,                  /* MYWORD  */
    MYSTRING = 398                 /* MYSTRING  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 224 "parser.y"

    int64_t integer;
    double floating;
    char *string;
    char *reserved;
    int64_t time_usecs;
    enum direction_t direction;
    uint16_t port;
    int32_t window;
    uint32_t sequence_number;
    struct {
        int protocol;    /* IPPROTO_TCP or IPPROTO_UDP */
        uint32_t start_sequence;
        uint16_t payload_bytes;
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
    uint8_t byte;
    PacketDrillSctpChunk *sctp_chunk;

#line 246 "parser.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE yylval;
extern YYLTYPE yylloc;
int yyparse (void);

#endif /* !YY_YY_PARSER_H_INCLUDED  */
