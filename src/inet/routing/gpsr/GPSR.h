//
// Copyright (C) 2013 Opensim Ltd
// Author: Levente Meszaros
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __INET_GPSR_H
#define __INET_GPSR_H

#include "inet/common/INETDefs.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/routing/gpsr/PositionTable.h"
#include "inet/transportlayer/udp/UDPPacket.h"
#include "inet/routing/gpsr/GPSR_m.h"

namespace inet {

/**
 * This class implements the Greedy Perimeter Stateless Routing for Wireless Networks.
 * The implementation supports both GG and RNG planarization algorithms.
 *
 * For more information on the routing algorithm, see the GPSR paper
 * http://www.eecs.harvard.edu/~htk/publication/2000-mobi-karp-kung.pdf
 */
// TODO: optimize internal data structures for performance to use less lookups and be more prepared for routing a packet
// KLUDGE: implement position registry protocol instead of using a global variable
// KLUDGE: the GPSR packet is now used to wrap the content of network datagrams
// KLUDGE: we should rather add these fields as header extensions
class INET_API GPSR : public cSimpleModule, public ILifecycle, public cListener, public INetfilter::IHook
{
  private:
    // GPSR parameters
    GPSRPlanarizationMode planarizationMode = (GPSRPlanarizationMode)-1;
    const char *interfaces = nullptr;
    simtime_t beaconInterval;
    simtime_t maxJitter;
    simtime_t neighborValidityInterval;

    // context
    cModule *host = nullptr;
    NodeStatus *nodeStatus = nullptr;
    IMobility *mobility = nullptr;
    IL3AddressType *addressType = nullptr;
    IInterfaceTable *interfaceTable = nullptr;
    IRoutingTable *routingTable = nullptr;    // TODO: delete when necessary functions are moved to interface table
    INetfilter *networkProtocol = nullptr;
    static PositionTable globalPositionTable;    // KLUDGE: implement position registry protocol

    // packet size
    int positionByteLength = -1;

    // internal
    cMessage *beaconTimer = nullptr;
    cMessage *purgeNeighborsTimer = nullptr;
    PositionTable neighborPositionTable;

  public:
    GPSR();
    virtual ~GPSR();

  protected:
    // module interface
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override;
    void handleMessage(cMessage *message) override;

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
    void sendUDPPacket(UDPPacket *packet, double delay);
    void processUDPPacket(UDPPacket *packet);

    // handling beacons
    GPSRBeacon *createBeacon();
    void sendBeacon(GPSRBeacon *beacon, double delay);
    void processBeacon(GPSRBeacon *beacon);

    // handling packets
    GPSROption *createGpsrOption(L3Address destination, cPacket *content);
    int computeOptionLength(GPSROption *gpsrOption);
    void setGprsOptionOnNetworkDatagram(INetworkDatagram *datagram);
    void removeGprsOptionFromNetworkDatagram(INetworkDatagram *datagram);

    // returns nullptr if not found
    GPSROption *findGprsOptionInNetworkDatagram(INetworkDatagram *datagram);

    // throws an error when not found
    GPSROption *getGprsOptionFromNetworkDatagram(INetworkDatagram *datagram);
    const GPSROption *getGprsOptionFromNetworkDatagram(INetworkDatagram *datagram) const { return const_cast<GPSR *>(this)->getGprsOptionFromNetworkDatagram(datagram); }

    // configuration
    bool isNodeUp() const;
    void configureInterfaces();

    // position
    static Coord intersectSections(Coord& begin1, Coord& end1, Coord& begin2, Coord& end2);
    Coord getDestinationPosition(const L3Address& address) const;
    Coord getNeighborPosition(const L3Address& address) const;

    // angle
    static double getVectorAngle(Coord vector);
    double getDestinationAngle(const L3Address& address);
    double getNeighborAngle(const L3Address& address);

    // address
    std::string getHostName() const;
    L3Address getSelfAddress() const;
    L3Address getSenderNeighborAddress(INetworkDatagram *datagram) const;

    // neighbor
    simtime_t getNextNeighborExpiration();
    void purgeNeighbors();
    std::vector<L3Address> getPlanarNeighbors();
    L3Address getNextPlanarNeighborCounterClockwise(const L3Address& startNeighborAddress, double startNeighborAngle);

    // next hop
    L3Address findNextHop(INetworkDatagram *datagram, const L3Address& destination);
    L3Address findGreedyRoutingNextHop(INetworkDatagram *datagram, const L3Address& destination);
    L3Address findPerimeterRoutingNextHop(INetworkDatagram *datagram, const L3Address& destination);

    // routing
    Result routeDatagram(INetworkDatagram *datagram, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHop);

    // netfilter
    virtual Result datagramPreRoutingHook(INetworkDatagram *datagram, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHop) override;
    virtual Result datagramForwardHook(INetworkDatagram *datagram, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHop) override { return ACCEPT; }
    virtual Result datagramPostRoutingHook(INetworkDatagram *datagram, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHop) override { return ACCEPT; }
    virtual Result datagramLocalInHook(INetworkDatagram *datagram, const InterfaceEntry *inputInterfaceEntry) override;
    virtual Result datagramLocalOutHook(INetworkDatagram *datagram, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHop) override;

    // lifecycle
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

    // notification
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj) override;
};

} // namespace inet

#endif // ifndef __INET_GPSR_H

