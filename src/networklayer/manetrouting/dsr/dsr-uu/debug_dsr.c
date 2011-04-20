/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/version.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#include "debug.h"
#include "dsr.h"
#include "timer.h"

atomic_t num_pkts = ATOMIC_INIT(0);

/* Most of this is shamelessly stolen from the linux kernel log routines */
#define DBG_LOG_BUF_LEN	(16384)
//#define DBG_LOG_BUF_LEN (1 << CONFIG_LOG_BUF_SHIFT)

static spinlock_t dbg_logbuf_lock = SPIN_LOCK_UNLOCKED;

static char __dbg_log_buf[DBG_LOG_BUF_LEN];
static char *dbg_log_buf = __dbg_log_buf;
static int dbg_log_buf_len = DBG_LOG_BUF_LEN;

#define DBG_LOG_BUF_MASK	(dbg_log_buf_len-1)
#define DBG_LOG_BUF(idx) (dbg_log_buf[(idx) & DBG_LOG_BUF_MASK])

static unsigned long dbg_log_start;	/* Index into dbg_log_buf: next char to be read by sysdbg_log() */
static unsigned long dbg_log_end;	/* Index into dbg_log_buf: most-recently-written-char + 1 */
static unsigned long logged_chars;	/* Number of chars produced since last read+clear operation */

DECLARE_WAIT_QUEUE_HEAD(dbg_log_wait);

int do_dbglog(int type, char *buf, int len)
{
	unsigned long i, j, limit, count;
	int do_clear = 0;
	char c;
	int error = 0;

	/* printk(KERN_DEBUG "do_dbglog\n"); */

	switch (type) {
	case 0:		/* Close log */
		break;
	case 1:		/* Open log */
		break;
	case 2:		/* Read from log */
		error = -EINVAL;
		if (!buf || len < 0)
			goto out;
		error = 0;
		if (!len)
			goto out;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,11)
		error = verify_area(VERIFY_WRITE, buf, len);
#else
		error = access_ok(VERIFY_WRITE, buf, len) ? 0 : 1;
#endif
		if (error)
			goto out;
		error =
		    wait_event_interruptible(dbg_log_wait,
					     (dbg_log_start - dbg_log_end));
		if (error)
			goto out;
		i = 0;
		/* printk(KERN_DEBUG "read\n"); */
		spin_lock_irq(&dbg_logbuf_lock);
		while (!error && (dbg_log_start != dbg_log_end) && i < len) {
			c = DBG_LOG_BUF(dbg_log_start);
			dbg_log_start++;
			spin_unlock_irq(&dbg_logbuf_lock);
			error = __put_user(c, buf);
			buf++;
			i++;
			cond_resched();
			spin_lock_irq(&dbg_logbuf_lock);
		}
		spin_unlock_irq(&dbg_logbuf_lock);
		if (!error)
			error = i;
		break;
	case 4:		/* Read/clear last kernel messages */
		do_clear = 1;
		/* FALL THRU */
	case 3:		/* Read last kernel messages */
		error = -EINVAL;
		if (!buf || len < 0)
			goto out;
		error = 0;
		if (!len)
			goto out;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,11)
		error = verify_area(VERIFY_WRITE, buf, len);
#else
		error = access_ok(VERIFY_WRITE, buf, len) ? 0 : 1;
#endif
		if (error)
			goto out;
		count = len;
		if (count > dbg_log_buf_len)
			count = dbg_log_buf_len;
		spin_lock_irq(&dbg_logbuf_lock);
		if (count > logged_chars)
			count = logged_chars;
		if (do_clear)
			logged_chars = 0;
		limit = dbg_log_end;
		/*
		 * __put_user() could sleep, and while we sleep
		 * printk() could overwrite the messages 
		 * we try to copy to user space. Therefore
		 * the messages are copied in reverse. <manfreds>
		 */
		for (i = 0; i < count && !error; i++) {
			j = limit - 1 - i;
			if (j + dbg_log_buf_len < dbg_log_end)
				break;
			c = DBG_LOG_BUF(j);
			spin_unlock_irq(&dbg_logbuf_lock);
			error = __put_user(c, &buf[count - 1 - i]);
			cond_resched();
			spin_lock_irq(&dbg_logbuf_lock);
		}
		spin_unlock_irq(&dbg_logbuf_lock);
		if (error)
			break;
		error = i;
		if (i != count) {
			int offset = count - error;
			/* buffer overflow during copy, correct user buffer. */
			for (i = 0; i < error; i++) {
				if (__get_user(c, &buf[i + offset]) ||
				    __put_user(c, &buf[i])) {
					error = -EFAULT;
					break;
				}
				cond_resched();
			}
		}
		break;
	case 5:		/* Clear ring buffer */
		logged_chars = 0;
		break;

	case 6:		/* Number of chars in the log buffer */
		error = dbg_log_end - dbg_log_start;
		break;
	case 7:		/* Size of the log buffer */
		error = dbg_log_buf_len;
		break;
	default:
		error = -EINVAL;
		break;
	}
      out:
	return error;
}
static void dsr_emit_log_char(char c)
{
	DBG_LOG_BUF(dbg_log_end) = c;
	dbg_log_end++;
	if (dbg_log_end - dbg_log_start > dbg_log_buf_len)
		dbg_log_start = dbg_log_end - dbg_log_buf_len;
	if (logged_chars < dbg_log_buf_len)
		logged_chars++;
}

int dsr_vprintk(const char *func, const char *fmt, va_list args)
{
	unsigned long flags;
	int printed_len, prefix_len;
	char *p;
	static char printk_buf[1024];
	struct timeval now;

	/* This stops the holder of console_sem just where we want him */
	spin_lock_irqsave(&dbg_logbuf_lock, flags);

	gettime(&now);

	prefix_len = sprintf(printk_buf, "%ld.%03ld: %s: ",
			     now.tv_sec, now.tv_usec / 1000, func);

	/* Emit the output into the temporary buffer */
	printed_len = vsnprintf(printk_buf + prefix_len,
				sizeof(printk_buf) - prefix_len, fmt, args);

	for (p = printk_buf; *p; p++)
		dsr_emit_log_char(*p);

	spin_unlock_irqrestore(&dbg_logbuf_lock, flags);
	return printed_len;
}

int trace(const char *func, const char *fmt, ...)
{
	va_list args;
	int r;

	va_start(args, fmt);
	r = dsr_vprintk(func, fmt, args);
	va_end(args);

	return r;
}

static int dbg_log_open(struct inode *inode, struct file *file)
{
	return do_dbglog(1, NULL, 0);
}

static int dbg_log_release(struct inode *inode, struct file *file)
{
	(void)do_dbglog(0, NULL, 0);
	return 0;
}

static ssize_t
dbg_log_read(struct file *file, char *buf, size_t count, loff_t * ppos)
{
	/* printk(KERN_DEBUG "dbg_log_read\n"); */
	if ((file->f_flags & O_NONBLOCK) && !do_dbglog(6, NULL, 0))
		return -EAGAIN;
	return do_dbglog(2, buf, count);
}

static unsigned int dbg_log_poll(struct file *file, poll_table * wait)
{
	poll_wait(file, &dbg_log_wait, wait);
	if (do_dbglog(6, NULL, 0))
		return POLLIN | POLLRDNORM;
	return 0;
}

struct file_operations proc_dbg_operations = {
	.read = dbg_log_read,
	.poll = dbg_log_poll,
	.open = dbg_log_open,
	.release = dbg_log_release,
};

int __init dbg_init(void)
{
	struct proc_dir_entry *entry;

	entry =
	    create_proc_entry("dsr_dbg", S_IRUSR | S_IRGRP | S_IROTH, proc_net);
	if (entry)
		entry->proc_fops = &proc_dbg_operations;

	return 0;
}

void __exit dbg_cleanup(void)
{
	proc_net_remove("dsr_dbg");
}

/* EXPORT_SYMBOL(trace); */
/* EXPORT_SYMBOL(dsr_vprintk); */
