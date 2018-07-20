%{
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
 * Author: ncardwell@google.com (Neal Cardwell)
 *
 * This is the specification for the lexical scanner for the packetdrill
 * script language. It is processed by the flex lexical scanner
 * generator.
 *
 * For full documentation see: http://flex.sourceforge.net/manual/
 *
 * Here is a quick and dirty tutorial on flex:
 *
 * A flex lexical scanner specification is basically a list of rules,
 * where each rule is a regular expressions for a lexical token to
 * match, followed by a C fragment to execute when the scanner sees
 * that pattern.
 *
 * The lexer feeds a stream of terminal symbols up to this parser,
 * passing up a FOO token for each "return FOO" in the lexer spec. The
 * lexer specifies what value to pass up to the parser by setting a
 * yylval.fooval field, where fooval is a field in the %union in the
 * .y file.
 *
 * TODO: detect overflow in numeric literals.
 */


#include "inet/common/INETDefs.h"

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include "PacketDrillUtils.h"

using namespace inet;

/* This include of the bison-generated .h file must go last so that we
 * can first include all of the declarations on which it depends.
 */
#include "parser.h"

/* Suppress flex's generation of an uncalled static input() function, which
 * leads to a compiler warning:
 * warning: ‘input’ defined but not used
 */
#define YY_NO_INPUT

/* Copy the string name "foo" after the "--" of a "--foo" option. */
static char *option(const char *s) {
    const int dash_dash_len = 2;
    char *str = (char *)malloc (strlen(s) - dash_dash_len);
    strncpy(str, s + dash_dash_len, strlen(s) - dash_dash_len);
    return str;
}

/* Copy the string inside a quoted string. */
static char *quoted(const char *s) {
    const int delim_len = 1;
    char *str = (char *)malloc (strlen(s) - 2*delim_len);
    strncpy(str, s + delim_len, strlen(s) - 2*delim_len);
    return str;
}


/* Convert a hex string prefixed by "0x" to an integer value. */
static int64 hextol(const char *s) {
    return strtol(yytext + 2, NULL, 16);
}

enum ifdef_os {
    FreeBSD_IFDEF = 1, Omnet_IFDEF
};

#define MAX_IFDEF_DEPTH 1
YY_BUFFER_STATE ifdef_stack[MAX_IFDEF_DEPTH];
int ifdef_stack_ptr = 0;

static inline int get_os_name_length(enum ifdef_os os) {
    switch (os) {
    case FreeBSD_IFDEF:
        return strlen("FreeBSD");
    case Omnet_IFDEF:
        return strlen("Omnet");
    default:
        return -1;
    }
}

static inline bool ignore_ifdef(enum ifdef_os os) {
#ifdef __FreeBSD__
    if (os == FreeBSD_IFDEF) {
        return false;
    }
#endif
#ifdef OMNETPP_VERSION
    if (os == Omnet_IFDEF) {
        return false;
    }
#endif
    return true;
}

static inline char* remove_ifdef_start_and_endtag(char *code, int os_name_length) {
    unsigned int ifdef_length = strlen("#ifdef ");
    unsigned int endif_length = strlen("#endif");
    unsigned int newline_before_endif = 0;
    char *code_without_ifdef = NULL;

    code_without_ifdef = code + ifdef_length + os_name_length;
    printf("ifdef_length=%d os_name_length=%d code_without_ifdef=%zu\n", ifdef_length, os_name_length, strlen(code_without_ifdef));
    newline_before_endif = strlen(code_without_ifdef) - endif_length;
    code_without_ifdef[newline_before_endif] = (char) 0;
    return code_without_ifdef;
}


static void handle_ifdef(enum ifdef_os os, const char *s) {
    char *code = NULL;
    char *code_without_ifdef = NULL;
    int os_name_length = get_os_name_length(os);
printf("handle ifdef code=%s\n", code);
    if (os_name_length == -1) {
        fprintf(stderr, "handle_ifdef with unknown os called.\n");
        exit(1);
    }

    if (ignore_ifdef(os)) {
        return;
    }

    if (ifdef_stack_ptr >= MAX_IFDEF_DEPTH) {
        fprintf(stderr, "Ifdefs nested too deeply");
        exit(1);
    }

    code = strdup(s);

    code_without_ifdef = remove_ifdef_start_and_endtag(code, os_name_length);
    ifdef_stack[ifdef_stack_ptr++] = YY_CURRENT_BUFFER;
    yy_switch_to_buffer(yy_scan_string(code_without_ifdef));
printf("code_without_ifdef=%s\n", code_without_ifdef);
    free(code);
}

%}

%{
#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;
%}
%option yylineno
%option nounput
%option noyywrap


/* A regexp for C++ comments: */
cpp_comment	\/\/[^\n]*\n

/* Here is a summary of the regexp for C comments:
 *   open-comment
 *   any number of:
 *     (non-stars) or (star then non-slash)
 *   close comment
 */
c_comment    \/\*(([^*])|(\*[^\/]))*\*\/

/* This matches the following platform specific #ifdef-forms:
 *   #ifdef FreeBSD => Code that only FreeBSD hosts should execute
 *   #ifdef Omnet   => Code that only Omnet hosts should execute
 *
 *   the pattern for using #ifdef is like this:
 *   #ifdef FreeBSD
 *   (specific code only for freebsd)
 *   #endif
 */

/* these are the tags that identify the start and ending of an ifdef block */
ifdef_begin #ifdef[ ]
ifdef_end   #endif
/* end_matcher actually matches everything except the "#endif" tag. */
end_matcher (([^#])|(#[^e])|(#e[^n])|(#en[^d])|(#end[^i])|(#endi[^f]))*

ifdef_freebsd   {ifdef_begin}(?i:FreeBSD){end_matcher}{ifdef_end}
ifdef_omnet     {ifdef_begin}(?i:Omnet){end_matcher}{ifdef_end}

/* The regexp for code snippets is analogous to that for C comments.
 * Here is a summary of the regexp for code snippets:
 *   %{
 *   any number of:
 *     (non-}) or (} then non-%)
 *   }%
 */
code         \%\{(([^}])|(\}[^\%]))*\}\%

/* IPv4: a regular experssion for an IPv4 address */
ipv4_addr    [0-9]+[.][0-9]+[.][0-9]+[.][0-9]+

/* IPv6: a regular experssion for an IPv6 address. The complexity is
 * unfortunate, but we can't use a super-simple approach because TCP
 * sequence number ranges like 1:1001 can look like IPv6 addresses if
 * we use a naive approach.
 */
seg     [0-9a-fA-F]{1,4}
v0      [:][:]
v1      ({seg}[:]){7,7}{seg}
v2      ({seg}[:]){1,7}[:]
v3      ({seg}[:]){1,6}[:]{seg}
v4      ({seg}[:]){1,5}([:]{seg}){1,2}
v5      ({seg}[:]){1,4}([:]{seg}){1,3}
v6      ({seg}[:]){1,3}([:]{seg}){1,4}
v7      ({seg}[:]){1,2}([:]{seg}){1,5}
v8      {seg}[:](([:]{seg}){1,6})
v9      [:]([:]{seg}){1,7}
/* IPv4-mapped IPv6 address: */
v10     [:][:]ffff[:]{ipv4_addr}
/* IPv4-translated IPv6 address: */
v11     [:][:]ffff[:](0){1,4}[:]{ipv4_addr}
/* IPv4-embedded IPv6 addresses: */
v12     ({seg}[:]){1,4}[:]{ipv4_addr}
ipv6_addr ({v0}|{v1}|{v2}|{v3}|{v4}|{v5}|{v6}|{v7}|{v8}|{v9}|{v10}|{v11}|{v12})

%%
udp                 return UDP;
sa_family           return SA_FAMILY;
sin_port            return SIN_PORT;
sin_addr            return SIN_ADDR;
ack                 return ACK;
eol                 return EOL;
ecr                 return ECR;
mss                 return MSS;
nop                 return NOP;
sack                return TCPSACK;
sackOK              return SACKOK;
TS                  return TIMESTAMP;
val                 return VAL;
htons               return _HTONS_;
htonl               return _HTONL_;
win                 return WIN;
wscale              return WSCALE;
sctp                return MYSCTP;
CHUNK               return CHUNK;
DATA                return MYDATA;
INIT                return MYINIT;
INIT_ACK            return MYINIT_ACK;
SACK                return MYSACK;
HEARTBEAT           return MYHEARTBEAT;
HEARTBEAT_ACK       return MYHEARTBEAT_ACK;
ABORT               return MYABORT;
SHUTDOWN            return MYSHUTDOWN;
SHUTDOWN_ACK        return MYSHUTDOWN_ACK;
ERROR               return MYERROR;
COOKIE_ECHO         return MYCOOKIE_ECHO;
COOKIE_ACK          return MYCOOKIE_ACK;
SHUTDOWN_COMPLETE   return MYSHUTDOWN_COMPLETE;
STATE_COOKIE        return STATE_COOKIE;
HEARTBEAT_INFORMATION return HEARTBEAT_INFORMATION;
SUPPORTED_EXTENSIONS return MYSUPPORTED_EXTENSIONS;
SUPPORTED_ADDRESS_TYPES    return MYSUPPORTED_ADDRESS_TYPES;
RECONFIG            return RECONFIG;
OUTGOING_SSN_RESET  return OUTGOING_SSN_RESET;
INCOMING_SSN_RESET  return INCOMING_SSN_RESET;
SSN_TSN_RESET       return SSN_TSN_RESET;
ADD_OUTGOING_STREAMS return ADD_OUTGOING_STREAMS;
ADD_INCOMING_STREAMS return ADD_INCOMING_STREAMS;
INVALID_STREAM_IDENTIFIER return MYINVALID_STREAM_IDENTIFIER;
RECONFIG_RESPONSE   return RECONFIG_RESPONSE;
IPv4                return IPV4_TYPE;
IPv6                return IPV6_TYPE;
types               return TYPES;
type                return TYPE;
flgs                return FLAGS;
len                 return LEN;
tag                 return TAG;
a_rwnd              return A_RWND;
is                  return IS;
os                  return OS;
tsn                 return TSN;
sid                 return MYSID;
ssn                 return SSN;
ppid                return PPID;
cum_tsn             return CUM_TSN;
gaps                return GAPS;
dups                return DUPS;
sack_assoc_id       return SACK_ASSOC_ID;
sack_delay          return MYSACK_DELAY;
sack_freq           return SACK_FREQ;
srto_initial        return SRTO_INITIAL;
srto_max            return SRTO_MAX;
srto_min            return SRTO_MIN;
sinit_num_ostreams  return SINIT_NUM_OSTREAMS;
sinit_max_instreams return SINIT_MAX_INSTREAMS;
sinit_max_attempts  return SINIT_MAX_ATTEMPTS;
sinit_max_init_timeo return SINIT_MAX_INIT_TIMEO;
assoc_value         return ASSOC_VALUE;
assoc_id            return ASSOC_ID;
spp_assoc_id        return SPP_ASSOC_ID;
spp_address         return SPP_ADDRESS;
spp_hbinterval      return SPP_HBINTERVAL;
spp_pathmaxrxt      return SPP_PATHMAXRXT;
spp_pathmtu         return SPP_PATHMTU;
spp_flags           return SPP_FLAGS;
spp_ipv6_flowlabel  return SPP_IPV6_FLOWLABEL_; /* avoid name clash */
spp_dscp            return SPP_DSCP_; /* avoid name clash */
sinfo_stream        return SINFO_STREAM;
sinfo_ssn           return SINFO_SSN;
sinfo_flags         return SINFO_FLAGS;
sinfo_ppid          return SINFO_PPID;
sinfo_context       return SINFO_CONTEXT;
sinfo_timetolive    return SINFO_TIMETOLIVE;
sinfo_tsn           return SINFO_TSN;
sinfo_cumtsn        return SINFO_CUMTSN;
sinfo_assoc_id      return SINFO_ASSOC_ID;
sinfo_pr_value      return SINFO_PR_VALUE;
sstat_assoc_id      return SSTAT_ASSOC_ID;
sstat_state         return SSTAT_STATE;
sstat_rwnd          return SSTAT_RWND;
sstat_unackdata     return SSTAT_UNACKDATA;
sstat_penddata      return SSTAT_PENDDATA;
sstat_instrms       return SSTAT_INSTRMS;
sstat_outstrms      return SSTAT_OUTSTRMS;
sstat_fragmentation_point return SSTAT_FRAGMENTATION_POINT;
sstat_primary       return SSTAT_PRIMARY;
srs_assoc_id        return SRS_ASSOC_ID;
srs_flags           return SRS_FLAGS;
srs_number_streams  return SRS_NUMBER_STREAMS;
srs_stream_list     return SRS_STREAM_LIST;
sasoc_assoc_id      return SASOC_ASSOC_ID;
sasoc_asocmaxrxt    return SASOC_ASOCMAXRXT;
sasoc_number_peer_destinations return SASOC_NUMBER_PEER_DESTINATIONS;
sasoc_peer_rwnd     return SASOC_PEER_RWND;
sasoc_local_rwnd    return SASOC_LOCAL_RWND;
sasoc_cookie_life   return SASOC_COOKIE_LIFE;
sas_assoc_id        return SAS_ASSOC_ID;
sas_instrms         return SAS_INSTRMS;
sas_outstrms        return SAS_OUTSTRMS;
inet_addr           return INET_ADDR;
req_sn              return REQ_SN;
resp_sn             return RESP_SN;
last_tsn            return LAST_TSN;
sids                return SIDS;
sender_next_tsn     return SENDER_NEXT_TSN;
receiver_next_tsn   return RECEIVER_NEXT_TSN;
number_of_new_streams return NUMBER_OF_NEW_STREAMS;
result              return RESULT;
[.][.][.]           return ELLIPSIS;
--[a-zA-Z0-9_]+     yylval.string = option(yytext); return OPTION;
[-]?[0-9]*[.][0-9]+ yylval.floating = atof(yytext);   return MYFLOAT;
[-]?[0-9]+          yylval.integer = atoll(yytext);  return INTEGER;
0x[0-9a-fA-F]+      yylval.integer = hextol(yytext); return HEX_INTEGER;
[a-zA-Z0-9_]+       yylval.string = strdup(yytext); return MYWORD;
\"(\\.|[^"])*\"     yylval.string = quoted(yytext); return MYSTRING;
\`(\\.|[^`])*\`     yylval.string = quoted(yytext); return BACK_QUOTED;
[^ \t\n]            return (int) yytext[0];
[ \t\n]+            /* ignore whitespace */;
{cpp_comment}       /* ignore C++-style comment */;
{c_comment}         /* ignore C-style comment */;
{ifdef_freebsd}     handle_ifdef(FreeBSD_IFDEF, yytext);
{ifdef_omnet}       handle_ifdef(Omnet_IFDEF, yytext);
<<EOF>>             {
                        if ( --ifdef_stack_ptr < 0 ) {
                            yyterminate();
                        } else {
                            yy_delete_buffer(YY_CURRENT_BUFFER);
                            yy_switch_to_buffer(ifdef_stack[ifdef_stack_ptr]);
                        }
                    }
%%

