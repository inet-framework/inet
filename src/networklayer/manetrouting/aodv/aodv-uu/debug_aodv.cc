/*****************************************************************************
 *
 * Copyright (C) 2001 Uppsala University & Ericsson AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Erik Nordstr�m, <erik.nordstrom@it.uu.se>
 *
 *
 *****************************************************************************/
#define NS_PORT
#define OMNETPP

#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#ifdef NS_PORT

#ifndef OMNETPP
#include "ns/aodv-uu.h"
#else
#include "../aodv_uu_omnet.h"
#endif

#else
#include <net/if.h>
#include "aodv_rreq.h"
#include "aodv_rrep.h"
#include "aodv_rerr.h"
#include "defs_aodv.h"
#include "debug_aodv.h"
#include "params.h"
#include "timer_queue_aodv.h"
#include "routing_table.h"
#endif

#ifndef NS_PORT
extern int log_to_file, rt_log_interval;
extern char *progname;
int log_file_fd = -1;
int log_rt_fd = -1;
int log_nmsgs = 0;
int debug = 0;
struct timer rt_log_timer;
#endif

void NS_CLASS log_init()
{
#ifndef _WIN32
    /* NS_PORT: Log filename is prefix + IP address + suffix */
#ifdef NS_PORT

    char AODV_LOG_PATH[strlen(AODV_LOG_PATH_PREFIX) +
                       strlen(AODV_LOG_PATH_SUFFIX) + 16];
    char AODV_RT_LOG_PATH[strlen(AODV_LOG_PATH_PREFIX) +
                          strlen(AODV_RT_LOG_PATH_SUFFIX) + 16];


    sprintf(AODV_LOG_PATH, "%s%d%s", AODV_LOG_PATH_PREFIX, node_id,
            AODV_LOG_PATH_SUFFIX);
    sprintf(AODV_RT_LOG_PATH, "%s%d%s", AODV_LOG_PATH_PREFIX, node_id,
            AODV_RT_LOG_PATH_SUFFIX);

#endif              /* NS_PORT */

    if (log_to_file)
    {
        if ((log_file_fd =
                    open(AODV_LOG_PATH, O_RDWR | O_CREAT | O_TRUNC,
                         S_IROTH | S_IWUSR | S_IRUSR | S_IRGRP)) < 0)
        {
            perror("open log file failed!");
            exit(-1);
        }
    }
    if (rt_log_interval)
    {
        if ((log_rt_fd =
                    open(AODV_RT_LOG_PATH, O_RDWR | O_CREAT | O_TRUNC,
                         S_IROTH | S_IWUSR | S_IRUSR | S_IRGRP)) < 0)
        {
            perror("open rt log file failed!");
            exit(-1);
        }
    }
    openlog(progname, 0, LOG_DAEMON);
#endif
}

void NS_CLASS log_rt_table_init()
{
    timer_init(&rt_log_timer, &NS_CLASS print_rt_table, NULL);
    timer_set_timeout(&rt_log_timer, rt_log_interval);
}

void NS_CLASS log_cleanup()
{
#ifndef _WIN32
    if (log_to_file && log_file_fd)
    {
        if (NS_OUTSIDE_CLASS close(log_file_fd) < 0)
            fprintf(stderr, "Could not close log_file_fd!\n");
    }
#endif
}

void NS_CLASS write_to_log_file(char *msg, int len)
{
#ifndef _WIN32
    if (!log_file_fd)
    {
        fprintf(stderr, "Could not write to log file\n");
        return;
    }
    if (len <= 0)
    {
        fprintf(stderr, "len=0\n");
        return;
    }
    if (write(log_file_fd, msg, len) < 0)
        perror("Could not write to log file");
#endif
}

const char *packet_type(uint32 type)
{
    static char temp[50];

    switch (type)
    {
    case AODV_RREQ:
        return "AODV_RREQ";
    case AODV_RREP:
        return "AODV_RREP";
    case AODV_RERR:
        return "AODV_RERR";
    default:
        sprintf(temp, "Unknown packet type %d", type);
        return temp;
    }
}

void NS_CLASS alog(int type, int errnum, const char *function, const char *format,
                   ...)
{
#ifndef _WIN32
    va_list ap;
    static char buffer[256] = "";
    static char log_buf[1024];
    char *msg;
    struct timeval now;
    struct tm *time;
    int len = 0;

    /* NS_PORT: Include IP address in log */
#ifdef NS_PORT
    if (DEV_NR(NS_DEV_NR).enabled == 1)
    {
        len += sprintf(log_buf + len, "%s: ",
                       ip_to_str(DEV_NR(NS_DEV_NR).ipaddr));
    }
#endif              /* NS_PORT */

    va_start(ap, format);

    if (type == LOG_WARNING)
        msg = &buffer[9];
    else
        msg = buffer;

    vsprintf(msg, format, ap);
    va_end(ap);

    if (!debug && !log_to_file)
        goto syslog;

    gettimeofday(&now, NULL);

#ifdef NS_PORT
    time = gmtime(&now.tv_sec);
#else
    time = localtime(&now.tv_sec);
#endif

    /*   if (type <= LOG_NOTICE) */
    /*  len += sprintf(log_buf + len, "%s: ", progname); */

    len += sprintf(log_buf + len, "%s %02d:%02d:%02d.%03ld %s: %s",nodeName, time->tm_hour,
                   time->tm_min, time->tm_sec, now.tv_usec / 1000, function,
                   msg);
//    len += sprintf(log_buf + len, "%s time = %lf - %s %s ",nodeName, simTime(), function,
//         msg);

    if (errnum == 0)
        len += sprintf(log_buf + len, "\n");
    else
        len += sprintf(log_buf + len, ": %s\n", strerror(errnum));

    if (len > 1024)
    {
        fprintf(stderr, "alog(): buffer to small! len = %d\n", len);
        goto syslog;
    }

    /* OK, we are clear to write the buffer to the aodv log file... */
    if (log_to_file)
        write_to_log_file(log_buf, len);

    /* If we have the debug option set, also write to stdout */
    if (debug)
        printf(log_buf);

    /* Syslog all messages that are of severity LOG_NOTICE or worse */
syslog:
    if (type <= LOG_NOTICE)
    {
        if (errnum != 0)
        {
            errno = errnum;
            syslog(type, "%s: %s: %m", function, msg);
        }
        else
            syslog(type, "%s: %s", function, msg);
    }
    /* Exit on error */
    if (type <= LOG_ERR)
        exit(-1);
#endif
}


char *NS_CLASS rreq_flags_to_str(RREQ * rreq)
{
    static char buf[5];
    int len = 0;
    char *str;

    if (rreq->j)
        buf[len++] = 'J';
    if (rreq->r)
        buf[len++] = 'R';
    if (rreq->g)
        buf[len++] = 'G';
    if (rreq->d)
        buf[len++] = 'D';

    buf[len] = '\0';

    str = buf;
    return str;
}

char *NS_CLASS rrep_flags_to_str(RREP * rrep)
{
    static char buf[3];
    int len = 0;
    char *str;

    if (rrep->r)
        buf[len++] = 'R';
    if (rrep->a)
        buf[len++] = 'A';

    buf[len] = '\0';

    str = buf;
    return str;
}

void NS_CLASS log_pkt_fields(AODV_msg * msg)
{

    RREQ *rreq;
    RREP *rrep;
    RERR *rerr;
    struct in_addr dest, orig;

    switch (msg->type)
    {
    case AODV_RREQ:
        rreq = (RREQ *) msg;
        dest.s_addr = rreq->dest_addr;
        orig.s_addr = rreq->orig_addr;
        DEBUG(LOG_DEBUG, 0,
              "rreq->flags:%s rreq->hopcount=%d rreq->rreq_id=%ld",
              rreq_flags_to_str(rreq), rreq->hcnt, ntohl(rreq->rreq_id));
        DEBUG(LOG_DEBUG, 0, "rreq->dest_addr:%s rreq->dest_seqno=%lu",
              ip_to_str(dest), ntohl(rreq->dest_seqno));
        DEBUG(LOG_DEBUG, 0, "rreq->orig_addr:%s rreq->orig_seqno=%ld",
              ip_to_str(orig), ntohl(rreq->orig_seqno));
        break;
    case AODV_RREP:
        rrep = (RREP *) msg;
        dest.s_addr = rrep->dest_addr;
        orig.s_addr = rrep->orig_addr;
        DEBUG(LOG_DEBUG, 0, "rrep->flags:%s rrep->hcnt=%d",
              rrep_flags_to_str(rrep), rrep->hcnt);
        DEBUG(LOG_DEBUG, 0, "rrep->dest_addr:%s rrep->dest_seqno=%d",
              ip_to_str(dest), ntohl(rrep->dest_seqno));
        DEBUG(LOG_DEBUG, 0, "rrep->orig_addr:%s rrep->lifetime=%d",
              ip_to_str(orig), ntohl(rrep->lifetime));
        break;
    case AODV_RERR:
        rerr = (RERR *) msg;
        DEBUG(LOG_DEBUG, 0, "rerr->dest_count:%d rerr->flags=%s",
              rerr->dest_count, rerr->n ? "N" : "-");
        break;
    }
}

char *NS_CLASS rt_flags_to_str(u_int16_t flags)
{
    static char buf[5];
    int len = 0;
    char *str;

    if (flags & RT_UNIDIR)
        buf[len++] = 'U';
    if (flags & RT_REPAIR)
        buf[len++] = 'R';
    if (flags & RT_INET_DEST)
        buf[len++] = 'I';
    if (flags & RT_GATEWAY)
        buf[len++] = 'G';
    buf[len] = '\0';

    str = buf;
    return str;
}

const char *NS_CLASS state_to_str(u_int8_t state)
{
    if (state == VALID)
        return "VAL";
    else if (state == INVALID)
        return "INV";
    else
        return "?";
}

char *NS_CLASS devs_ip_to_str()
{
    static char buf[MAX_NR_INTERFACES * IFNAMSIZ];
    char *str;
    int i, index = 0;

    for (i = 0; i < MAX_NR_INTERFACES; i++)
    {
        if (!DEV_NR(i).enabled)
            continue;
        index += sprintf(buf + index, "%s,", ip_to_str(DEV_NR(i).ipaddr));
    }
    str = buf;
    return str;
}

void NS_CLASS print_rt_table(void *arg)
{
#ifndef _WIN32
    char rt_buf[2048], ifname[64], seqno_str[11];
    int len = 0;
    int i = 0;
    struct timeval now;
    struct tm *time;

    if (rt_tbl.num_entries == 0)
        goto schedule;

    gettimeofday(&now, NULL);

#ifdef NS_PORT
    time = gmtime(&now.tv_sec);
#else
    time = localtime(&now.tv_sec);
#endif

    len +=
        sprintf(rt_buf,
                "# Time: %02d:%02d:%02d.%03ld IP: %s seqno: %u entries/active: %u/%u\n",
                time->tm_hour, time->tm_min, time->tm_sec, now.tv_usec / 1000,
                devs_ip_to_str(), this_host.seqno, rt_tbl.num_entries,
                rt_tbl.num_active);
    len +=
        sprintf(rt_buf + len,
                "%-15s %-15s %-3s %-3s %-5s %-6s %-5s %-5s %-15s\n",
                "Destination", "Next hop", "HC", "St.", "Seqno", "Expire",
                "Flags", "Iface", "Precursors");

    write(log_rt_fd, rt_buf, len);
    len = 0;

    for (i = 0; i < RT_TABLESIZE; i++)
    {
        list_t *pos;
        list_foreach(pos, &rt_tbl.tbl[i])
        {
            rt_table_t *rt = (rt_table_t *) pos;

            if (rt->dest_seqno == 0)
                sprintf(seqno_str, "-");
            else
                sprintf(seqno_str, "%u", rt->dest_seqno);

            /* Print routing table entries one by one... */
            if (list_empty(&rt->precursors))
                len += sprintf(rt_buf + len,
                               "%-15s %-15s %-3d %-3s %-5s %-6lu %-5s %-5s\n",
                               ip_to_str(rt->dest_addr),
                               ip_to_str(rt->next_hop), rt->hcnt,
                               state_to_str(rt->state), seqno_str,
                               (rt->hcnt == 255) ? 0 :
                               timeval_diff(&rt->rt_timer.timeout, &now),
                               rt_flags_to_str(rt->flags),
                               if_indextoname(rt->ifindex, ifname));

            else
            {
                list_t *pos2;
                len += sprintf(rt_buf + len,
                               "%-15s %-15s %-3d %-3s %-5s %-6lu %-5s %-5s %-15s\n",
                               ip_to_str(rt->dest_addr),
                               ip_to_str(rt->next_hop), rt->hcnt,
                               state_to_str(rt->state), seqno_str,
                               (rt->hcnt == 255) ? 0 :
                               timeval_diff(&rt->rt_timer.timeout, &now),
                               rt_flags_to_str(rt->flags),
                               if_indextoname(rt->ifindex, ifname),
                               ip_to_str(((precursor_t *) rt->precursors.next)->
                                         neighbor));

                /* Print all precursors for the current routing entry */
                list_foreach(pos2, &rt->precursors)
                {
                    precursor_t *pr = (precursor_t *) pos2;

                    /* Skip first entry since it is already printed */
                    if (pos2->prev == &rt->precursors)
                        continue;

                    len += sprintf(rt_buf + len, "%64s %-15s\n", " ",
                                   ip_to_str(pr->neighbor));

                    /* Since the precursor list is grown dynamically
                     * the write buffer should be flushed for every
                     * entry to avoid buffer overflows */
                    write(log_rt_fd, rt_buf, len);
                    len = 0;

                }
            }
            if (len > 0)
            {
                write(log_rt_fd, rt_buf, len);
                len = 0;
            }
        }
    }
    /* Schedule a new printing of routing table... */
schedule:
    timer_set_timeout(&rt_log_timer, rt_log_interval);
#endif
}

/* This function lets you print more than one IP address at the same time */
char *NS_CLASS ip_to_str(struct in_addr addr)
{
    static char buf[16 * 4];
    static int index = 0;
    char *str;
#ifdef NS_PORT
#ifndef OMNETPP
#undef htonl
#endif
    addr.s_addr = htonl(addr.s_addr);
#endif
#ifdef OMNETPP
    IPAddress add_aux = addr.s_addr;
    strcpy(&buf[index],add_aux.str().c_str());
#else
    strcpy(&buf[index], inet_ntoa(addr));
#endif
    str = &buf[index];
    index += 16;
    index %= 64;
    return str;
}

