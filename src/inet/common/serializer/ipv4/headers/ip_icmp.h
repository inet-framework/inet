/*
 * Copyright (c) 1982, 1986, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      @(#)ip_icmp.h   8.1 (Berkeley) 6/10/93
 * $FreeBSD: src/sys/netinet/ip_icmp.h,v 1.16 1999/12/29 04:41:01 peter Exp $
 */

#ifndef __INET_IP_ICMP_H
#define __INET_IP_ICMP_H

namespace inet {

namespace serializer {

/*
 * Interface Control Message Protocol Definitions.
 * Per RFC 792, September 1981.
 */

/*
 * Internal of an ICMP Router Advertisement
 */
struct icmp_ra_addr
{
    u_int32_t ira_addr;
    u_int32_t ira_preference;
};

/*
 * Structure of an icmp header.
 */
struct icmp
{
    u_char icmp_type;    /* type of message, see below */
    u_char icmp_code;    /* type sub code */
    u_short icmp_cksum;    /* ones complement cksum of struct */
    union
    {
        u_char ih_pptr;    /* ICMP_PARAMPROB */
        struct in_addr ih_gwaddr;    /* ICMP_REDIRECT */
        struct ih_idseq
        {
            n_short icd_id;
            n_short icd_seq;
        } ih_idseq;
        int ih_void;

        /* ICMP_UNREACH_NEEDFRAG -- Path MTU Discovery (RFC1191) */
        struct ih_pmtu
        {
            n_short ipm_void;
            n_short ipm_nextmtu;
        } ih_pmtu;

        struct ih_rtradv
        {
            u_char irt_num_addrs;
            u_char irt_wpa;
            u_int16_t irt_lifetime;
        } ih_rtradv;
    } icmp_hun;
#define icmp_pptr         icmp_hun.ih_pptr
#define icmp_gwaddr       icmp_hun.ih_gwaddr
#define icmp_id           icmp_hun.ih_idseq.icd_id
#define icmp_seq          icmp_hun.ih_idseq.icd_seq
#define icmp_void         icmp_hun.ih_void
#define icmp_pmvoid       icmp_hun.ih_pmtu.ipm_void
#define icmp_nextmtu      icmp_hun.ih_pmtu.ipm_nextmtu
#define icmp_num_addrs    icmp_hun.ih_rtradv.irt_num_addrs
#define icmp_wpa          icmp_hun.ih_rtradv.irt_wpa
#define icmp_lifetime     icmp_hun.ih_rtradv.irt_lifetime
    union
    {
        struct id_ts
        {
            n_time its_otime;
            n_time its_rtime;
            n_time its_ttime;
        } id_ts;
        struct id_ip
        {
            struct ip idi_ip;
            /* options and then 64 bits of data */
        } id_ip;
        struct icmp_ra_addr id_radv;
        u_int32_t id_mask;
        char id_data[1];
    } icmp_dun;
#define icmp_otime        icmp_dun.id_ts.its_otime
#define icmp_rtime        icmp_dun.id_ts.its_rtime
#define icmp_ttime        icmp_dun.id_ts.its_ttime
#define icmp_ip           icmp_dun.id_ip.idi_ip
#define icmp_radv         icmp_dun.id_radv
#define icmp_mask         icmp_dun.id_mask
#define icmp_data         icmp_dun.id_data
};

/*
 * Lower bounds on packet lengths for various types.
 * For the error advice packets must first insure that the
 * packet is large enough to contain the returned ip header.
 * Only then can we do the check to see if 64 bits of packet
 * data have been returned, since we need to check the returned
 * ip header length.
 */
#define ICMP_MINLEN       8                               /* abs minimum */
#define ICMP_TSLEN        (8 + 3 * sizeof(n_time))       /* timestamp */
#define ICMP_MASKLEN      12                              /* address mask */
#define ICMP_ADVLENMIN    (8 + sizeof(struct ip) + 8)    /* min */
#ifndef _IP_VHL
#define ICMP_ADVLEN(p)    (8 + ((p)->icmp_ip.ip_hl << 2) + 8)
/* N.B.: must separately check that ip_hl >= 5 */
#else // ifndef _IP_VHL
#define ICMP_ADVLEN(p)    (8 + (IP_VHL_HL((p)->icmp_ip.ip_vhl) << 2) + 8)
/* N.B.: must separately check that header length >= 5 */
#endif // ifndef _IP_VHL

} // namespace serializer

} // namespace inet

#endif // ifndef __INET_IP_ICMP_H

