/***************************************************************************
 * file:        WiseRoute.h
 *
 * author:      Damien Piguet, Jerome Rousselot
 *
 * copyright:   (C) 2007-2009 CSEM SA, Neuchatel, Switzerland.
 *
 * description: Implementation of the routing protocol of WiseStack.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 *
 * Funding: This work was partially financed by the European Commission under the
 * Framework 6 IST Project "Wirelessly Accessible Sensor Populations"
 * (WASP) under contract IST-034963.
 ***************************************************************************
 * ported to Mixim 2.0.1 by Theodoros Kapourniotis
 * last modification: 06/02/11
 **************************************************************************/

#ifndef __INET_WISEROUTE_H
#define __INET_WISEROUTE_H

#include "inet/common/packet/Packet.h"
#include "inet/networklayer/base/NetworkProtocolBase.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/IArp.h"
#include "inet/networklayer/contract/INetworkProtocol.h"
#include "inet/networklayer/wiseroute/WiseRouteHeader_m.h"

namespace inet {

/**
 * @brief Wiseroute is a simple loop-free routing algorithm that
 * builds a routing tree from a central network point. It is especially
 * useful for wireless sensor networks and convergecast traffic,
 * hence its name (Wireless Sensors Routing).
 * The sink (the device at the center of the network) broadcasts
 * a route building message. Each network node that receives it
 * selects the sink as parent in the routing tree, and rebroadcasts
 * the route building message. This procedure maximizes the probability
 * that all network nodes can join the network, and avoids loops.
 *
 * @ingroup netwLayer
 * @author Jerome Rousselot
 **/
class INET_API WiseRoute : public NetworkProtocolBase, public INetworkProtocol
{
  private:
    /** @brief Copy constructor is not allowed.
     */
    WiseRoute(const WiseRoute&);
    /** @brief Assignment operator is not allowed.
     */
    WiseRoute& operator=(const WiseRoute&);

  public:
    WiseRoute() {}
    /** @brief Initialization of the module and some variables*/
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    virtual void initialize(int) override;

    virtual void finish() override;

    virtual ~WiseRoute();

    const Protocol& getProtocol() const override { return Protocol::wiseRoute; }

    // OperationalBase:
    virtual void handleStartOperation(LifecycleOperation *operation) override {}    //TODO implementation
    virtual void handleStopOperation(LifecycleOperation *operation) override {}    //TODO implementation
    virtual void handleCrashOperation(LifecycleOperation *operation) override {}    //TODO implementation

  protected:
    enum messagesTypes {
        DATA,
        ROUTE_FLOOD,
        SEND_ROUTE_FLOOD_TIMER
    };

    typedef enum floodTypes {
        NOTAFLOOD,
        FORWARD,
        FORME,
        DUPLICATE
    } floodTypes;

    typedef struct tRouteTableEntry
    {
        L3Address nextHop;
        double rssi;
    } tRouteTableEntry;

    typedef std::map<L3Address, tRouteTableEntry> tRouteTable;
    typedef std::multimap<tRouteTable::key_type, unsigned long> tFloodTable;

    tRouteTable routeTable;
    tFloodTable floodTable;

    IArp *arp = nullptr;

    /**
     * @brief Length of the NetwPkt header
     * Read from omnetpp.ini
     **/
    int headerLength = 0;

    L3Address myNetwAddr;
    L3Address sinkAddress;

    /** @brief Minimal received RSSI necessary for adding source to routing table. */
    double rssiThreshold = 0.0;

    /** @brief Interval [seconds] between two route floods. A route flood is a simple flood from
     *         which other nodes can extract routing (next hop) information.
     */
    double routeFloodsInterval = 0.0;

    /** @brief Flood sequence number */
    unsigned long floodSeqNumber = 0;

    cMessage *routeFloodTimer = nullptr;

    long nbDataPacketsForwarded = 0;
    long nbDataPacketsReceived = 0;
    long nbDataPacketsSent = 0;
    long nbDuplicatedFloodsReceived = 0;
    long nbFloodsSent = 0;
    long nbPureUnicastSent = 0;
    long nbRouteFloodsSent = 0;
    long nbRouteFloodsReceived = 0;
    long nbUnicastFloodForwarded = 0;
    long nbPureUnicastForwarded = 0;
    long nbGetRouteFailures = 0;
    long nbRoutesRecorded = 0;
    long nbHops = 0;

    cOutVector receivedRSSI;
    cOutVector routeRSSI;
    cOutVector allReceivedRSSI;
    cOutVector allReceivedBER;
    cOutVector routeBER;
    cOutVector receivedBER;
    cOutVector nextHopSelectionForSink;

    bool trace = false;

    /**
     * @name Handle Messages
     * @brief Functions to redefine by the programmer
     *
     * These are the functions provided to add own functionality to your
     * modules. These functions are called whenever a self message or a
     * data message from the upper or lower layer arrives respectively.
     *
     **/
    /*@{*/

    /** @brief Handle messages from upper layer */
    virtual void handleUpperPacket(Packet *packet) override;

    /** @brief Handle messages from lower layer */
    virtual void handleLowerPacket(Packet *packet) override;

    /** @brief Handle self messages */
    virtual void handleSelfMessage(cMessage *msg) override;

    /** @brief Update routing table.
     *
     * The tuple provided in argument gives the next hop address to the origin.
     * The table is updated only if the RSSI value is above the threshold.
     */
    virtual void updateRouteTable(const tRouteTable::key_type& origin, const L3Address& lastHop, double rssi, double ber);

    /** @brief Decapsulate a message and delete original msg */
    void decapsulate(Packet *packet);

    /** @brief update flood table. returns detected flood type (general or unicast flood to forward,
     *         duplicate flood to delete, unicast flood to me
     */
    floodTypes updateFloodTable(bool isFlood, const tFloodTable::key_type& srcAddr, const tFloodTable::key_type& destAddr, unsigned long seqNum);

    /** @brief find a route to destination address. */
    tFloodTable::key_type getRoute(const tFloodTable::key_type& destAddr, bool iAmOrigin = false) const;

    /**
     * @brief Attaches a "control info" (NetwToMac) structure (object) to the message pMsg.
     *
     * This is most useful when passing packets between protocol layers
     * of a protocol stack, the control info will contain the destination MAC address.
     *
     * The "control info" object will be deleted when the message is deleted.
     * Only one "control info" structure can be attached (the second
     * setL3ToL2ControlInfo() call throws an error).
     *
     * @param pMsg      The message where the "control info" shall be attached.
     * @param pDestAddr The MAC address of the message receiver.
     */
    virtual void setDownControlInfo(Packet *const pMsg, const MacAddress& pDestAddr);
};

} // namespace inet

#endif // ifndef __INET_WISEROUTE_H

