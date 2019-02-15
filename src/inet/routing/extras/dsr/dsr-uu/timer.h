/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef _TIMER_H
#define _TIMER_H

#include <omnetpp.h>

namespace inet {

namespace inetmanet {

typedef unsigned long usecs_t;

/* Omnet++ */

class DSRUU;

typedef void (DSRUU::*fct_t) (void *data);
class DSRUUTimer:public cOwnedObject
{
  protected:
    cMessage msgtimer;
    DSRUU *a_;


  public:
    simtime_t expires;
    fct_t function;
    void *data;


    DSRUUTimer(DSRUU * a):cOwnedObject()
    {
        expires = 0;
        a_ = a;
    }

    DSRUUTimer():cOwnedObject()
    {
        a_ = (DSRUU*) getOwner();
        expires = 0;
    }

    ~DSRUUTimer() {cancel();}

    void setOwer(cOwnedObject *owner_)
    {
        a_ = (DSRUU*) owner_;
    }
    DSRUUTimer(DSRUU * a, const char *name):cOwnedObject(name)
    {
        a_ = a;
    }
    void init(simtime_t expires_, fct_t fct_, void *data_)
    {
        expires = expires_;
        data = data_;
        function = fct_;
    }
    void init(fct_t fct_, void *data_)
    {
        expires = 0;
        data = data_;
        function = fct_;
    }
    const char *get_name()
    {
        return getName();
    }
    cMessage * getMsgTimer()
    {
        return (&msgtimer);
    }
    bool pending() {return msgtimer.isScheduled();}
    bool test(cMessage *msg )
    {
        return (msg==&msgtimer);
    }
    simtime_t getExpires() {return expires;}
    void setExpires(double exp) {expires = exp;}
    bool testAndExcute(cMessage *msg)
    {
        if (msg==&msgtimer)
        {
            (a_->*function)(data);
            return true;
        }
        else
            return false;
    }

    void  resched(double delay);


    void cancel();



};

static inline char *print_timeval(struct timeval *tv)
{
    static char buf[56][56];
    static int n = 0;

    n = (n + 1) % 2;
    /*#ifdef _WIN32
    #define snprintf _snprintf_s
    #endif
    */
    snprintf(buf[n], sizeof(buf), "%ld:%02ld:%03lu", tv->tv_sec / 60,
             tv->tv_sec % 60, tv->tv_usec / 1000);

    snprintf(buf[n], sizeof(buf), "%ld:%02ld:%03lu", tv->tv_sec / 60,
             tv->tv_sec % 60, tv->tv_usec / 1000);

    return buf[n];
}

/* These functions may overflow (although unlikely)... Should probably be
 * improtved in the future */
static inline long timeval_diff(struct timeval *tv1, struct timeval *tv2)
{
    if (!tv1 || !tv2)
        return 0;
    else
        return ((tv1->tv_sec - tv2->tv_sec) * 1000000 +
                tv1->tv_usec - tv2->tv_usec);
}

static inline int timeval_add_usecs(struct timeval *tv, usecs_t usecs)
{
    long add;

    if (!tv)
        return -1;

    add = tv->tv_usec + usecs;
    tv->tv_sec += add / 1000000;
    tv->tv_usec = add % 1000000;

    return 0;
}

} // namespace inetmanet

} // namespace inet

#endif              /* _TIMER_H */
