//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GPSR_H
#define __INET_GPSR_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/packet/Packet.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/routing/gpsr/Gpsr_m.h"
#include "inet/routing/gpsr/PositionTable.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {

/**
 * This class implements the Greedy Perimeter Stateless Routing for Wireless Networks.
 * The implementation supports both GG and RNG planarization algorithms.
 *
 * For more information on the routing algorithm, see the GPSR paper
 * http://www.eecs.harvard.edu/~htk/publication/2000-mobi-karp-kung.pdf
 */
// TODO optimize internal data structures for performance to use less lookups and be more prepared for routing a packet
// TODO implement position piggybacking that is all packets should carry the position of the sender, all packets act as a beacon and reset beacon timer
// TODO implement promiscuous mode, all receivers should process all packets with respect to neighbor positions
// KLUDGE implement position registry protocol instead of using a global variable
class INET_API Gpsr : public RoutingProtocolBase, public cListener, public NetfilterBase::HookBase
{
  private:
    // GPSR parameters
    GpsrPlanarizationMode planarizationMode = static_cast<GpsrPlanarizationMode>(-1);
    const char *interfaces = nullptr;
    simtime_t beaconInterval;
    simtime_t maxJitter;
    simtime_t neighborValidityInterval;
    bool displayBubbles;

    // context
    cModule *host = nullptr;
    opp_component_ptr<IMobility> mobility;
    const IL3AddressType *addressType = nullptr;
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    const char *outputInterface = nullptr;
    ModuleRefByPar<IRoutingTable> routingTable; // TODO delete when necessary functions are moved to interface table
    ModuleRefByPar<INetfilter> networkProtocol;
    PositionTable& globalPositionTable = SIMULATION_SHARED_VARIABLE(globalPositionTable); // KLUDGE implement position registry protocol

    // packet size
    int positionByteLength = -1;

    // internal
    cMessage *beaconTimer = nullptr;
    cMessage *purgeNeighborsTimer = nullptr;
    PositionTable neighborPositionTable;

  public:
    Gpsr();
    virtual ~Gpsr();

  protected:
    // module interface
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override;
    void handleMessageWhenUp(cMessage *message) override;

  private:
    // handling messages
    void processSelfMessage(cMessage *message);
    void processMessage(cMessage *message);

    // handling beacon timers
    void scheduleBeaconTimer();
    void processBeaconTimer();

    // handling purge neighbors timers
    void schedulePurgeNeighborsTimer();
    void processPurgeNeighborsTimer();

    // handling UDP packets
    void sendUdpPacket(Packet *packet);
    void processUdpPacket(Packet *packet);

    // handling beacons
    const Ptr<GpsrBeacon> createBeacon();
    void sendBeacon(const Ptr<GpsrBeacon>& beacon);
    void processBeacon(Packet *packet);

    // handling packets
    GpsrOption *createGpsrOption(L3Address destination);
    int computeOptionLength(GpsrOption *gpsrOption);
    void setGpsrOptionOnNetworkDatagram(Packet *packet, const Ptr<const NetworkHeaderBase>& networkHeader, GpsrOption *gpsrOption);

    // returns nullptr if not found
    GpsrOption *findGpsrOptionInNetworkDatagramForUpdate(const Ptr<NetworkHeaderBase>& networkHeader);
    const GpsrOption *findGpsrOptionInNetworkDatagram(const Ptr<const NetworkHeaderBase>& networkHeader) const;

    // throws an error when not found
    GpsrOption *getGpsrOptionFromNetworkDatagramForUpdate(const Ptr<NetworkHeaderBase>& networkHeader);
    const GpsrOption *getGpsrOptionFromNetworkDatagram(const Ptr<const NetworkHeaderBase>& networkHeader) const;

    // configuration
    void configureInterfaces();

    // position
    Coord lookupPositionInGlobalRegistry(const L3Address& address) const;
    void storePositionInGlobalRegistry(const L3Address& address, const Coord& position) const;
    void storeSelfPositionInGlobalRegistry() const;
    Coord computeIntersectionInsideLineSegments(Coord& begin1, Coord& end1, Coord& begin2, Coord& end2) const;
    Coord getNeighborPosition(const L3Address& address) const;

    // angle
    double getVectorAngle(Coord vector) const;
    double getNeighborAngle(const L3Address& address) const;

    // address
    std::string getHostName() const;
    L3Address getSelfAddress() const;
    L3Address getSenderNeighborAddress(const Ptr<const NetworkHeaderBase>& networkHeader) const;

    // neighbor
    simtime_t getNextNeighborExpiration();
    void purgeNeighbors();
    std::vector<L3Address> getPlanarNeighbors() const;
    std::vector<L3Address> getPlanarNeighborsCounterClockwise(double startAngle) const;

    // next hop
    L3Address findNextHop(const L3Address& destination, GpsrOption *gpsrOption);
    L3Address findGreedyRoutingNextHop(const L3Address& destination, GpsrOption *gpsrOption);
    L3Address findPerimeterRoutingNextHop(const L3Address& destination, GpsrOption *gpsrOption);

    // routing
    Result routeDatagram(Packet *datagram, GpsrOption *gpsrOption);

    // netfilter
    virtual Result datagramPreRoutingHook(Packet *datagram) override;
    virtual Result datagramForwardHook(Packet *datagram) override { return ACCEPT; }
    virtual Result datagramPostRoutingHook(Packet *datagram) override { return ACCEPT; }
    virtual Result datagramLocalInHook(Packet *datagram) override { return ACCEPT; }
    virtual Result datagramLocalOutHook(Packet *datagram) override;

    // lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    // notification
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
};

} // namespace inet

#endif

