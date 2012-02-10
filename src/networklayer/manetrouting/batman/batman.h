/*
 * BatmanData.h
 *
 *  Created on: Nov 29, 2011
 *      Author: alfonso
 */

#ifndef __BATMAN_H_
#define __BATMAN_H_

#include "ManetRoutingBase.h"
#include "BatmanData.h"

class BatmanTimer :ManetTimer
{
public:
    virtual void expire();
};

class Batman : public ManetRoutingBase
{
private:
    typedef std::map<Uint128,OrigNode*>  OrigMap;
    typedef std::vector<BatmanIf*> Interfacelist;
    typedef std::vector<GwNode*> Gwlist;
    typedef std::vector<ForwNode*> Forwlist;

    typedef std::vector<Hna_local_entry> HnaLocalEntryList;
    typedef std::vector<Hna_task*> HnaTaskList;

//    hna_global_hash = hash_new(128, compare_hna, choose_hna);


protected:
    uint8_t debug_level;
    uint8_t debug_level_max;
    uint8_t gateway_class;
    uint8_t routing_class;
    simtime_t originator_interval;
    simtime_t debug_timeout;
    Uint128 pref_gateway;

    int nat_tool_avail;
    int8_t disable_client_nat;

    GwNode *curr_gateway;

    uint8_t found_ifs;
    uint8_t active_ifs;
    int32_t receive_max_sock;

    uint8_t unix_client;
    uint8_t log_facility_active;

    OrigMap origMap;
    int numOrig;

    Interfacelist if_list;
    Gwlist gw_list;
    Forwlist forw_list;
    // struct vis_if vis_if;


    uint8_t tunnel_running;

    uint8_t hop_penalty;
    simtime_t purge_timeout;
    uint8_t minimum_send;
    uint8_t minimum_recv;
    uint8_t global_win_size;
    uint8_t local_win_size;
    uint8_t num_words;
    uint8_t aggregation_enabled;
    uint32_t MAX_AGGREGATION_BYTES;
    cMessage *timer;

    HnaLocalEntryList hna_list;
    HnaTaskList hna_chg_list;
    std::map<BatmanHnaMsg,Hna_global_entry*> hnaMap;
    std::vector<BatmanHnaMsg> hna_buff_local;
// methods

private:
    void sendPackets(const simtime_t &curr_time);
    OrigNode  * getOrigNode(const Uint128 &addr);
    NeighNode * create_neighbor(OrigNode *orig_node, OrigNode *orig_neigh_node, const Uint128 &neigh, BatmanIf *if_incoming);
    OrigNode * get_orig_node(const Uint128 &addr );
    void update_orig(OrigNode *orig_node, BatmanPacket *in, const Uint128 &neigh, BatmanIf *if_incoming, BatmanHnaMsg *hna_recv_buff, int16_t hna_buff_len, uint8_t is_duplicate, const simtime_t &curr_time );
    void purge_orig(const simtime_t &curr_time);
    void choose_gw(void);
    void update_routes(OrigNode *orig_node, NeighNode *, BatmanHnaMsg * hna_recv_buff, int16_t hna_buff_len);
    void get_gw_speeds(unsigned char gw_class, int *down, int *up);
    void update_gw_list(OrigNode *orig_node, uint8_t new_gwflags, uint16_t gw_port);
    unsigned char get_gw_class(int down, int up);
    int isBidirectionalNeigh(OrigNode *orig_node, OrigNode *orig_neigh_node, BatmanPacket *in, const simtime_t &recv_time, BatmanIf *if_incoming);
    uint8_t count_real_packets(BatmanPacket *in, const Uint128 &neigh, BatmanIf *if_incoming);
    void schedule_own_packet(BatmanIf *batman_if);
    void schedule_forward_packet(OrigNode *orig_node, BatmanPacket *in, const Uint128 &neigh, uint8_t directlink, int16_t hna_buff_len, BatmanIf *if_incoming, const simtime_t &curr_time);
    void appendPacket(cPacket *oldPacket, cPacket * packetToAppend);
    void send_outstanding_packets(const simtime_t &);
    int8_t send_udp_packet(cPacket *, int32_t, const Uint128 &, int32_t send_sock, BatmanIf *batman_if);
    void hna_local_task_add_ip(const Uint128 &ip_addr, uint16_t netmask, uint8_t route_action);
    void hna_local_buffer_fill(void);
    void hna_local_task_exec(void);
    void hna_local_update_routes(Hna_local_entry *hna_local_entry, int8_t route_action);
    void _hna_global_add(OrigNode *orig_node, Hna_element *hna_element);
    void _hna_global_del(OrigNode *orig_node, Hna_element *hna_element);
    int hna_buff_delete(std::vector<Hna_element *> &buf, int *buf_len, Hna_element *e);
    void hna_global_add(OrigNode *orig_node, BatmanHnaMsg *new_hna, int16_t new_hna_len);
    void hna_global_update(OrigNode *orig_node, BatmanHnaMsg *new_hna, int16_t new_hna_len, NeighNode *);
    void hna_global_check_tq(OrigNode *orig_node);
    void hna_global_del(OrigNode *orig_node);
    void hna_free(void);

    BatmanIf * is_batman_if(InterfaceEntry * dev);
    void ring_buffer_set(std::vector<uint8_t> &tq_recv, uint8_t &tq_index, uint8_t value);
    uint8_t ring_buffer_avg(std::vector<uint8_t> &tq_recv);
// Routing table modification
    void add_del_route(const Uint128  &, uint8_t netmask, const Uint128  &, int32_t, InterfaceEntry *dev, uint8_t rt_table, int8_t route_type, int8_t route_action);
    void add_del_rule(uint32_t network, uint8_t netmask, int8_t rt_table, uint32_t prio, InterfaceEntry *dev, int8_t rule_type, int8_t rule_action);
    int add_del_interface_rules(int8_t rule_action);
//
// Default routes methods
//
    virtual void del_default_route(){}
    virtual void add_default_route(){}
    // Bits methods
    virtual void bit_init(std::vector<TYPE_OF_WORD> &);
    virtual uint8_t get_bit_status(std::vector<TYPE_OF_WORD> &seq_bits, uint16_t last_seqno, uint16_t curr_seqno );
    virtual void bit_mark(std::vector<TYPE_OF_WORD>  &seq_bits, int32_t);
    virtual void bit_shift(std::vector<TYPE_OF_WORD> &seq_bits, int32_t);
    virtual char bit_get_packet(std::vector<TYPE_OF_WORD> &seq_bits, int16_t seq_num_diff, int8_t set_mark );
    virtual int bit_packet_count( std::vector<TYPE_OF_WORD> &seq_bits );
    virtual uint8_t bit_count( int32_t to_count );

    virtual bool is_aborted(){return false;}
    virtual void deactivate_interface(BatmanIf *);
    virtual void activate_interface(BatmanIf *iface);
    virtual void check_active_inactive_interfaces(void);
    virtual void check_active_interfaces(void);
    virtual void check_inactive_interfaces(void);
    //build packets
    virtual BatmanPacket *buildDefaultBatmanPkt(const BatmanIf *);
    // get timer
    virtual simtime_t getTime();


protected:
    virtual void handleMessage(cMessage *msg);
    virtual int numInitStages() const  {return 5;}
    virtual void initialize(int stage);
    virtual void processLinkBreak(const cObject *details){};
    virtual void packetFailed(IPv4Datagram *dgram) {}
    virtual void scheduleNextEvent();

public:
    Batman();
    ~Batman();
    virtual uint32_t getRoute(const Uint128 &, std::vector<Uint128> &add);
    virtual bool getNextHop(const Uint128 &, Uint128 &add, int &iface, double &val);
    virtual void setRefreshRoute(const Uint128 &destination, const Uint128 & nextHop, bool isReverse) {};
    virtual bool isProactive() {return true;};
    virtual bool isOurType(cPacket * msg)
    {
        if (dynamic_cast<BatmanPacket*>(msg))
            return true;
        else if (dynamic_cast<visPacket*>(msg))
            return true;
        else
            return false;
    };
    virtual bool getDestAddress(cPacket *, Uint128 &) {return false;};
};


#endif /* BATMANDATA_H_ */
