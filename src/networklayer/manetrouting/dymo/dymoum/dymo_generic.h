/***************************************************************************
 *   Copyright (C) 2005 by Francisco J. Ros                                *
 *   fjrm@dif.um.es                                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __DYMO_GENERIC_H__
#define __DYMO_GENERIC_H__

#ifndef NS_NO_GLOBALS

#include "defs_dymo.h"
#ifndef OMNETPP
#include <sys/types.h>
#include <netinet/in.h>
#else
#include "compatibility.h"
#endif

/* Broadcast address (255.255.255.255) */
#define DYMO_BROADCAST ((in_addr_t) 0xFFFFFFFF)

/* DYMO port is not determined yet, so I am using AODV_PORT - 1 */
#define DYMO_PORT 653

/* TTL field in IP header for every DYMO packet */
#define DYMO_IPTTL 1

/* Maximum number of DYMO messages per second which can be generated
   by a node */
#ifndef OMNETPP
#define DYMO_RATELIMIT  10
#endif

/* Macro for incrementing a sequence number */
#define INC_SEQNUM(s) (s == 65535 ? s = 256 : s++)


/* DYMO message types */
#define DYMO_RE_TYPE    1
#define DYMO_RERR_TYPE  2
#define DYMO_UERR_TYPE  3
#define DYMO_HELLO_TYPE 4
#ifdef NS_PORT
#define DYMO_ECHOREPLY_TYPE 5
#endif  /* NS_PORT */

/* Network diameter (in number of hops) */
#ifndef OMNETPP
#define NET_DIAMETER    10
#endif

#ifndef OMNETPP
/* Generic DYMO element fixed struct */
#ifdef NS_PORT
struct DYMO_element     // FIXME: adjust byte ordering
{
#else
typedef struct
{
#endif  /* NS_PORT */
    u_int32_t   m : 1;
    u_int32_t   h : 2;
    u_int32_t   type : 5;
    u_int32_t   len : 12;
    u_int32_t   ttl : 6;
    u_int32_t   i : 1;
    u_int32_t   res : 5;

    u_int32_t   notify_addr; // if M bit set
    u_int32_t   target_addr; // if not a DYMOcast addr in IP dest addr

#ifdef NS_PORT
    static int offset_;
    inline static int& offset() { return offset_; }
    inline static DYMO_element *access(const Packet *p)
    {
        return (DYMO_element *) p->access(offset_);
    }
};

typedef DYMO_element hdr_dymoum;
#define HDR_DYMOUM(p) ((hdr_dymoum *) hdr_dymoum::access(p))
#else
} DYMO_element;
#endif  /* NS_PORT */

#endif
#endif  /* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS

/* Decrements TTL */
NS_STATIC NS_INLINE void generic_preprocess(DYMO_element *e)
{
    e->ttl--;
}

/* Returns true if TTL is greater than 0 */
// TODO: this must be changed according to the specification when support for
// multiple elements be added.
NS_STATIC NS_INLINE int generic_postprocess(DYMO_element *e)
{
    return (e->ttl > 0);
}

/* Processes a DYMO message */
void generic_process_message(DYMO_element *e,struct in_addr src, u_int32_t ifindex);

#endif  /* NS_NO_DECLARATIONS */

#endif  /* __DYMO_GENERIC_H__ */
