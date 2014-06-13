/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef _DSR_RTC_H
#define _DSR_RTC_H

#ifdef __KERNEL__
#include <linux/in.h>
#include <linux/types.h>

#define DSR_RTC_PROC_NAME "dsr_rtc"
#endif

#include "dsr-srt.h"

/* DSR route cache API */

struct dsr_srt *dsr_rtc_find(struct in_addr src, struct in_addr dst);
int dsr_rtc_add(struct dsr_srt *srt, unsigned long time, unsigned short flags);
int dsr_rtc_del(struct in_addr src, struct in_addr dst);
void dsr_rtc_flush(void);

#endif              /* _DSR_RTC_H */
