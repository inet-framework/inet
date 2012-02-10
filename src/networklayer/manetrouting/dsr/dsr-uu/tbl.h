/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef _TBL_H
#define _TBL_H
#ifndef OMNETPP
#ifdef __KERNEL__
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#else
#include <stdlib.h>
#include <errno.h>
#include "dsr_list.h"
#endif
#else
#include "dsr_list.h"
#include <errno.h>
#endif

#define TBL_FIRST(tbl) (tbl)->head.next
#define TBL_EMPTY(tbl) (TBL_FIRST(tbl) == &(tbl)->head)
#define TBL_FULL(tbl) ((tbl)->len >= (tbl)->max_len)
#define tbl_is_first(tbl, e) (&e->l == TBL_FIRST(tbl))

#ifdef __KERNEL__

typedef struct list_head dsr_list_t;
#define LIST_INIT_HEAD(name) LIST_HEAD_INIT(name)

#define TBL_INIT(name, max_len) { LIST_INIT_HEAD(name.head), \
                                 0, \
                                 max_len, \
                                 RW_LOCK_UNLOCKED }
#define INIT_TBL(ptr, max_length) do { \
        (ptr)->head.next = (ptr)->head.prev = &((ptr)->head); \
        (ptr)->len = 0; (ptr)->max_len = max_length; \
        (ptr)->lock = RW_LOCK_UNLOCKED; \
} while (0)

#define DSR_WRITE_LOCK(l)   write_lock_bh(l)
#define DSR_WRITE_UNLOCK(l) write_unlock_bh(l)
#define DSR_READ_LOCK(l)    read_lock_bh(l)
#define DSR_READ_UNLOCK(l)  read_unlock_bh(l)

#define MALLOC(s, p)        kmalloc(s, p)
#define FREE(p)             kfree(p)


#else               /* __KERNEL__ */

#define TBL_INIT(name, max_len) { LIST_INIT_HEAD(name.head), \
                                 0, \
                                 max_len }
#define INIT_TBL(ptr, max_length) do { \
        (ptr)->head.next = (ptr)->head.prev = &((ptr)->head); \
        (ptr)->len = 0; (ptr)->max_len = max_length; \
} while (0)

#define DSR_WRITE_LOCK(l)
#define DSR_WRITE_UNLOCK(l)
#define DSR_READ_LOCK(l)
#define DSR_READ_UNLOCK(l)
//#define MALLOC(s, p)        malloc(s)
//#define FREE(p)             free(p)
#define MALLOC(s, p) new char[s]
#define FREE(p) delete [] p;

#endif              /* __KERNEL__ */

#define TBL(name, max_len) \
    struct tbl name = TBL_INIT(name, max_len)

struct tbl
{
    dsr_list_t head;
    volatile unsigned int len;
    volatile unsigned int max_len;
#ifdef __KERNEL__
    rwlock_t lock;
#endif
};

/* Criteria function should return 1 if the criteria is fulfilled or 0 if not
 * fulfilled */
typedef int (*criteria_t) (void *elm, void *data);
typedef int (*do_t) (void *elm, void *data);

static inline int crit_none(void *foo, void *bar)
{
    return 1;
}

/* Functions prefixed with "__" are unlocked, the others are safe. */

static inline int tbl_empty(struct tbl *t)
{
    int res = 0;
    DSR_READ_LOCK(&t->lock);

    if (TBL_FIRST(t) == &(t)->head)
        res = 1;

    DSR_READ_UNLOCK(&t->lock);
    return res;
}

static inline int __tbl_add(struct tbl *t, dsr_list_t * l, criteria_t crit)
{
    int len;

    if (t->len >= t->max_len)
    {
        //printk(KERN_WARNING "Max list len reached\n");
        return -ENOSPC;
    }

    if (list_empty(&t->head))
    {
        list_add(l, &t->head);
    }
    else
    {
        dsr_list_t *pos;

        list_for_each(pos, &t->head)
        {

            if (crit(pos, l))
                break;
        }
        list_add(l, pos->prev);
    }

    len = ++t->len;

    return len;
}

static inline int __tbl_add_tail(struct tbl *t, dsr_list_t * l)
{
    int len;

    if (t->len >= t->max_len)
    {
        //printk(KERN_WARNING "Max list len reached\n");
        return -ENOSPC;
    }

    list_add_tail(l, &t->head);

    len = ++t->len;

    return len;
}

static inline int tbl_add_tail(struct tbl *t, dsr_list_t * l)
{
    int len;
    DSR_WRITE_LOCK(&t->lock);
    len = __tbl_add_tail(t, l);
    DSR_WRITE_UNLOCK(&t->lock);
    return len;
}

static inline void *__tbl_find(struct tbl *t, void *id, criteria_t crit)
{
    dsr_list_t *pos;

    list_for_each(pos, &t->head)
    {
        if (crit(pos, id))
            return pos;
    }
    return NULL;
}

static inline void *__tbl_detach(struct tbl *t, dsr_list_t * l)
{
    //int len;

    if (TBL_EMPTY(t))
        return NULL;

    list_del(l);

    //len = --t->len;
    t->len--;

    return l;
}

static inline int __tbl_del(struct tbl *t, dsr_list_t * l)
{

    if (!__tbl_detach(t, l))
        return -1;

    FREE(l);

    return 1;
}

static inline int __tbl_find_do(struct tbl *t, void *data, do_t func)
{
    dsr_list_t *pos, *tmp;

    list_for_each_safe(pos, tmp, &t->head)
    if (func(pos, data))
        return 1;

    return 0;
}

static inline int tbl_find_do(struct tbl *t, void *data, do_t func)
{
    int res;

    DSR_WRITE_LOCK(&t->lock);
    res = __tbl_find_do(t, data, func);
    DSR_WRITE_UNLOCK(&t->lock);

    return res;
}

static inline int __tbl_do_for_each(struct tbl *t, void *data, do_t func)
{
    dsr_list_t *pos;
    int res = 0;

    list_for_each(pos, &t->head)
    res += func(pos, data);

    return res;
}

static inline int tbl_do_for_each(struct tbl *t, void *data, do_t func)
{
    int res;

    DSR_WRITE_LOCK(&t->lock);
    res = __tbl_do_for_each(t, data, func);
    DSR_WRITE_UNLOCK(&t->lock);

    return res;
}

static inline void *tbl_find_detach(struct tbl *t, void *id, criteria_t crit)
{
    dsr_list_t *e;

    DSR_WRITE_LOCK(&t->lock);

    e = (dsr_list_t *) __tbl_find(t, id, crit);

    if (!e)
    {
        DSR_WRITE_UNLOCK(&t->lock);
        return NULL;
    }
    list_del(e);
    t->len--;

    DSR_WRITE_UNLOCK(&t->lock);

    return e;
}

static inline void *tbl_detach(struct tbl *t, dsr_list_t * l)
{
    void *e;

    DSR_WRITE_LOCK(&t->lock);
    e = __tbl_detach(t, l);
    DSR_WRITE_UNLOCK(&t->lock);
    return e;
}

static inline void *tbl_detach_first(struct tbl *t)
{
    dsr_list_t *e;

    DSR_WRITE_LOCK(&t->lock);

    if (TBL_EMPTY(t))
    {
        DSR_WRITE_UNLOCK(&t->lock);
        return NULL;
    }

    e = TBL_FIRST(t);

    list_del(e);
    t->len--;

    DSR_WRITE_UNLOCK(&t->lock);

    return e;
}

static inline int tbl_add(struct tbl *t, dsr_list_t * l, criteria_t crit)
{
    int len;

    DSR_WRITE_LOCK(&t->lock);
    len = __tbl_add(t, l, crit);
    DSR_WRITE_UNLOCK(&t->lock);
    return len;
}

static inline int tbl_del(struct tbl *t, dsr_list_t * l)
{
    int res;

    DSR_WRITE_LOCK(&t->lock);

    res = __tbl_del(t, l);

    DSR_WRITE_UNLOCK(&t->lock);

    return res;
}
static inline int tbl_find_del(struct tbl *t, void *id, criteria_t crit)
{
    dsr_list_t *e;

    DSR_WRITE_LOCK(&t->lock);

    e = (dsr_list_t *) __tbl_find(t, id, crit);

    if (!e)
    {
        DSR_WRITE_UNLOCK(&t->lock);
        return -1;
    }
    list_del(e);
    t->len--;
    FREE(e);

    DSR_WRITE_UNLOCK(&t->lock);

    return 1;
}

static inline int tbl_del_first(struct tbl *t)
{
    dsr_list_t *l;
    int n = 0;

    l = (dsr_list_t *) tbl_detach_first(t);

    FREE(l);

    return n;
}
static inline int tbl_for_each_del(struct tbl *t, void *id, criteria_t crit)
{
    dsr_list_t *pos, *tmp;
    int n = 0;

    DSR_WRITE_LOCK(&t->lock);

    list_for_each_safe(pos, tmp, &t->head)
    {
        if (crit(pos, id))
        {
            list_del(pos);
            t->len--;
            n++;
            FREE(pos);
        }
    }
    DSR_WRITE_UNLOCK(&t->lock);

    return n;
}

static inline int in_tbl(struct tbl *t, void *id, criteria_t crit)
{
    DSR_READ_LOCK(&t->lock);
    if (__tbl_find(t, id, crit))
    {
        DSR_READ_UNLOCK(&t->lock);
        return 1;
    }
    DSR_READ_UNLOCK(&t->lock);
    return 0;
}

static inline void tbl_flush(struct tbl *t, do_t at_flush)
{
    dsr_list_t *pos, *tmp;

    DSR_WRITE_LOCK(&t->lock);

    list_for_each_safe(pos, tmp, &t->head)
    {
        list_del(pos);

        if (at_flush)
            at_flush(pos, NULL);

        t->len--;
        FREE(pos);
    }
    DSR_WRITE_UNLOCK(&t->lock);
}

#endif              /* _TBL_H */
