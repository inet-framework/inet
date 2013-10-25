//
// Copyright (C) 2004 Andras Varga
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

#ifndef __INET_GPSR_H_
#define __INET_GPSR_H_

#include "INETDefs.h"
#include "Coord.h"
#include "ILifecycle.h"
#include "IMobility.h"
#include "IAddressType.h"
#include "INetfilter.h"
#include "IRoutingTable.h"
#include "NodeStatus.h"
#include "PositionTable.h"
#include "UDPPacket.h"
#include "GPSR_m.h"

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
        GPSRPlanarizationMode planarizationMode;
        const char * interfaces;
        simtime_t beaconInterval;
        simtime_t maxJitter;
        simtime_t neighborValidityInterval;

        // context
        cModule * host;
        NodeStatus * nodeStatus;
        IMobility * mobility;
        IAddressType * addressType;
        IInterfaceTable * interfaceTable;
        IRoutingTable * routingTable; // TODO: delete when necessary functions are moved to interface table
        INetfilter * networkProtocol;
        static PositionTable globalPositionTable; // KLUDGE: implement position registry protocol

        // internal
        cMessage * beaconTimer;
        cMessage * purgeNeighborsTimer;
        PositionTable neighborPositionTable;

    public:
        GPSR();
        virtual ~GPSR();

    protected:
        // module interface
        virtual int numInitStages() const { return NUM_INIT_STAGES; }
        void initialize(int stage);
        void handleMessage(cMessage * message);

    private:
        // handling messages
        void processSelfMessage(cMessage * message);
        void processMessage(cMessage * message);

        // handling beacon timers
        void scheduleBeaconTimer();
        void processBeaconTimer();

        // handling purge neighbors timers
        void schedulePurgeNeighborsTimer();
        void processPurgeNeighborsTimer();

        // handling UDP packets
        void sendUDPPacket(UDPPacket * packet, double delay);
        void processUDPPacket(UDPPacket * packet);

        // handling beacons
        GPSRBeacon * createBeacon();
        void sendBeacon(GPSRBeacon * beacon, double delay);
        void processBeacon(GPSRBeacon * beacon);

        // handling packets
        GPSRPacket * createPacket(Address destination, cPacket * content);
        int computePacketBitLength(GPSRPacket * packet);

        // configuration
        bool isNodeUp();
        void configureInterfaces();

        // position
        Coord intersectSections(Coord & begin1, Coord & end1, Coord & begin2, Coord & end2);
        Coord getDestinationPosition(const Address & address);
        Coord getNeighborPosition(const Address & address);

        // angle
        double getVectorAngle(Coord vector);
        double getDestinationAngle(const Address & address);
        double getNeighborAngle(const Address & address);

        // address
        std::string getHostName();
        Address getSelfAddress();
        Address getSenderNeighborAddress(INetworkDatagram * datagram);

        // neighbor
        simtime_t getNextNeighborExpiration();
        void purgeNeighbors();
        std::vector<Address> getPlanarNeighbors();
        Address getNextPlanarNeighborCounterClockwise(Address & startNeighborAddress, double startNeighborAngle);

        // next hop
        Address findNextHop(INetworkDatagram * datagram, const Address & destination);
        Address findGreedyRoutingNextHop(INetworkDatagram * datagram, const Address & destination);
        Address findPerimeterRoutingNextHop(INetworkDatagram * datagram, const Address & destination);

        // routing
        Result routeDatagram(INetworkDatagram * datagram, const InterfaceEntry *& outputInterfaceEntry, Address & nextHop);

        // netfilter
        virtual Result datagramPreRoutingHook(INetworkDatagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, Address & nextHop);
        virtual Result datagramForwardHook(INetworkDatagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, Address & nextHop) { return ACCEPT; }
        virtual Result datagramPostRoutingHook(INetworkDatagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, Address & nextHop) { return ACCEPT; }
        virtual Result datagramLocalInHook(INetworkDatagram * datagram, const InterfaceEntry * inputInterfaceEntry);
        virtual Result datagramLocalOutHook(INetworkDatagram * datagram, const InterfaceEntry *& outputInterfaceEntry, Address & nextHop);

        // lifecycle
        virtual bool handleOperationStage(LifecycleOperation * operation, int stage, IDoneCallback * doneCallback);

        // notification
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
};

#endif
