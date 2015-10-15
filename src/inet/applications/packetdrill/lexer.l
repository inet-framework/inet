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
ack                 return ACK;
eol                 return EOL;
ecr                 return ECR;
mss                 return MSS;
nop                 return NOP;
sack                return TCPSACK;
sackOK              return SACKOK;
TS                  return TIMESTAMP;
val                 return VAL;
win                 return WIN;
wscale              return WSCALE;
[.][.][.]           return ELLIPSIS;
--[a-zA-Z0-9_]+     yylval.string = option(yytext); return OPTION;
[-]?[0-9]*[.][0-9]+ yylval.floating = atof(yytext);   return MYFLOAT;
[-]?[0-9]+          yylval.integer = atoll(yytext);  return INTEGER;
0x[0-9a-fA-F]+      yylval.integer = hextol(yytext); return HEX_INTEGER;
[a-zA-Z0-9_]+       yylval.string = strdup(yytext); return MYWORD;
\"(\\.|[^"])*\"     yylval.string = quoted(yytext); return MYSTRING;
[^ \t\n]            return (int) yytext[0];
[ \t\n]+            /* ignore whitespace */;
{cpp_comment}       /* ignore C++-style comment */;
{c_comment}         /* ignore C-style comment */;
%%
