/*
 * BatmanData.h
 *
 *  Created on: Nov 29, 2009
 *      Author: alfonso
 */

#ifndef __BATMANDATA_H__
#define __BATMANDATA_H__
#include <set>
#include <vector>
#include <map>

#include "inet/common/INETDefs.h"

#include "inet/routing/extras/base/ManetRoutingBase.h"
#include "inet/routing/extras/batman/BatmanMsg.h"

namespace inet {

namespace inetmanet {

#define TYPE_OF_WORD uint64_t /* you should choose something big, if you don't want to waste cpu */
#define WORD_BIT_SIZE  (sizeof(TYPE_OF_WORD)*8)


const_simtime_t PURGE_TIMEOUT = 200; // 200000 msec /* purge originators after time in ms if no valid packet comes in -> TODO: check influence on TQ_LOCAL_WINDOW_SIZE */
//#define BATMANMAXJITTER  0.1
//#define JITTER (uniform(0,(double)BATMANMAXJITTER))
#define TTL 32                 /* Time To Live of broadcast messages */
#define TQ_LOCAL_WINDOW_SIZE 64     /* sliding packet range of received originator messages in squence numbers (should be a multiple of our word size) */
#define TQ_GLOBAL_WINDOW_SIZE 10
#define TQ_LOCAL_BIDRECT_SEND_MINIMUM 1
#define TQ_LOCAL_BIDRECT_RECV_MINIMUM 1
#define TQ_TOTAL_BIDRECT_LIMIT 1
#define TQ_MAX_VALUE 255
#define GW_PORT 4306

/**
 * hop penalty is applied "twice"
 * when the packet comes in and if rebroadcasted via the same interface
 */
#define TQ_HOP_PENALTY 5
#define DEFAULT_ROUTING_CLASS 30


//#define MAX_AGGREGATION_BYTES 512 /* should not be bigger than 512 bytes or change the size of forw_node->direct_link_flags */
//#define MAX_AGGREGATION_MS 0.1 // 100

#define ROUTE_TYPE_UNICAST          0
#define ROUTE_TYPE_THROW            1
#define ROUTE_TYPE_UNREACHABLE      2
#define ROUTE_TYPE_UNKNOWN          3
#define ROUTE_ADD                   0
#define ROUTE_DEL                   1

#define RULE_TYPE_SRC              0
#define RULE_TYPE_DST              1
#define RULE_TYPE_IIF              2
#define RULE_ADD                   0
#define RULE_DEL                   1



#define BATMAN_RT_TABLE_NETWORKS 65
#define BATMAN_RT_TABLE_HOSTS 66
#define BATMAN_RT_TABLE_UNREACH 67
#define BATMAN_RT_TABLE_TUNNEL 68

#define BATMAN_RT_PRIO_DEFAULT 6600
#define BATMAN_RT_PRIO_UNREACH BATMAN_RT_PRIO_DEFAULT + 100
#define BATMAN_RT_PRIO_TUNNEL BATMAN_RT_PRIO_UNREACH + 100


#define SIZE_Hna_element            BATMAN_HNA_MSG_SIZE
#define ADDR_STR_LEN 16
#define BATMAN_PORT 254
#define MAX_HOPS 0xFF

class BatmanIf
{
  public:
    InterfaceEntry* dev;
    int32_t udp_send_sock;
    int32_t udp_recv_sock;
    int32_t udp_tunnel_sock;
    uint8_t if_num;
    uint8_t if_active;
    int32_t if_index;
    int8_t if_rp_filter_old;
    int8_t if_send_redirects_old;
    // pthread_t listen_thread_id;
    //struct sockaddr_in addr;
    //struct sockaddr_in broad;
    //uint32_t netaddr;
    //uint8_t netmask;
    //uint8_t wifi_if;
    uint16_t seqno;
    bool wifi_if;
    L3Address address;
    L3Address broad;
};


class NeighNode;

class OrigNode : public cObject
{
  public:
    L3Address orig;
    uint32_t totalRec;
    NeighNode *router;
    BatmanIf* batmanIf;
    std::vector<TYPE_OF_WORD> bcast_own;
    std::vector<uint8_t> bcast_own_sum;
    uint8_t tq_own;
    int tq_asym_penalty;
    simtime_t last_valid;        /* when last packet from this node was received */
    uint8_t  gwflags;      /* flags related to gateway functions: gateway class */
    std::vector<HnaElement> hna_buff;
    uint16_t last_real_seqno;   /* last and best known squence number */
    uint8_t last_ttl;         /* ttl of last received packet */
    uint8_t num_hops;
    std::vector<NeighNode *> neigh_list;

  public:
    void clear();
    OrigNode();
    ~OrigNode();
    virtual std::string info() const;
};


class NeighNode : public cObject
{
  public:
    L3Address addr;
    uint8_t real_packet_count;
    std::vector <uint8_t> tq_recv;
    uint8_t tq_index;
    uint8_t tq_avg;
    uint8_t last_ttl;
    simtime_t last_valid;            /* when last packet via this neighbour was received */
    uint8_t num_hops;
    std::vector<TYPE_OF_WORD> real_bits;
    OrigNode *orig_node;
    OrigNode *owner_node;
    BatmanIf *if_incoming;
    void clear();
    ~NeighNode();
    NeighNode() {clear();}
    NeighNode(OrigNode *, OrigNode *, const L3Address &, BatmanIf *, const uint32_t&, const uint32_t&);
    virtual std::string info() const;
};


class ForwNode                       /* structure for forw_list maintaining packets to be send/forwarded */
{
  public:
    simtime_t send_time;
    uint8_t   own;
    BatmanPacket *pack_buff;
    uint16_t  pack_buff_len;
    uint32_t direct_link_flags;
    short unsigned int num_packets;
    BatmanIf *if_incoming;
};

class GwNode
{
  public:
    OrigNode *orig_node;
    uint16_t gw_port;
    uint16_t gw_failure;
    simtime_t last_failure;
    simtime_t deleted;
  public:
    GwNode() : orig_node(NULL), gw_port(0), gw_failure(0), last_failure(0), deleted(0) {}
};

class GwClient
{
  public:
    L3Address wip_addr;
    L3Address vip_addr;
    uint16_t client_port;
    simtime_t last_keep_alive;
    uint8_t nat_warn;
};

class HnaTask
{
  public:
    L3Address addr;
    short unsigned int netmask;
    short unsigned int route_action;

  public:
    HnaTask() : netmask(0), route_action(0) {}
};

class HnaLocalEntry
{
  public:
    L3Address addr;
    short unsigned int netmask;
    int idIface;
};

class HnaGlobalEntry
{
  public:
    L3Address addr;
    short unsigned int netmask;
    OrigNode *curr_orig_node;
    std::vector<OrigNode *> orig_list;
    HnaGlobalEntry() { curr_orig_node = NULL; }
    ~HnaGlobalEntry() { orig_list.clear(); }

  private:     // noncopyable
    HnaGlobalEntry(const HnaGlobalEntry& m);
    HnaGlobalEntry& operator=(const HnaGlobalEntry& m);
  public:
    friend bool operator<(const HnaGlobalEntry&, const HnaGlobalEntry&);
    friend bool operator==(const HnaGlobalEntry&, const HnaGlobalEntry&);
};

inline bool operator<(const HnaGlobalEntry& a, const HnaGlobalEntry& b)
{
    return (a.addr < b.addr) || (a.addr == b.addr && a.netmask < b.netmask);
}

inline bool operator==(const HnaGlobalEntry& a, const HnaGlobalEntry& b)
{
    return (a.addr==b.addr && a.netmask==b.netmask);
}

} // namespace inetmanet

} // namespace inet

#endif /* __BATMANDATA_H__ */

