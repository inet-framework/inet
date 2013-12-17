//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// Authors: Veronika Rybova, Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)

#ifndef __INET_PIMDM_H
#define __INET_PIMDM_H

#include <omnetpp.h>
#include "PIMPacket_m.h"
#include "PIMTimer_m.h"
#include "InterfaceTableAccess.h"
#include "NotifierConsts.h"
#include "PIMNeighborTable.h"
#include "PIMInterfaceTable.h"
#include "IPv4ControlInfo.h"
#include "IPv4InterfaceData.h"
#include "IPv4Route.h"
#include "PIMBase.h"

/**
 * @brief Class implements PIM-DM (dense mode).
 */
class PIMDM : public PIMBase, protected cListener
{
    private:
        struct TimerContext
        {
            IPv4Address source;
            IPv4Address group;
            int interfaceId;

            TimerContext(IPv4Address source, IPv4Address group) : source(source), group(group), interfaceId(-1) {}
            TimerContext(IPv4Address source, IPv4Address group, int interfaceId) : source(source), group(group), interfaceId(interfaceId) {}
        };

        // per (S,G) state

        struct UpstreamInterface : public PIMMulticastRoute::PIMInInterface
        {
            enum State { FORWARDING, PRUNED, ACK_PENDING }; // XXX not yet used

            State state;
            cMessage* graftRetryTimer;
            cMessage* sourceActiveTimer;
            cMessage* stateRefreshTimer;
            // TODO missing overrideTimer, pruneLimitTimer

            UpstreamInterface(InterfaceEntry *ie, IPv4Address neighbor)
                : PIMInInterface(ie, neighbor), state(FORWARDING),
                  graftRetryTimer(NULL), sourceActiveTimer(NULL), stateRefreshTimer(NULL) {}
            int getInterfaceId() const { return ie->getInterfaceId(); }
        };

        struct DownstreamInterface : public PIMMulticastRoute::PIMOutInterface
        {
            // prune state
            cMessage *pruneTimer;
            // TODO prunePendingTimer

            // assert winner state
            // TODO assertState, assertTimer, winnerAddress, winnerMetric

            DownstreamInterface(InterfaceEntry *ie)
                : PIMOutInterface(ie) {}
            DownstreamInterface(InterfaceEntry *ie, PIMMulticastRoute::InterfaceState forwarding, PIMInterface::PIMMode mode, cMessage *pruneTimer,
                    PIMMulticastRoute::AssertState assert)
                : PIMOutInterface(ie, forwarding, mode, assert), pruneTimer(pruneTimer) {}
        };


    private:
        // parameters
        double pruneInterval;
        double graftRetryInterval;
        double sourceActiveInterval;
        double stateRefreshInterval;
	public:
        PIMDM() : PIMBase(PIMInterface::DenseMode) {}

	private:
	    // process events
        void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
	    void newMulticast(IPv4Address srcAddress, IPv4Address destAddress);
	    void newMulticastAddr(PIMInterface *pimInt, IPv4Address newAddr);
	    void oldMulticastAddr(PIMInterface *pimInt, IPv4Address oldAddr);
	    void dataOnPruned(IPv4Address destAddr, IPv4Address srcAddr);
	    void dataOnNonRpf(IPv4Address group, IPv4Address source, int intId);
	    void dataOnRpf(IPv4Datagram *datagram);
	    void rpfIntChange(PIMMulticastRoute *route);

	    // process timers
	    void processPIMTimer(cMessage *timer);
	    void processPruneTimer(cMessage *timer);
	    void processGraftRetryTimer(cMessage *timer);
	    void processSourceActiveTimer(cMessage *timer);
	    void processStateRefreshTimer(cMessage *timer);

	    // create timers
	    cMessage *createPruneTimer(IPv4Address source, IPv4Address group, int intId, int holdTime);
	    cMessage *createGraftRetryTimer(IPv4Address source, IPv4Address group);
	    cMessage *createSourceActiveTimer(IPv4Address source, IPv4Address group);
	    cMessage *createStateRefreshTimer(IPv4Address source, IPv4Address group);

	    // process PIM packets
	    void processPIMPkt(PIMPacket *pkt);
	    void processJoinPruneGraftPacket(PIMJoinPrune *pkt, PIMPacketType type);
	    void processPrunePacket(PIMMulticastRoute *route, int intId, int holdTime);
	    void processGraftPacket(IPv4Address source, IPv4Address group, IPv4Address sender, int intId);
	    void processGraftAckPacket(PIMMulticastRoute *route);
	    void processStateRefreshPacket(PIMStateRefresh *pkt);

	    //create PIM packets
	    void sendPimJoinPrune(IPv4Address nextHop, IPv4Address src, IPv4Address grp, int intId);
	    void sendPimGraft(IPv4Address nextHop, IPv4Address src, IPv4Address grp, int intId);
	    void sendPimGraftAck(PIMGraftAck *msg);
	    void sendPimStateRefresh(IPv4Address originator, IPv4Address src, IPv4Address grp, int intId, bool P);

	    PIMInterface *getIncomingInterface(IPv4Datagram *datagram);
	    void cancelAndDeleteTimer(cMessage *timer);
        bool deleteMulticastRoute(PIMMulticastRoute *route);

        // routing table access
        PIMMulticastRoute *getRouteFor(IPv4Address group, IPv4Address source);
        std::vector<PIMMulticastRoute*> getRouteFor(IPv4Address group);
        std::vector<PIMMulticastRoute*> getRoutesForSource(IPv4Address source);

	protected:
		virtual int numInitStages() const  {return NUM_INIT_STAGES;}
		virtual void handleMessage(cMessage *msg);
		virtual void initialize(int stage);
};

#endif
