//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef AODVROUTING_H_
#define AODVROUTING_H_

#include "INETDefs.h"
#include "IInterfaceTable.h"
#include "IAddressType.h"
#include "IRoutingTable.h"
#include "INetfilter.h"
#include "ILifecycle.h"
#include "NodeStatus.h"
#include "UDPSocket.h"
#include "AODVRouteData.h"
#include "UDPPacket.h"
#include "AODVControlPackets_m.h"
#include <map>

/*
 * This class implements AODV routing protocol and Netfilter hooks
 * in the IP-layer with regard to this protocol.
 */
class INET_API AODVRouting : public cSimpleModule, public ILifecycle, public INetfilter::IHook
{
    protected:

        class RREQIdentifier
        {
            public:
                Address originatorAddr;
                unsigned int rreqID;
                RREQIdentifier(Address originatorAddr, unsigned int rreqID) : originatorAddr(originatorAddr), rreqID(rreqID) {};
                bool operator==(const RREQIdentifier& other) const
                {
                  return this->originatorAddr == other.originatorAddr && this->rreqID == other.rreqID;
                }
        };

        class RREQIdentifierCompare
        {
            public:
                bool operator() (const RREQIdentifier& lhs, const RREQIdentifier& rhs) const
                {
                    return lhs.rreqID < rhs.rreqID;
                }
        };

        // context
        IAddressType *addressType; // to support both IPv4 and v6 addresses.

        // environment
        cModule *host; // the host module that owns this module
        IRoutingTable *routingTable; // the routing table of the owner module
        IInterfaceTable *interfaceTable; // the interface table of the owner module
        INetfilter *networkProtocol;

        // parameters
        unsigned int AodvUDPPort; // UDP port

        // state
        UDPSocket socket; // UDP socket to disseminate AODV control packets
        bool isOperational; // for lifecycle
        unsigned int rreqId; // when sending a new RREQ packet, rreqID incremented by one from the last id used by this node
        unsigned int sequenceNum; // it helps to prevent loops in the routes (RFC 3561 6.1 p11.)
        std::map<Address, WaitForRREP *> waitForRREPTimers; // timeout for a Route Replies
        std::map<RREQIdentifier, simtime_t,RREQIdentifierCompare> rreqsArrivalTime; // it maps (originatorAddr,rreqID) ( <- it is a unique identifier for
                                                              // an arbitrary RREQ in the network ) to arrival time
    protected:

        void handleMessage(cMessage *msg);
        void initialize(int stage);
        virtual int numInitStages() const { return NUM_INIT_STAGES; }

        bool hasOngoingRouteDiscovery(const Address& destAddr);
        void startRouteDiscovery(const Address& destAddr, unsigned int timeToLive = 0);

        /* Netfilter */
        Result ensureRouteForDatagram(INetworkDatagram * datagram);
        virtual Result datagramPreRoutingHook(INetworkDatagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, Address & nextHopAddress) { Enter_Method("datagramPreRoutingHook"); return ensureRouteForDatagram(datagram); }
        virtual Result datagramForwardHook(INetworkDatagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, Address & nextHopAddress);
        virtual Result datagramPostRoutingHook(INetworkDatagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, Address & nextHopAddress);
        virtual Result datagramLocalInHook(INetworkDatagram * datagram, const InterfaceEntry * inputInterfaceEntry);
        virtual Result datagramLocalOutHook(INetworkDatagram * datagram, const InterfaceEntry *& outputInterfaceEntry, Address & nextHopAddress);

        void startAODVRouting();
        void stopAODVRouting();

        Address getSelfIPAddress();
        void delayDatagram(INetworkDatagram * datagram);
        void sendRREQ(AODVRREP * rrep, const Address& destAddr, unsigned int timeToLive);
        void sendRERR();
        void sendRREP();
        void updateRoutingTable(IRoute * route, const Address& nextHop, unsigned int hopCount, bool hasValidDestNum, unsigned int destSeqNum, bool isInactive, simtime_t lifeTime);

        AODVRREQ * createRREQ(const Address& destAddr, unsigned int timeToLive);
        AODVRREP * createRREP(AODVRREQ * rreq, IRoute * route);
        AODVRREP * createGratuitousRREP(AODVRREQ * rreq, IRoute * route);
        void handleRREP(AODVRREP* rrep, const Address& sourceAddr);
        void handleRREQ(AODVRREQ* rreq, const Address& sourceAddr);
        void sendAODVPacket(AODVControlPacket * packet, const Address& destAddr, unsigned int timeToLive);
        virtual bool handleOperationStage(LifecycleOperation * operation, int stage, IDoneCallback * doneCallback) {} // TODO
    public:
        AODVRouting();
        virtual ~AODVRouting();
};

#endif
