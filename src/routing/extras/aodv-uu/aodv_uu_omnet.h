/*****************************************************************************
 *
 * Copyright (C) 2002 Uppsala University.
 * Copyright (C) 2006 Malaga University.
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
 * Authors: Bj�n Wiberg <bjorn.wiberg@home.se>
 * Authors: Alfonso Ariza Quintana.<aarizaq@uma.ea>
 *
 *****************************************************************************/


#ifndef __INET_AODV_UU_OMNET_H
#define __INET_AODV_UU_OMNET_H

/* Constants for interface queue packet buffering/dropping */
#define IFQ_BUFFER 0
#define IFQ_DROP 1
#define IFQ_DROP_BY_DEST 2
#define PKT_ENC 0x1       /* Packet is encapsulated */
#define PKT_DEC 0x2 /* Packet arrived at GW and has been decapsulated (and
* should therefore be routed to the Internet */
// #define CONFIG_GATEWAY
// #define DEBUG_HELLO

#ifndef NS_PORT
#define NS_PORT
#endif
#ifndef OMNETPP
#define OMNETPP
#endif

/* This is a C++ port of AODV-UU for ns-2 */
#ifndef NS_PORT
#error "To compile the ported version, NS_PORT must be defined!"
#endif /* NS_PORT */

#ifndef AODV_USE_STL
#define AODV_USE_STL
#endif

#ifndef AODV_USE_STL_RT
#define AODV_USE_STL_RT
#endif

#define AODV_GLOBAL_STATISTISTIC

/* Global definitions and lib functions */
#include "aodv-uu/params.h"
#include "aodv-uu/defs_aodv.h"

/* System-dependent datatypes */
/* Needed by some network-related datatypes */
#include "inet/routing/extras/base/ManetRoutingBase.h"
#include "aodv-uu/list.h"

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

#include "aodv_msg_struct.h"
/* Forward declaration needed to be able to reference the class */

namespace inet { namespace inetmanet { class AODVUU; } }


#ifndef IP_BROADCAST
#define IP_BROADCAST ((u_int32_t) 0xffffffff)
#endif /* !IP_BROADCAST */

/* Extract global data types, defines and global declarations */
#undef NS_NO_GLOBALS
#define NS_NO_DECLARATIONS

#include "aodv-uu/timer_queue_aodv.h"
#include "aodv-uu/aodv_hello.h"
#include "aodv-uu/aodv_rerr.h"
#include "aodv-uu/aodv_rrep.h"
#include "aodv-uu/aodv_rreq.h"
#include "aodv-uu/aodv_socket.h"
#include "aodv-uu/aodv_timeout.h"
#include "aodv-uu/debug_aodv.h"
#include "aodv-uu/routing_table.h"
#include "aodv-uu/seek_list.h"
#include "aodv-uu/locality.h"

#include "inet/routing/extras/aodv-uu/packet_queue_omnet.h"

#undef NS_NO_DECLARATIONS

/* In omnet we don't care about byte order */
#undef ntohl
#define ntohl(x) x
#undef htonl
#define htonl(x) x
#undef htons
#define htons(x) x
#undef ntohs
#define ntohs(x) x

namespace inet {

namespace inetmanet {

/* The AODV-UU routing agent class */
class AODVUU : public ManetRoutingBase
{

  private:
    char nodeName[50];
    bool useIndex;
    bool isRoot;
    uint32_t costStatic;
    uint32_t costMobile;
    bool useHover;
    bool propagateProactive;
    struct timer proactive_rreq_timer;
    long proactive_rreq_timeout;
    bool isBroadcast (L3Address add)
    {
        if (this->isInMacLayer() && add==L3Address(MACAddress::BROADCAST_ADDRESS))
             return true;
        if (!this->isInMacLayer() && add==L3Address(IPv4Address::ALLONES_ADDRESS))
            return true;
        return false;
    }
    // cMessage  messageEvent;
    typedef std::multimap<simtime_t, struct timer*> AodvTimerMap;
    AodvTimerMap aodvTimerMap;
    typedef std::map<L3Address, struct rt_table*> AodvRtTableMap;
    AodvRtTableMap aodvRtTableMap;


  public:
    static int  log_file_fd;
    static bool log_file_fd_init;
    AODVUU() {isRoot = false; is_init = false; log_file_fd_init = false; sendMessageEvent = new cMessage();/*&messageEvent;*/}
    ~AODVUU();

    void actualizeTablesWithCollaborative(const L3Address &);

    void packetFailed(IPv4Datagram *p);
    void packetFailedMac(ieee80211::Ieee80211DataFrame *);

    // Routing information access
    virtual bool supportGetRoute() {return false;}
    virtual uint32_t getRoute(const L3Address &,std::vector<L3Address> &);
    virtual bool getNextHop(const L3Address &,L3Address &add,int &iface,double &);
    virtual bool isProactive();
    virtual void setRefreshRoute(const L3Address &destination, const L3Address & nextHop,bool isReverse);
    virtual bool setRoute(const L3Address & destination, const L3Address &nextHop, const int &ifaceIndex,const int &hops, const L3Address &mask=L3Address());
    virtual bool setRoute(const L3Address & destination, const L3Address &nextHop, const char *ifaceName,const int &hops, const L3Address &mask=L3Address());

  protected:
    bool is_init;
    void drop (cPacket *p,int cause = 0)
    {
        delete p;
        // icmpAccess.get()->sendErrorMessage(p, ICMP_DESTINATION_UNREACHABLE, cause);
    }
    int startAODVUUAgent();
    void scheduleNextEvent();
    const char *if_indextoname(int, char *);
    IPv4Datagram *pkt_encapsulate(IPv4Datagram *, IPv4Address);
    IPv4Datagram *pkt_decapsulate(IPv4Datagram *);
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);


    cMessage * sendMessageEvent;

    void recvAODVUUPacket(cMessage * p);
    void processPacket(IPv4Datagram *,unsigned int);
    void processMacPacket(cPacket * p, const L3Address &dest, const L3Address &src, int ifindex);

    int initialized;
    int  node_id;
    IPv4Address *gateWayAddress;

    int NS_DEV_NR;
    int NS_IFINDEX;
    // cModule *ipmod;

    /*
      Extract method declarations (and occasionally, variables)
      from header files
    */
#define NS_NO_GLOBALS
#undef NS_NO_DECLARATIONS

#undef _AODV_NEIGHBOR_H
#include "aodv-uu/aodv_neighbor.h"

#undef _AODV_HELLO_H
#include "aodv-uu/aodv_hello.h"

#undef _AODV_RERR_H
#include "aodv-uu/aodv_rerr.h"

#undef _AODV_RREP_H
#include "aodv-uu/aodv_rrep.h"

#undef _AODV_RREQ_H
#include "aodv-uu/aodv_rreq.h"

#undef _AODV_SOCKET_H
#include "aodv-uu/aodv_socket.h"

#undef _AODV_TIMEOUT_H
#include "aodv-uu/aodv_timeout.h"

#undef _DEBUG_H
#include "aodv-uu/debug_aodv.h"

#undef _ROUTING_TABLE_H
#include "aodv-uu/routing_table.h"

#undef _SEEK_LIST_H
#include "aodv-uu/seek_list.h"

#undef _TIMER_QUEUE_H
#include "aodv-uu/timer_queue_aodv.h"

#undef _LOCALITY_H
#include "aodv-uu/locality.h"

#undef _PACKET_QUEUE_H
#include "packet_queue_omnet.h"

#undef NS_NO_GLOBALS

    /* (Previously global) variables from main.c */
    int log_to_file;
    int rt_log_interval;
    int unidir_hack;
    int rreq_gratuitous;
    int expanding_ring_search;
    int internet_gw_mode;
    int local_repair;
    int receive_n_hellos;
    int hello_jittering;
    int optimized_hellos;
    int ratelimit;
    int llfeedback;
    char *progname;
    int wait_on_reboot;
    struct timer worb_timer;

    /* Parameters that are dynamic configuration values: */
    int active_route_timeout;
    int ttl_start;
    int delete_period;

    /* From aodv_hello.c */
    struct timer hello_timer;
#ifndef AODV_USE_STL
    /* From aodv_rreq.c */
    list_t rreqRecords;
#define rreq_records this->rreqRecords
    list_t rreqBlacklist;
#define  rreq_blacklist this->rreqBlacklist

    /* From seek_list.c */
    list_t seekHead;
#define seekhead this->seekHead

    /* From timer_queue_aodv.c */
    list_t timeList;
#define TQ this->timeList
#else
    typedef std::vector <rreq_record *>RreqRecords;
    typedef std::map <L3Address, struct blacklist *>RreqBlacklist;
    typedef std::map <L3Address, seek_list_t*>SeekHead;

    RreqRecords rreq_records;
    RreqBlacklist rreq_blacklist;
    SeekHead seekhead;
#endif
    /* From debug.c */
// int  log_file_fd;
    int log_rt_fd;
    int log_nmsgs;
    int debug;
    struct timer rt_log_timer;

    /* From defs.h */
    struct host_info this_host;
    struct dev_info dev_ifindex (int);
    struct dev_info dev_nr(int);
    unsigned int dev_indices[MAX_NR_INTERFACES];

//  inline int ifindex2devindex(unsigned int ifindex);
    int ifindex2devindex(unsigned int ifindex);
#ifdef AODV_GLOBAL_STATISTISTIC
    static bool iswrite;
    static int totalSend;
    static int totalRreqSend;
    static int totalRreqRec;
    static int totalRrepSend;
    static int totalRrepRec;
    static int totalRrepAckSend;
    static int totalRrepAckRec;
    static int totalRerrSend;
    static int totalRerrRec;
#else
    bool iswrite;
    int totalSend;
    int totalRreqSend;
    int totalRreqRec;
    int totalRrepSend;
    int totalRrepRec;
    int totalRrepAckSend;
    int totalRrepAckRec;
    int totalRerrSend;
    int totalRerrRec;
#endif
// used for break link notification
    //virtual void processPromiscuous(const cObject *details){};
    virtual void processLinkBreak(const cObject *details);
    //virtual void processFullPromiscuous(const cObject *details){}
    virtual bool isOurType(cPacket *);
    virtual bool getDestAddress(cPacket *,L3Address &);


};

#if 0
/* From defs.h (needs the AODVUU class declaration) */
inline int NS_CLASS ifindex2devindex(unsigned int ifindex)
{
    int i;

    for (i = 0; i < this_host.nif; i++)
        if (dev_indices[i] == ifindex)
            return i;

    return -1;
}
#endif

} // namespace inetmanet

} // namespace inet

#endif

