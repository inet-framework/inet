/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef __INET_DSR_UU_OMNETPP_H
#define __INET_DSR_UU_OMNETPP_H


#ifndef OMNETPP
#define OMNETPP
#endif


#include "inet/routing/extras/base/compatibility.h"

// Force to considere all links bi dir (link cache disjktra)
#define BIDIR
//#include <stdarg.h>

#include "inet/routing/extras/base/ManetRoutingBase.h"
#include "inet/routing/extras/dsr/DsrUuPktOmnet.h"
#include "inet/routing/extras/dsr/dsr-uu/DsrDataBase.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/networklayer/common/NextHopAddressTag_m.h"

#include "inet/routing/extras/dsr/dsr-uu/dsr_options.h"
#include "inet/routing/extras/dsr/dsr-uu/timer.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-pkt.h"

// generate ev prints
#ifdef _WIN32
#define DEBUG omnet_debug
#else
#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG(f, args...) omnet_debug(f, ## args)
#endif


#ifndef DSR_ADDRESS_SIZE
#define DSR_ADDRESS_SIZE 4
#endif

/*
#ifdef _WIN32
#define DEBUG(f, args)
#else
#ifdef DEBUG
#undef DEBUG
#define DEBUG_PROC
#define DEBUG(f, args...)
//#define DEBUG(f, args...) trace(__FUNCTION__, f, ## args)
#else
#define DEBUG(f, args...)
#endif
#endif
*/

namespace inet {

namespace inetmanet {

// from dsr-ack.h
#define DSR_ACK_REQ_HDR_LEN sizeof(struct dsr_ack_req_opt)
#define DSR_ACK_REQ_OPT_LEN (DSR_ACK_REQ_HDR_LEN - 2)
#define DSR_ACK_HDR_LEN sizeof(struct dsr_ack_opt)
#define DSR_ACK_OPT_LEN (DSR_ACK_HDR_LEN - 2)

// from dsr-opt.h
#define DSR_NO_NEXT_HDR_TYPE 0
/* Header lengths */
#define DSR_FIXED_HDR_LEN 4 /* Should be the same as DSR_OPT_HDR_LEN, but that
* is not the case in ns-2 */
#define DSR_OPT_PAD1_LEN 1
#define DSR_PKT_MIN_LEN 24  /* IPv4 header + DSR header =  20 + 4 */

    /* Header types */
#define DSR_OPT_PADN       0
#define DSR_OPT_RREP       1
#define DSR_OPT_RREQ       2
#define DSR_OPT_RERR       3
#define DSR_OPT_PREV_HOP   5
#define DSR_OPT_ACK       32
#define DSR_OPT_SRT       96
#define DSR_OPT_TIMEOUT  128
#define DSR_OPT_FLOWID   129
#define DSR_OPT_ACK_REQ  160
#define DSR_OPT_PAD1     224

// From dsr-pkt.h
/* Flags: */
#define PKT_PROMISC_RECV 0x01
#define PKT_REQUEST_ACK  0x02
#define PKT_PASSIVE_ACK  0x04
#define PKT_XMIT_JITTER  0x08

/* Packet actions: */
#define DSR_PKT_NONE           1
#define DSR_PKT_SRT_REMOVE     (DSR_PKT_NONE << 2)
#define DSR_PKT_SEND_ICMP      (DSR_PKT_NONE << 3)
#define DSR_PKT_SEND_RREP      (DSR_PKT_NONE << 4)
#define DSR_PKT_SEND_BUFFERED  (DSR_PKT_NONE << 5)
#define DSR_PKT_SEND_ACK       (DSR_PKT_NONE << 6)
#define DSR_PKT_FORWARD        (DSR_PKT_NONE << 7)
#define DSR_PKT_FORWARD_RREQ   (DSR_PKT_NONE << 8)
#define DSR_PKT_DROP           (DSR_PKT_NONE << 9)
#define DSR_PKT_ERROR          (DSR_PKT_NONE << 10)
#define DSR_PKT_DELIVER        (DSR_PKT_NONE << 11)
#define DSR_PKT_ACTION_LAST    (12)

// from dsr-rerr
#define NODE_UNREACHABLE          1
#define FLOW_STATE_NOT_SUPPORTED  2
#define OPTION_NOT_SUPPORTED      3

// from dsr-rrep.h
#define DSR_RREP_OPT_LEN(srt) (DSR_RREP_HDR_LEN + (srt->addrs.size()*DSR_ADDRESS_SIZE) + DSR_ADDRESS_SIZE)
/* Length of source route is length of option, minus reserved/flags field minus
 * the last source route hop (which is the destination) */
#define DSR_RREP_ADDRS_LEN(rrep_opt) ((dynamic_cast<dsr_rrep_opt*>(rrep_opt)->addrs.size()-1)*DSR_ADDRESS_SIZE)

// from dsr-rreq.h
#define DSR_RREQ_OPT_LEN (DSR_RREQ_HDR_LEN - 2)
#define DSR_RREQ_TOT_LEN IP_HDR_LEN + DSR_OPT_HDR_LEN + DSR_RREQ_HDR_LEN
#define DSR_RREQ_ADDRS_LEN(rreq_opt) (rreq_opt->length - 6)

// from dsr-srt.h
#define SRT_FIRST_HOP_EXT 0x1
#define SRT_LAST_HOP_EXT  0x2

#define DSR_SRT_OPT_LEN(srt) (DSR_SRT_HDR_LEN + (srt->addrs.size()*DSR_ADDRESS_SIZE))

/* Flags */
#define SRT_BIDIR 0x1

// From dsr.h
#define DSR_BROADCAST ((unsigned int) 0xffffffff)


#define IPPROTO_DSR IP_PROT_DSR

#define IP_HDR_LEN 20
#define DSR_OPTS_MAX_SIZE 50    /* This is used to reduce the MTU of the DSR *
 * device so that packets are not too big after
 * adding the DSR header. A better solution
 * should probably be found... */

enum confval
{
#ifdef DEBUG
    PrintDebug,
#endif
    FlushLinkCache,
    PromiscOperation,
    BroadcastJitter,
    RouteCacheTimeout,
    SendBufferTimeout,
    SendBufferSize,
    RequestTableSize,
    RequestTableIds,
    MaxRequestRexmt,
    MaxRequestPeriod,
    RequestPeriod,
    NonpropRequestTimeout,
    RexmtBufferSize,
    MaintHoldoffTime,
    MaxMaintRexmt,
    UseNetworkLayerAck,
    TryPassiveAcks,
    PassiveAckTimeout,
    GratReplyHoldOff,
    MAX_SALVAGE_COUNT,
    RREQMulVisit,
    RREQMaxVisit,
    RREPDestinationOnly,
    PathCache,
    RetryPacket,
    CONFVAL_MAX,
};

enum confval_type
{
    SECONDS,
    MILLISECONDS,
    MICROSECONDS,
    NANOSECONDS,
    QUANTA,
    BINARY,
    COMMAND,
    CONFVAL_TYPE_MAX,
};

#define MAINT_BUF_MAX_LEN 100
#define RREQ_TBL_MAX_LEN 64 /* Should be enough */
#define SEND_BUF_MAX_LEN 100
#define RREQ_TLB_MAX_ID 16

static struct
{
    const char *name;
    const unsigned int val;
    enum confval_type type;
} confvals_def[CONFVAL_MAX] =
{
#ifdef DEBUG
    { "PrintDebug", 0, BINARY},
#endif
    { "FlushLinkCache", 1, COMMAND}
    ,{ "PromiscOperation", 1, BINARY}
    ,{ "BroadcastJitter", 20, MILLISECONDS}
    ,{ "RouteCacheTimeout", 300, SECONDS}
    ,{ "SendBufferTimeout", 30, SECONDS}
    ,{ "SendBufferSize", SEND_BUF_MAX_LEN, QUANTA}
    ,{ "RequestTableSize", RREQ_TBL_MAX_LEN, QUANTA}
    ,{ "RequestTableIds", RREQ_TLB_MAX_ID, QUANTA}
    ,{ "MaxRequestRexmt", 16, QUANTA}
    ,{ "MaxRequestPeriod", 10, SECONDS}
    ,{ "RequestPeriod", 500, MILLISECONDS}
    ,{ "NonpropRequestTimeout", 30, MILLISECONDS}
    ,{ "RexmtBufferSize", MAINT_BUF_MAX_LEN}
    ,{ "MaintHoldoffTime", 250, MILLISECONDS}
    ,{ "MaxMaintRexmt", 2, QUANTA}
    ,{ "UseNetworkLayerAck", 1, BINARY}
    ,{ "TryPassiveAcks", 1, QUANTA}
    ,{ "PassiveAckTimeout", 100, MILLISECONDS}
    ,{ "GratReplyHoldOff", 1, SECONDS}
    ,{ "MAX_SALVAGE_COUNT", 15, QUANTA}
    ,{"RREQMulVisit",0,BINARY}
    ,{"RREQMaxVisit",1,QUANTA}
    ,{"RREPDestinationOnly",0,BINARY}
#ifdef OMNETPP
    ,{"PathCache", 0, BINARY}
    ,{"RetryPacket", 0, BINARY}
#endif
};

struct dsr_node
{
    struct in_addr ifaddr;
    struct in_addr bcaddr;
    unsigned int confvals[CONFVAL_MAX];
};

#define DSR_SPIN_LOCK(l)
#define DSR_SPIN_UNLOCK(l)

#define ETH_ALEN 6
#define NSCLASS DSRUU::

class DSRUU;

#define ConfVal(name) DSRUU::get_confval(name)
#define ConfValToUsecs(cv) confval_to_usecs(cv)

} // namespace inetmanet

} // namespace inet
#include "dsr-uu/timer.h"

namespace inet{

namespace inetmanet {


static inline char *print_ip(struct in_addr addr)
{
    static char buf[16 * 4];
    Ipv4Address add = addr.s_addr.toIpv4();
    strcpy(buf,add.str().c_str());
    return buf;
}

static inline char *print_eth(char *addr)
{
    static char buf[30];

    sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x",
            (unsigned char)addr[0], (unsigned char)addr[1],
            (unsigned char)addr[2], (unsigned char)addr[3],
            (unsigned char)addr[4], (unsigned char)addr[5]);

    return buf;
}

static inline char *print_pkt(char *p, int len)
{
    static char buf[3000];
    int i, l = 0;

    for (i = 0; i < len; i++)
        l += sprintf(buf + l, "%02X", (unsigned char)p[i]);

    return buf;
}


#define SEND_BUF_DROP 1
#define SEND_BUF_SEND 2


#define init_timer(timer)
#define timer_pending(timer) ((timer)->pending())

#define del_timer_sync(timer) del_timer(timer)
//#define MALLOC(s, p) malloc(s)
//#define FREE(p) free(p)
#define XMIT(pkt) omnet_xmit(pkt)
#define DELIVER(pkt) omnet_deliver(pkt)
#define __init
#define __exit
#define ntohl(x) x
#define htonl(x) x
#define htons(x) x
#define ntohs(x) x

#define IPDEFTTL 64


#define  grat_rrep_tbl_timer (*grat_rrep_tbl_timer_ptr)
#define  send_buf_timer  (*send_buf_timer_ptr)
#define  neigh_tbl_timer  (*neigh_tbl_timer_ptr)
#define  lc_timer  (*lc_timer_ptr)
#define  ack_timer  (*ack_timer_ptr)
#define  etx_timer  (*etx_timer_ptr)

#define dsr_rtc_find(s,d) RouteFind(s,d)
#define dsr_rtc_add(srt,t,f) RouteAdd(srt,t,f)


class DSRUU:public ManetRoutingBase
{
private:
    // from headers

    int dsr_ack_add_ack_req(struct in_addr neigh);
    struct dsr_ack_req_opt *dsr_ack_req_opt_add(struct dsr_pkt *dp,
            unsigned short id);

    int dsr_ack_req_opt_recv(struct dsr_pkt *dp, struct dsr_ack_req_opt *areq);
    int dsr_ack_opt_recv(struct dsr_ack_opt *ack);
    int dsr_ack_req_send(struct in_addr neigh_addr, unsigned short id);
    int dsr_ack_send(struct in_addr neigh_addr, unsigned short id);
    int dsr_ack_req_send(struct dsr_pkt *dp);

    int dsr_recv(struct dsr_pkt *dp);
    void dsr_start_xmit(struct dsr_pkt *dp);

    struct dsr_opt_hdr *dsr_opt_hdr_add(struct dsr_opt_hdr *opt_hdr, unsigned int len, unsigned int protocol);
    struct dsr_opt *dsr_opt_find_opt(struct dsr_pkt *dp, int type);
    static int dsr_opt_parse(struct dsr_pkt *dp);
    int dsr_opt_recv(struct dsr_pkt *dp);
    int dsr_opt_remove(struct dsr_pkt *dp);

    friend struct dsr_pkt;


    struct dsr_pkt *dsr_pkt_alloc(Packet *p);
    struct dsr_pkt * dsr_pkt_alloc2(Packet  * p, cObject *ctrl);


    struct dsr_opt_hdr * dsr_pkt_alloc_opts(struct dsr_pkt *dp);
    //struct dsr_opt_hdr * dsr_pkt_alloc_opts_expand(struct dsr_pkt *dp, int len);
    void dsr_pkt_free(struct dsr_pkt *dp);
    void dsr_pkt_free_opts(struct dsr_pkt *dp);

    int dsr_rerr_send(struct dsr_pkt *dp_trigg, struct in_addr unr_addr);
    int dsr_rerr_opt_recv(struct dsr_pkt *dp, struct dsr_rerr_opt *dsr_rerr_opt);

    int dsr_rrep_opt_recv(struct dsr_pkt *dp, struct dsr_rrep_opt *rrep_opt);
    int dsr_rrep_send(struct dsr_srt *srt, struct dsr_srt *srt_to_me);

    void grat_rrep_tbl_timeout(void *data);
    int grat_rrep_tbl_add(struct in_addr src, struct in_addr prev_hop);
    int grat_rrep_tbl_find(struct in_addr src, struct in_addr prev_hop);
    int grat_rrep_tbl_init(void);
    void grat_rrep_tbl_cleanup(void);


    void rreq_tbl_set_max_len(unsigned int max_len);
    int dsr_rreq_opt_recv(struct dsr_pkt *dp, struct dsr_rreq_opt *rreq_opt);
    int rreq_tbl_route_discovery_cancel(struct in_addr dst);
    int dsr_rreq_route_discovery(struct in_addr target);
    int dsr_rreq_send(struct in_addr target, int ttl);
    void rreq_tbl_timeout(void *data);
    int dsr_rreq_duplicate(struct in_addr initiator, struct in_addr target,
                                   unsigned int id,double cost,unsigned int length, VectorAddress &addrs);

    int rreq_tbl_add_id(struct in_addr initiator, struct in_addr target,
                    unsigned short id,double cost,const VectorAddress &addr,int length);

    int rreq_tbl_init(void);
    void rreq_tbl_cleanup(void);
    void rreq_timer_test(cMessage *);


    static inline char *print_srt(struct dsr_srt *srt)
    {
    #define BUFLEN 256
        static char buf[BUFLEN];
        unsigned int len;

        if (!srt)
            return nullptr;

        len = sprintf(buf, "%s<->", print_ip(srt->src));

        for (unsigned int i = 0; i < (srt->addrs.size()) &&
                (len + 16) < BUFLEN; i++)
            len += sprintf(buf + len, "%s<->", print_ip(srt->addrs[i]));

        if ((len + 16) < BUFLEN)
            len = sprintf(buf + len, "%s", print_ip(srt->dst));
        return buf;
    }
    struct in_addr dsr_srt_next_hop(struct dsr_srt *srt, int sleft);
    struct in_addr dsr_srt_prev_hop(struct dsr_srt *srt, int sleft);
    struct dsr_srt_opt *dsr_srt_opt_add(struct dsr_opt_hdr *opt_hdr, int len, int flags, int salvage, struct dsr_srt *srt);
    struct dsr_srt_opt *dsr_srt_opt_add_char(char *buffer, int len, int flags, int salvage, struct dsr_srt *srt);

    struct dsr_srt *dsr_srt_new(struct in_addr,struct in_addr,unsigned int, const std::vector<L3Address>&,const std::vector<EtxCost> & = std::vector<EtxCost>());
    void dsr_srt_split_both(struct dsr_srt *,struct in_addr,struct in_addr,struct dsr_srt **,struct dsr_srt **);


    struct dsr_srt *dsr_srt_new_rev(struct dsr_srt *srt);
    void dsr_srt_del(struct dsr_srt *srt);
    int dsr_srt_add(struct dsr_pkt *dp);
    int dsr_srt_opt_recv(struct dsr_pkt *dp, struct dsr_srt_opt *srt_opt);
    struct dsr_srt *dsr_srt_concatenate(struct dsr_srt *srt1, struct dsr_srt *srt2); int dsr_srt_check_duplicate(struct dsr_srt *srt);
    struct dsr_srt *dsr_srt_new_split(struct dsr_srt *srt, struct in_addr addr);

    struct dsr_srt * dsr_srt_new_split_rev(struct dsr_srt *srt, struct in_addr addr);
    struct dsr_srt * dsr_srt_shortcut(struct dsr_srt *srt, struct in_addr a1,
                                     struct in_addr a2);


    struct neighbor_info
    {
        struct sockaddr hw_addr;
        unsigned short id;
        usecs_t rtt, rto;       /* RTT and Round Trip Timeout */
        struct timeval last_ack_req;
    };


    int neigh_tbl_add(struct in_addr neigh_addr, struct ethhdr *ethh);

    int neigh_tbl_del(struct in_addr neigh_addr);
    int neigh_tbl_query(struct in_addr neigh_addr,
                        struct neighbor_info *neigh_info);
    int neigh_tbl_id_inc(struct in_addr neigh_addr);
    int neigh_tbl_set_rto(struct in_addr neigh_addr, struct neighbor_info *neigh_info);
    int neigh_tbl_set_ack_req_time(struct in_addr neigh_addr);
    void neigh_tbl_garbage_timeout(unsigned long data);

    int neigh_tbl_init(void);
    void neigh_tbl_cleanup(void);


    private:
        DsrDataBase pathCacheMap;
        simtime_t nextPurge;
        void ph_srt_add_map(struct dsr_srt *srt, usecs_t timeout, unsigned short flags,bool = false);
        void ph_srt_add_node_map(struct in_addr node, usecs_t timeout, unsigned short flags,unsigned int cost);
        void ph_srt_delete_node_map(struct in_addr src);
        struct dsr_srt * ph_srt_find_map(struct in_addr src, struct in_addr dst, unsigned int timeout);
        void ph_srt_delete_link_map(struct in_addr src1, struct in_addr src2);
        void ph_srt_add_link_map(struct dsr_srt *srt, usecs_t timeout);
        void ph_add_link_map(struct in_addr src, struct in_addr dst, usecs_t timeout, int status, int cost);
        struct dsr_srt *ph_srt_find_link_route_map(struct in_addr src, struct in_addr dst, unsigned int timeout);

// Buffer storate
    public:
        struct PacketStoreage {
                simtime_t time;
                struct dsr_pkt *packet;
        };
    private:
        unsigned int buffMaxlen;
        typedef std::multimap<L3Address, PacketStoreage> PacketBuffer;
        PacketBuffer packetBuffer;

        void send_buf_set_max_len(unsigned int max_len);
        int send_buf_find(struct in_addr dst);
        int send_buf_enqueue_packet(struct dsr_pkt *dp);
        int send_buf_set_verdict(int verdict, struct in_addr dst);
        int send_buf_init(void);
        void send_buf_cleanup(void);
        void send_buf_timeout(void *data);
/////////////////
        // maintenance routines
////////////////

        unsigned int MaxMaintBuff;
        struct maint_entry
        {
            struct in_addr nxt_hop;
            unsigned int rexmt;
            unsigned short id;
            simtime_t tx_time;
            simtime_t expires;
            usecs_t rto;
            int ack_req_sent;
            struct dsr_pkt *dp;
        };

        typedef std::multimap<simtime_t,maint_entry *> MaintBuf;
        MaintBuf maint_buf;


        maint_entry *maint_entry_create(struct dsr_pkt *dp, unsigned short id, unsigned long rto);

        int maint_buf_init(void);
        void maint_buf_cleanup(void);

        void maint_buf_set_max_len(unsigned int max_len);
        int maint_buf_add(struct dsr_pkt *dp);
        int maint_buf_del_all(struct in_addr nxt_hop);
        int maint_buf_del_all_id(struct in_addr nxt_hop, unsigned short id);
        int maint_buf_del_addr(struct in_addr nxt_hop);
        void maint_buf_set_timeout(void);
        void maint_buf_timeout(void *data);
        int maint_buf_salvage(struct dsr_pkt *dp);
        void maint_insert(struct maint_entry *m);

        struct Id_Entry_Route
        {
            double cost;
            unsigned int length;
            VectorAddress add;
            Id_Entry_Route()
            {
                cost= 0;
                length = 0;
                add.clear();
            }
        };

        struct Id_Entry
        {
            struct in_addr trg_addr;
            unsigned short id;
            std::deque<Id_Entry_Route*> rreq_id_tbl_routes;
            Id_Entry()
            {
                trg_addr.s_addr.reset();
                rreq_id_tbl_routes.clear();
                id = 0;
            }
            ~Id_Entry()
            {
                while (!rreq_id_tbl_routes.empty())
                {
                    delete rreq_id_tbl_routes.back();
                    rreq_id_tbl_routes.pop_back();
                }
            }
        };

        struct rreq_tbl_entry
        {
            int state;
            struct in_addr node_addr;
            int ttl;
            DSRUUTimer *timer;
            struct timeval tx_time;
            struct timeval last_used;
            usecs_t timeout;
            unsigned int num_rexmts;
            std::deque<Id_Entry*> rreq_id_tbl;
            rreq_tbl_entry()
            {
                state = 0;
                node_addr.s_addr.reset();
                ttl = 0;
                timer = nullptr;
                tx_time.tv_sec = tx_time.tv_usec = 0;
                last_used = tx_time;
                timeout = 0;
                num_rexmts = 0;
                rreq_id_tbl.clear();
            }
            ~rreq_tbl_entry()
            {
                while (!rreq_id_tbl.empty())
                {
                    delete rreq_id_tbl.back();
                    rreq_id_tbl.pop_back();
                }
                if (timer)
                    delete timer;
            }
        };

        struct RreqSeqInfo
        {
             unsigned int seq;
             simtime_t time;
             std::vector<VectorAddress> paths;
        };

        typedef std::deque<RreqSeqInfo> RreqSeqInfoVector;
        typedef std::map<L3Address,RreqSeqInfoVector> RreqInfoMap;
        RreqInfoMap rreqInfoMap;

        typedef std::map<L3Address,rreq_tbl_entry*> DsrRreqTbl;
        DsrRreqTbl dsrRreqTbl;
        rreq_tbl_entry *__rreq_tbl_entry_create(struct in_addr node_addr);
        rreq_tbl_entry *__rreq_tbl_add(struct in_addr node_addr);

        // neighbor routinges
        struct neighbor
        {
            struct in_addr addr;
            struct sockaddr hw_addr;
            unsigned short id;
            struct timeval last_ack_req;
            usecs_t t_srtt, rto, t_rxtcur, t_rttmin, t_rttvar, jitter;  /* RTT in usec */
        };


        typedef std::map<L3Address,neighbor*> NeighborMap;
        NeighborMap neighborMap;
        neighbor * neigh_tbl_create(struct in_addr addr,struct sockaddr *hw_addr, unsigned short id);

        struct grat_rrep_entry
        {
            struct in_addr src, prev_hop;
            struct timeval expires;
        };
        std::deque<grat_rrep_entry*> gratRrep;

  public:
    friend class DSRUUTimer;
    //static simtime_t current_time;
    static struct dsr_pkt *lifoDsrPkt;
    static int lifo_token;

    bool nodeActive;
    //virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

    INetfilter::IHook::Result processPacket(Packet *,unsigned int);

    INetfilter::IHook::Result ensureRouteForDatagram(Packet *datagram) override;
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  private:
    bool is_init;
    struct in_addr myaddr_;
    MacAddress macaddr_;
    int interfaceId;

    // Trace *trace_;
    // Mac *mac_;
    // LL *ll_;
    // CMUPriQueue *ifq_;
    // MobileNode *node_;

   // struct tbl rreq_tbl;

    unsigned int rreq_seqno;

    DSRUUTimer *grat_rrep_tbl_timer_ptr;
    DSRUUTimer *send_buf_timer_ptr;
    DSRUUTimer *neigh_tbl_timer_ptr;
    DSRUUTimer *lc_timer_ptr;
    DSRUUTimer *ack_timer_ptr;
    // ETX Parameters
    bool etxActive;
    DSRUUTimer *etx_timer_ptr;
    int etxSize;
    double etxTime;
    double etxWindow;
    double etxWindowSize;
    double etxJitter;
    struct ETXEntry;
    int etxNumRetry;

    typedef std::map<Ipv4Address, ETXEntry*> ETXNeighborTable;
    struct ETXEntry
    {
        double deliveryDirect;
        double deliveryReverse;
        std::vector<simtime_t> timeVector;
        //IPv4Address address;
    };
// In dsr-uu-omnet.cc used for ETX
    ETXNeighborTable etxNeighborTable;
    void EtxMsgSend(void *data);
    void EtxMsgProc(Packet *msg);
    double getCost(Ipv4Address add);
    void AddCost(struct dsr_pkt *,struct dsr_srt *);
    void AddCostRrep(struct dsr_pkt *dp, struct dsr_srt *srt);
    void ActualizeMyCostRrep(dsr_rrep_opt *);
    void ExpandCost(struct dsr_pkt *);
    double PathCost(struct dsr_pkt *dp);

    void linkFailed(Ipv4Address);
//************++

    struct in_addr my_addr()
    {
        return myaddr_;
    }

    void drop (cMessage *msg,int code) { delete msg;}

    // virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual void processLinkBreak(const Packet* details) override;

  protected:
    struct in_addr ifaddr;
    struct in_addr bcaddr;
    static unsigned int confvals[CONFVAL_MAX];
    void tap(Packet * p);
    void omnet_xmit(struct dsr_pkt *dp);
    void omnet_deliver(struct dsr_pkt *dp);
    void packetFailed(const Packet *ipDgram);
    void packetLinkAck(Packet *ipDgram);
    void handleTimer(cMessage *);
    void defaultProcess(Packet *);

    struct dsr_srt *RouteFind(struct in_addr , struct in_addr);
    int RouteAdd(struct dsr_srt *, unsigned long, unsigned short );

    bool proccesICMP(cMessage *msg);

    Packet *newDsrPacket(struct dsr_pkt *dp, int interface_id, bool = true);
    Packet *newDsrPacket(struct dsr_pkt *dp, int interface_id, const Packet *);


  public:
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override {throw cRuntimeError("DSR doesn't use Udp sockets");}
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override {throw cRuntimeError("DSR doesn't use Udp sockets");}

    virtual uint32_t getRoute(const L3Address& dest, std::vector<L3Address>& hopsList) override {throw cRuntimeError("DSR doesn't implement this method");return 0;}

    virtual bool getNextHop(const L3Address& dest, L3Address& nextHop, int& ifaceId, double& cost) override {throw cRuntimeError("DSR doesn't implement this method");return false;}
    virtual void setRefreshRoute(const L3Address& dest, const L3Address& nextHop, bool isReverse) override {throw cRuntimeError("DSR doesn't implement this method");}
    virtual bool isProactive() override {throw cRuntimeError("DSR doesn't implement this method"); return false;}
    virtual bool supportGetRoute () override {throw cRuntimeError("DSR doesn't implement this method"); return false;}
    virtual bool isOurType(const Packet *pk) override {throw cRuntimeError("DSR doesn't implement this method"); return false;}
    virtual bool getDestAddress(Packet *pk, L3Address &dest) override {throw cRuntimeError("DSR doesn't implement this method"); return false;}

    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;

    DSRUU();
    ~DSRUU();

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    struct iphdr * dsr_build_ip(struct dsr_pkt *dp, struct in_addr src,
                                struct in_addr dst, int ip_len, int tot_len,
                                int protocol, int ttl);

    void add_timer(DSRUUTimer * t)
    {
        /* printf("Setting timer %s to %f\n", t->get_name(), t->expires - Scheduler::getInstance().clock()); */
        if (t->getExpires() - simTime() < 0)
            t->resched(0);
        else
            t->resched( SIMTIME_DBL(t->getExpires() -simTime()));
    }
    /*      void mod_timer (DSRUUTimer *t, unsinged long expires_)  *//*            { t->expires = expires_ ; t->resched(t->expires); } */
    void del_timer(DSRUUTimer * t)
    {
        //printf("Cancelling timer %s\n", t->get_name());
        t->cancel();
    }
    void set_timer(DSRUUTimer * t, struct timeval *expires)
    {
        //printf("In set_timer\n");
        double exp;
        exp = expires->tv_usec;
        exp /= 1000000l;
        exp += expires->tv_sec;
        t->setExpires( exp);
        /*  printf("Set timer %s to %f\n", t->get_name(), t->expires - Scheduler::getInstance().clock()); */
        add_timer(t);
    }

    void set_timer(DSRUUTimer * t, double expire)
    {
        //printf("In set_timer\n");
        t->setExpires( expire);
        /*  printf("Set timer %s to %f\n", t->get_name(), t->expires - Scheduler::getInstance().clock()); */
        add_timer(t);
    }


    static const unsigned int get_confval(enum confval cv)
    {
        return confvals[cv];
    }
    static const unsigned int set_confval(enum confval cv, unsigned int val)
    {
        confvals[cv] = val;
        return val;
    }
};



static inline usecs_t confval_to_usecs(enum confval cv)
{
    usecs_t usecs = 0;
    unsigned int val;

    val = ConfVal(cv);

    switch (confvals_def[cv].type)
    {
    case SECONDS:
        usecs = val * 1000000;
        break;
    case MILLISECONDS:
        usecs = val * 1000;
        break;
    case MICROSECONDS:
        usecs = val;
        break;
    case NANOSECONDS:
        usecs = val / 1000;
        break;
    case BINARY:
    case QUANTA:
    case COMMAND:
    case CONFVAL_TYPE_MAX:
        break;
    }

    return usecs;
}




static inline int omnet_vprintk(const char *fmt, va_list args)
{
    int printed_len, prefix_len=0;
    static char printk_buf[1024];


    /* Emit the output into the temporary buffer */
//#ifdef _WIN32
#if 0
    printed_len = _vsnprintf_s(printk_buf + prefix_len,
                               sizeof(printk_buf) - prefix_len,sizeof(printk_buf) - prefix_len, fmt, args);
#else
    printed_len = vsnprintf(printk_buf + prefix_len,
                            sizeof(printk_buf) - prefix_len, fmt, args);
#endif
    EV_DEBUG << printk_buf;
    return printed_len;
}



static inline void omnet_debug(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    omnet_vprintk(fmt, args);
    va_end(args);
}

static inline void gettime(struct timeval *tv)
{
    double now, usecs;

    /* Timeval is required, timezone is ignored */
    if (!tv)
        return;
    now = SIMTIME_DBL(simTime());
    tv->tv_sec = (long)now; /* Removes decimal part */
    usecs = (now - tv->tv_sec) * 1000000;
    tv->tv_usec = (long)(usecs+0.5);
    if (tv->tv_usec>1000000)
    {
        tv->tv_usec -=1000000;
        tv->tv_sec++;
    }
}


static inline void timevalFromSimTime(struct timeval *tv,simtime_t time)
{
    double now, usecs;

    /* Timeval is required, timezone is ignored */
    if (!tv)
        return;

    now = SIMTIME_DBL(time);
    tv->tv_sec = (long)now; /* Removes decimal part */
    usecs = (now - tv->tv_sec) * 1000000;
    tv->tv_usec = (long)(usecs+0.5);
    if (tv->tv_usec>1000000)
    {
        tv->tv_usec -=1000000;
        tv->tv_sec++;
    }
}

} // namespace inetmanet

} // namespace inet

#endif              /* _DSR_NS_AGENT_H */
