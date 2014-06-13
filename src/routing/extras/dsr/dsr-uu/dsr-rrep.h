/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef _DSR_RREP_H
#define _DSR_RREP_H

#include "dsr.h"
#include "dsr-srt.h"

#ifndef NO_GLOBALS

struct dsr_rrep_opt
{
    u_int8_t type;
    u_int8_t length;
#if defined(__LITTLE_ENDIAN_BITFIELD)
    u_int8_t res:7;
    u_int8_t l:1;
#elif defined (__BIG_ENDIAN_BITFIELD)
    u_int8_t l:1;
    u_int8_t res:7;
#else
#error  "Please fix <asm/byteorder.h>"
#endif
    u_int32_t addrs[0];
};

#define DSR_RREP_HDR_LEN sizeof(struct dsr_rrep_opt)
#define DSR_RREP_OPT_LEN(srt) (DSR_RREP_HDR_LEN + srt->laddrs + sizeof(struct in_addr))
/* Length of source route is length of option, minus reserved/flags field minus
 * the last source route hop (which is the destination) */
#define DSR_RREP_ADDRS_LEN(rrep_opt) (rrep_opt->length - 1 - sizeof(struct in_addr))

#endif              /* NO_GLOBALS */

#ifndef NO_DECLS

int dsr_rrep_opt_recv(struct dsr_pkt *dp, struct dsr_rrep_opt *rrep_opt);
int dsr_rrep_send(struct dsr_srt *srt, struct dsr_srt *srt_to_me);

void grat_rrep_tbl_timeout(unsigned long data);
int grat_rrep_tbl_add(struct in_addr src, struct in_addr prev_hop);
int grat_rrep_tbl_find(struct in_addr src, struct in_addr prev_hop);
int grat_rrep_tbl_init(void);
void grat_rrep_tbl_cleanup(void);

#endif              /* NO_DECLS */

#endif              /* _DSR_RREP */
