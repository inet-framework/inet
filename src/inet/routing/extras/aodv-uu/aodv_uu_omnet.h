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
#include <deque>
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
    int  RERR_UDEST_SIZE;
    int RERR_SIZE;
    int RREP_SIZE;
    int  RREQ_SIZE;

    opp_string nodeName;
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
        if (this->isInMacLayer() && add==L3Address(MacAddress::BROADCAST_ADDRESS))
             return true;
        if (!this->isInMacLayer() && add==L3Address(Ipv4Address::ALLONES_ADDRESS))
            return true;
        return false;
    }
    // cMessage  messageEvent;
    typedef std::multimap<simtime_t, struct timer*> AodvTimerMap;
    AodvTimerMap aodvTimerMap;
    typedef std::map<L3Address, struct rt_table*> AodvRtTableMap;
    AodvRtTableMap aodvRtTableMap;

    // this static map simulate the exchange of seq num by the proactive protocol.
    static std::map<L3Address,u_int32_t *> mapSeqNum;


  private:
    class PacketDestOrigin
    {
        private:
            L3Address dest;
            L3Address origin;
        public:
            PacketDestOrigin() {}
            PacketDestOrigin(const L3Address &s,const L3Address &o)
            {
                dest = s;
                origin = o;
            }
            L3Address getDest() {return dest;}
            void setDests(const L3Address & s) {dest = s;}
            L3Address getOrigin() {return origin;}
            void setOrigin(const L3Address & s) {origin = s;}

            inline bool operator<(const PacketDestOrigin& b) const
            {
                if (dest != b.dest)
                    return dest < b.dest;
                else
                    return origin < b.origin;
            }
            inline bool operator == (const PacketDestOrigin& b) const
            {
                if (dest == b.dest && origin == b.origin)
                    return true;
                else
                    return false;
            }
            PacketDestOrigin& operator=(const PacketDestOrigin& a)
            {
                dest = a.dest; origin = a.origin; return *this;
            }
    };

    struct RREPProcessed
    {
        u_int8_t hcnt;
        u_int8_t totalHops;
        u_int32_t dest_seqno;
        u_int32_t origin_seqno;
        uint32_t cost;
        uint8_t  hopfix;
        L3Address next;
    };

    struct RREQInfo
    {
        u_int8_t hcnt;
        u_int32_t dest_seqno;
        u_int32_t origin_seqno;
        uint32_t cost;
        uint8_t  hopfix;
        Packet *pkt = nullptr;
    };

    struct RREQProcessed : cMessage
    {
        PacketDestOrigin destOrigin;
        std::deque<RREQInfo> infoList;
    };

    std::map<PacketDestOrigin,RREPProcessed> rrepProc;
    std::map<PacketDestOrigin,RREQProcessed*> rreqProc;

    struct DelayInfo : public cObject
    {
        struct in_addr dst;
        int len;
        u_int8_t ttl;
        struct dev_info *dev = nullptr;
    };
    bool storeRreq;
    bool checkRrep;
    virtual bool isThisRrepPrevSent(Ptr<const AODV_msg>);
    virtual bool getDestAddressRreq(Packet *msg,PacketDestOrigin &orgDest,RREQInfo &rreqInfo);
  public:
    static int  log_file_fd;
    static bool log_file_fd_init;
    AODVUU() { progname = nullptr; isRoot = false; is_init = false; log_file_fd_init = false; sendMessageEvent = new cMessage("AodvUU-sendMessageEvent"); mapSeqNum.clear(); /*&messageEvent;*/storeRreq = false;}
    ~AODVUU();

    void actualizeTablesWithCollaborative(const L3Address &);

    void packetFailed(const Packet *p);
    void packetFailedMac(const Packet *);

    // Routing information access
    virtual bool supportGetRoute() override {return false;}
    virtual uint32_t getRoute(const L3Address &,std::vector<L3Address> &) override;
    virtual bool getNextHop(const L3Address &,L3Address &add,int &iface,double &) override;
    virtual bool isProactive() override;
    virtual void setRefreshRoute(const L3Address &destination, const L3Address & nextHop,bool isReverse) override;
    virtual bool setRoute(const L3Address & destination, const L3Address &nextHop, const int &ifaceIndex,const int &hops, const L3Address &mask=L3Address()) override;
    virtual bool setRoute(const L3Address & destination, const L3Address &nextHop, const char *ifaceName,const int &hops, const L3Address &mask=L3Address()) override;

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual INetfilter::IHook::Result ensureRouteForDatagram(Packet *datagram) override;

  protected:
    Packet * pkt_encapsulate(Packet *p, Ipv4Address gateway);


    bool is_init;
    void drop (Packet *p,int cause = 0)
    {
        delete p;
        // icmpAccess.get()->sendErrorMessage(p, ICMP_DESTINATION_UNREACHABLE, cause);
    }
    int startAODVUUAgent();
    void scheduleNextEvent();
    const char *if_indextoname(int, char *);
/*    IPv4Datagram *pkt_encapsulate(IPv4Datagram *, IPv4Address);
    IPv4Datagram *pkt_decapsulate(IPv4Datagram *);*/
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;


    cMessage * sendMessageEvent = nullptr;

    void recvAODVUUPacket(Packet * p);
    INetfilter::IHook::Result processPacket(Packet *,unsigned int);
    void processMacPacket(Packet * p, const L3Address &dest, const L3Address &src, int ifindex);

    int initialized;
    int  node_id;
    L3Address *gateWayAddress = nullptr;

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
    const char *progname;
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
    static int totalLocalRep;
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
    int totalLocalRep;
#endif

    // used for break link notification
    virtual void processLinkBreak(const Packet *details) override;
    virtual bool isOurType(const Packet *) override;
    virtual bool getDestAddress(Packet *,L3Address &) override;

    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;

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

