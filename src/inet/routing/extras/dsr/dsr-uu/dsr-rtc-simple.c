/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>

#undef DEBUG
#include "tbl.h"
#include "dsr-rtc.h"
#include "dsr-srt.h"
#include "debug.h"

#define RTC_MAX_LEN 1024

static unsigned int rtc_len;
static rwlock_t rtc_lock = RW_LOCK_UNLOCKED;
static LIST_HEAD(rtc_head);
static TBL(rtc_tbl, RTC_MAX_LEN);

#define list_is_first(e) (&e->l == rtc_head.next)

MODULE_AUTHOR("Erik Nordstroem <erikn@it.uu.se>");
MODULE_DESCRIPTION("Dynamic Source Routing (DSR) simple route cache");
MODULE_LICENSE("GPL");

/* Timers and timeouts could potentially be handled in the kernel. However,
 * currently they are not, because it complicates things quite a bit. The code
 * for adding timers is still here though... - Erik */

struct rtc_entry {
	dsr_list_t l;
	unsigned long expires;
	unsigned short flags;
	struct dsr_srt srt;
};

#define RTC_TIMER

#ifdef RTC_TIMER
static DSRUUTimer rtc_timer;

static void dsr_rtc_timeout(unsigned long data);

static inline void __dsr_rtc_set_next_timeout(void)
{
	struct rtc_entry *ne;

	if (list_empty(&rtc_head))
		return;

	/* Get first entry */
	ne = (struct rtc_entry *)rtc_head.next;

	if (timer_pending(&rtc_timer)) {
		mod_timer(&rtc_timer, ne->expires);
	} else {
		rtc_timer.function = dsr_rtc_timeout;
		rtc_timer.expires = ne->expires;
		rtc_timer.data = 0;
		add_timer(&rtc_timer);
	}
}

static void dsr_rtc_timeout(unsigned long data)
{
	dsr_list_t *pos, *tmp;
	int time = TimeNow;

	DSR_WRITE_LOCK(&rtc_lock);

	DEBUG("srt timeout\n");

	list_for_each_safe(pos, tmp, &rtc_head) {
		struct rtc_entry *e = (struct rtc_entry *)pos;

		if (e->expires > time)
			break;

		list_del(&e->l);
		FREE(e);
		rtc_len--;
	}
	__dsr_rtc_set_next_timeout();
	DSR_WRITE_UNLOCK(&rtc_lock);
}
#endif				/* RTC_TIMER */

static inline void __dsr_rtc_flush(void)
{
	dsr_list_t *pos, *tmp;

	list_for_each_safe(pos, tmp, &rtc_head) {
		struct rtc_entry *e = (struct rtc_entry *)pos;
		list_del(&e->l);
		rtc_len--;
		FREE(e);
	}
}

static inline int __dsr_rtc_add(struct rtc_entry *e)
{

	if (rtc_len >= RTC_MAX_LEN) {
		printk(KERN_WARNING "dsr_rtc: Max list len reached\n");
		return -ENOSPC;
	}

	if (list_empty(&rtc_head)) {
		list_add(&e->l, &rtc_head);
	} else {
		dsr_list_t *pos;

		list_for_each(pos, &rtc_head) {
			struct rtc_entry *curr = (struct rtc_entry *)pos;

			if (curr->expires > e->expires)
				break;
		}
		list_add(&e->l, pos->prev);
	}
	return 1;
}

static inline struct rtc_entry *__dsr_rtc_find(__u32 daddr)
{
	dsr_list_t *pos;

	list_for_each(pos, &rtc_head) {
		struct rtc_entry *e = (struct rtc_entry *)pos;

		if (e->srt.dst.s_addr == daddr)
			return e;
	}
	return NULL;
}

static inline int __dsr_rtc_del(struct rtc_entry *e)
{
	if (e == NULL)
		return 0;

	if (list_is_first(e)) {

		list_del(&e->l);
#ifdef RTC_TIMER
		if (!list_empty(&rtc_head)) {
			/* Get the first entry */
			struct rtc_entry *f = (struct rtc_entry *)rtc_head.next;

			/* Update the timer */
			mod_timer(&rtc_timer, f->expires);
		}
#endif
	} else
		list_del(&e->l);

	return 1;
}

int dsr_rtc_del(struct in_addr src, struct in_addr dst)
{
	int res;
	struct rtc_entry *e;

	DSR_WRITE_LOCK(&rtc_lock);

	e = __dsr_rtc_find(dst.s_addr);

	if (e == NULL) {
		res = 0;
		goto unlock;
	}

	res = __dsr_rtc_del(e);

	if (res)
		FREE(e);
      unlock:
	DSR_WRITE_UNLOCK(&rtc_lock);

	return res;
}

struct dsr_srt *dsr_rtc_find(struct in_addr src, struct in_addr dst)
{
	struct rtc_entry *e;
	struct dsr_srt *srt;

/*     printk("Checking activeness\n"); */
	DSR_READ_LOCK(&rtc_lock);
	e = __dsr_rtc_find(dst.s_addr);

	if (e) {
		/* We must make a copy of the source route so that we do not
		 * return a pointer into the shared data structure */
		srt =
		    MALLOC(e->srt.laddrs + sizeof(struct dsr_srt), GFP_ATOMIC);
		memcpy(srt, &e->srt, e->srt.laddrs + sizeof(struct dsr_srt));
		DSR_READ_UNLOCK(&rtc_lock);
		return srt;
	}
	DSR_READ_UNLOCK(&rtc_lock);
	return NULL;
}

int dsr_rtc_add(struct dsr_srt *srt, unsigned long time, unsigned short flags)
{
	struct rtc_entry *e;
	int status = 0;

	if (!srt || dsr_rtc_find(srt->src, srt->dst))
		return 0;

	DEBUG("Adding source route to route cache\n");

	e = MALLOC(sizeof(struct rtc_entry) + srt->laddrs, GFP_ATOMIC);

	if (e == NULL) {
		printk(KERN_ERR "rtc: OOM in rtc_add\n");
		return -ENOMEM;
	}

	e->flags = flags;
	e->expires = TimeNow + (time * HZ) / 1000;
	memcpy(&e->srt, srt, sizeof(struct dsr_srt));
	memcpy(e->srt.addrs, srt->addrs, srt->laddrs);

	DSR_WRITE_LOCK(&rtc_lock);

	status = __dsr_rtc_add(e);

	if (status)
		rtc_len++;

#ifdef RTC_TIMER
	/* If the added element was added first in the list we update the timer */
	if (status && list_is_first(e)) {

		if (timer_pending(&rtc_timer))
			mod_timer(&rtc_timer, e->expires);
		else {
			rtc_timer.function = dsr_rtc_timeout;
			rtc_timer.expires = e->expires;
			rtc_timer.data = 0;
			add_timer(&rtc_timer);
		}
	}
#endif
	DSR_WRITE_UNLOCK(&rtc_lock);

	if (status < 0) {
		DEBUG("add failed\n");
		FREE(e);
	}
	return status;
}

void
dsr_rtc_update(struct dsr_srt *srt, unsigned long time, unsigned short flags)
{
	struct rtc_entry *e;

	if (!srt)
		return;

	DSR_WRITE_LOCK(&rtc_lock);

	e = __dsr_rtc_find(srt->dst.s_addr);

	if (e == NULL) {
		/* printk("rtc_update: No entry to update!\n"); */
		goto unlock;
	}
	e->flags = flags;
	/* Update expire time */
	e->expires = TimeNow + (time * HZ) / 1000;
	memcpy(&e->srt, srt, sizeof(struct dsr_srt));
	memcpy(e->srt.addrs, srt->addrs, srt->laddrs);

	/* Remove from list */
	list_del(&e->l);
	__dsr_rtc_add(e);
#ifdef RTC_TIMER
	__dsr_rtc_set_next_timeout();
#endif

      unlock:
	DSR_WRITE_UNLOCK(&rtc_lock);
}

static int dsr_rtc_print(char *buf)
{
	dsr_list_t *pos;
	int len = 0;

	DSR_READ_LOCK(&rtc_lock);

	len += sprintf(buf, "# %-5s %-8s Source Route\n", "Flags", "Expires");

	list_for_each(pos, &rtc_head) {
		char flags[4];
		int num_flags = 0;
		struct rtc_entry *e = (struct rtc_entry *)pos;

		flags[num_flags] = '\0';

		len += sprintf(buf + len, "  %-5s %-8lu %s\n", flags,
			       (e->expires - TimeNow) * 1000 / HZ,
			       print_srt(&e->srt));
	}

	DSR_READ_UNLOCK(&rtc_lock);
	return len;
}
static int
dsr_rtc_proc_info(char *buffer, char **start, off_t offset, int length)
{
	int len;

	len = dsr_rtc_print(buffer);

	*start = buffer + offset;
	len -= offset;
	if (len > length)
		len = length;
	else if (len < 0)
		len = 0;
	return len;
}

void dsr_rtc_flush(void)
{
#ifdef RTC_TIMER
	if (timer_pending(&rtc_timer))
		del_timer(&rtc_timer);
#endif

	DSR_WRITE_LOCK(&rtc_lock);

	__dsr_rtc_flush();

	DSR_WRITE_UNLOCK(&rtc_lock);
}

int __init dsr_rtc_init(void)
{
	proc_net_create(DSR_RTC_PROC_NAME, 0, dsr_rtc_proc_info);

#ifdef RTC_TIMER
	init_timer(&rtc_timer);
#endif
	return 0;
}
void __exit dsr_rtc_cleanup(void)
{
	dsr_rtc_flush();
	proc_net_remove(DSR_RTC_PROC_NAME);
}

EXPORT_SYMBOL(dsr_rtc_add);
EXPORT_SYMBOL(dsr_rtc_del);
EXPORT_SYMBOL(dsr_rtc_find);
EXPORT_SYMBOL(dsr_rtc_flush);

module_init(dsr_rtc_init);
module_exit(dsr_rtc_cleanup);
