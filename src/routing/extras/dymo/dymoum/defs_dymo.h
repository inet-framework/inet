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

#ifndef __DEFS_H__
#define __DEFS_H__

#ifdef OMNETPP
#include "inet/routing/extras/base/compatibility.h"
#endif

#ifndef NS_NO_GLOBALS

#ifndef NS_PORT
#include <net/if.h>
#include <netinet/in.h>

#define NS_CLASS
#define NS_STATIC   static
#define NS_INLINE   inline
#else
#define NS_CLASS    DYMOUM::
#define NS_STATIC
#define NS_INLINE   inline
#ifndef OMNETPP
#define NS_DEV_NR   0
#define NS_IFINDEX  0
#endif
#endif  /* NS_PORT */

/* Version information */
#define DYMO_UM_VERSION "0.3"
#define DYMO_DRAFT_VERSION "Draft-05"

/* Misc defines */
#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif  /* MAX */

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif  /* MIN */

#ifndef NULL
#define NULL ((void *) 0)
#endif  /* NULL */

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif  /* IFNAMSIZ */

/* Maximum number of interfaces per node */
#define DYMO_MAX_NR_INTERFACES 10
#ifndef OMNETPP
/* Returns a dev_info struct given its corresponding iface index */
#define DEV_IFINDEX(ifindex) (this_host.devs[ifindex2devindex(ifindex)])
/* Returns a dev_info struct given its corresponding device number */
#define DEV_NR(n) (this_host.devs[n])
#else
#define DEV_IFINDEX(n) (this_host.devs[n])
#define DEV_NR(n) (this_host.devs[n])
#endif

namespace inet {

namespace inetmanet {

/* Data for a network device */
struct dev_info
{
    int         enabled;     /* 1 if struct is used, else 0 */
    int         sock;        /* DYMO socket associated with this device */
    int         icmp_sock;   /* Raw socket used to send/receive ICMP messages */
    u_int32_t       ifindex;     /* Index for this interface */
    char        ifname[IFNAMSIZ];/* Interface name */
    struct in_addr  ipaddr;      /* The local IP address */
    struct in_addr  bcast;       /* Broadcast address */
};

/* Data for a host */
struct host_info
{
    u_int32_t       seqnum;     /* Sequence number */
    u_int8_t        prefix : 7; /* Prefix */
    u_int8_t        is_gw : 1;  /* Is this host a gateway? */
    int         nif;        /* Number of interfaces to broadcast on */
    struct dev_info devs[DYMO_MAX_NR_INTERFACES];
};

} // namespace inetmanet

} // namespace inet

#endif  /* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS

/* Information about this host */
#ifndef OMNETPP
/* in omnet++ version this variables are defined private in the file dymo_um_omnet.h inside the definition of the class DYMOUM */

struct host_info this_host;


/* Array of interface indices */
u_int32_t dev_indices[DYMO_MAX_NR_INTERFACES];

/* Given a network interface index, returns the index into the
   devs array */
NS_STATIC NS_INLINE int ifindex2devindex(u_int32_t ifindex)
{
    int i;
    for (i = 0; i < this_host.nif; i++)
        if (dev_indices[i] == ifindex)
            return i;
    return -1;
}
#endif

#endif  /* NS_NO_DECLARATIONS */

#ifndef NS_PORT

/* Callback functions */
typedef void (*callback_func_t) (int);
int attach_callback_func(int fd, callback_func_t func);

/* Given a socket descriptor, returns the corresponding dev_info
   struct */
static inline struct dev_info *devfromsock(int sock)
{
    int i;

    for (i = 0; i < this_host.nif; i++)
        if (this_host.devs[i].sock == sock)
            return &this_host.devs[i];

    return NULL;
}

/* Given an ICMP socket descriptor, returns the corresponding dev_info
   struct */
static inline struct dev_info *devfromicmpsock(int icmp_sock)
{
    int i;

    for (i = 0; i < this_host.nif; i++)
        if (this_host.devs[i].icmp_sock == icmp_sock)
            return &this_host.devs[i];

    return NULL;
}

#endif  /* NS_PORT */


#endif  /* __DEFS_H__ */
