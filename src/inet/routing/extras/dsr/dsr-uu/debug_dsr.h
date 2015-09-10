/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef _DSR_DEBUG_H
#define _DSR_DEBUG_H

#ifndef OMNETPP

#ifdef __KERNEL__
#include <stdarg.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/if_ether.h>
#include <linux/in.h>
#include <linux/init.h>
extern atomic_t num_pkts;
#else
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#endif              /* __KERNEL__ */

#ifdef DEBUG
#undef DEBUG
#define DEBUG_PROC
#define DEBUG(f, args...) do { if (get_confval(PrintDebug)) trace(__FUNCTION__, f, ## args); } while (0)
//#define DEBUG(f, args...) trace(__FUNCTION__, f, ## args)
#else
#define DEBUG(f, args...)
#endif

#ifndef NO_GLOBALS

#define DEBUG_BUFLEN 256

static inline char *print_ip(struct in_addr addr)
{
    static char buf[16 * 4];
    static int index = 0;
    char *str;

    sprintf(&buf[index], "%d.%d.%d.%d",
            0x0ff & addr.s_addr,
            0x0ff & (addr.s_addr >> 8),
            0x0ff & (addr.s_addr >> 16), 0x0ff & (addr.s_addr >> 24));

    str = &buf[index];
    index += 16;
    index %= 64;

    return str;
}

static inline char *print_eth(char *addr)
{
    static char buf[30];

    sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x",
            (unsigned char)addr[0], (unsigned char)addr[1],
            (unsigned char)addr[2], (unsigned char)addr[3],
            (unsigned char)addr[4], (unsigned char)addr[5]);

    return buf;
}

static inline char *print_pkt(char *p, int len)
{
    static char buf[3000];
    int i, l = 0;

    for (i = 0; i < len; i++)
        l += sprintf(buf + l, "%02X", (unsigned char)p[i]);

    return buf;
}

#endif              /* NO_GLOBALS */

#ifndef NO_DECLS

int trace(const char *func, const char *fmt, ...);

#endif              /* NO_DECLS */

#ifdef __KERNEL__
int __init dbg_init(void);
void __exit dbg_cleanup(void);
#endif

#endif


#endif              /* _DEBUG_H */
