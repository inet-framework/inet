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

            PIMDM *owner;
            State state;
            cMessage* graftRetryTimer;
            cMessage* sourceActiveTimer; // route will be deleted when this timer expires
            cMessage* stateRefreshTimer;
            cMessage* overrideTimer; // XXX not used
            simtime_t lastPruneSentTime; // for rate limiting prune messages, 0 if no prune was sent

            UpstreamInterface(PIMDM *owner, InterfaceEntry *ie, IPv4Address neighbor)
                : PIMInInterface(ie, neighbor), owner(owner), state(FORWARDING),
                  graftRetryTimer(NULL), sourceActiveTimer(NULL), stateRefreshTimer(NULL),
                  overrideTimer(NULL), lastPruneSentTime(0.0) {}
            virtual ~UpstreamInterface();
            int getInterfaceId() const { return ie->getInterfaceId(); }

            void startPruneLimitTimer() { lastPruneSentTime = simTime(); }
            bool isPruneLimitTimerRunning() { return lastPruneSentTime > 0.0 && simTime() < lastPruneSentTime + owner->pruneLimitInterval; }
        };

        struct DownstreamInterface : public PIMMulticastRoute::PIMOutInterface
        {
            enum State { NO_INFO, PRUNE_PENDING, PRUNED }; // XXX not yet used

            PIMDM *owner;
            State state;
            // prune state
            cMessage *pruneTimer;
            // TODO prunePendingTimer

            // assert winner state
            // TODO assertState, assertTimer, winnerAddress, winnerMetric

            DownstreamInterface(PIMDM *owner, InterfaceEntry *ie)
                : PIMOutInterface(ie), owner(owner), pruneTimer(NULL) {}
            DownstreamInterface(PIMDM *owner, InterfaceEntry *ie, PIMMulticastRoute::InterfaceState forwarding, PIMInterface::PIMMode mode, cMessage *pruneTimer,
                    PIMMulticastRoute::AssertState assert)
                : PIMOutInterface(ie, forwarding, mode, assert), owner(owner), pruneTimer(pruneTimer) {}
            ~DownstreamInterface();
        };

    private:
        // parameters
        double pruneInterval;
        double pruneLimitInterval;
        double graftRetryInterval;
        double sourceActiveInterval;
        double stateRefreshInterval;
	public:
        PIMDM() : PIMBase(PIMInterface::DenseMode) {}

	private:
	    // process signals
        void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
	    void unroutableMulticastPacketArrived(IPv4Address srcAddress, IPv4Address destAddress);
        void multicastPacketArrivedOnNonRpfInterface(IPv4Address group, IPv4Address source, int interfaceId);
        void multicastPacketArrivedOnRpfInterface(IPv4Address group, IPv4Address source, int interfaceId);
	    void multicastReceiverAdded(PIMInterface *pimInt, IPv4Address newAddr);
	    void multicastReceiverRemoved(PIMInterface *pimInt, IPv4Address oldAddr);
	    void rpfInterfaceHasChanged(PIMMulticastRoute *route, InterfaceEntry *newRpfInterface);

	    // process timers
	    void processPIMTimer(cMessage *timer);
	    void processPruneTimer(cMessage *timer);
	    void processGraftRetryTimer(cMessage *timer);
	    void processSourceActiveTimer(cMessage *timer);
	    void processStateRefreshTimer(cMessage *timer);

	    // create/schedule timers
	    cMessage *createPruneTimer(IPv4Address source, IPv4Address group, int intId, int holdTime);
	    cMessage *createGraftRetryTimer(IPv4Address source, IPv4Address group);
	    cMessage *createSourceActiveTimer(IPv4Address source, IPv4Address group);
	    cMessage *createStateRefreshTimer(IPv4Address source, IPv4Address group);
	    void restartTimer(cMessage *timer, double interval);

	    // process PIM packets
	    void processPIMPacket(PIMPacket *pkt);
        void processJoinPrunePacket(PIMJoinPrune *pkt);
        void processGraftPacket(PIMGraft *pkt);
        void processGraftAckPacket(PIMGraftAck *pkt);
	    void processStateRefreshPacket(PIMStateRefresh *pkt);
	    void processAssertPacket(PIMAssert *pkt);

        void processPrune(PIMMulticastRoute *route, int intId, int holdTime);
        void processGraft(IPv4Address source, IPv4Address group, IPv4Address sender, int intId);

	    // create and send PIM packets
	    void sendPrunePacket(IPv4Address nextHop, IPv4Address src, IPv4Address grp, int intId);
	    void sendGraftPacket(IPv4Address nextHop, IPv4Address src, IPv4Address grp, int intId);
	    void sendGraftAckPacket(PIMGraft *msg);
	    void sendStateRefreshPacket(IPv4Address originator, IPv4Address src, IPv4Address grp, int intId, bool P);
	    void sendToIP(PIMPacket *packet, IPv4Address source, IPv4Address dest, int outInterfaceId);

	    // helpers
	    PIMInterface *getIncomingInterface(IPv4Datagram *datagram);
	    void cancelAndDeleteTimer(cMessage *timer);
        PIMMulticastRoute *getRouteFor(IPv4Address group, IPv4Address source);
        std::vector<PIMMulticastRoute*> getRouteFor(IPv4Address group);

	protected:
		virtual int numInitStages() const  {return NUM_INIT_STAGES;}
		virtual void handleMessage(cMessage *msg);
		virtual void initialize(int stage);
};

#endif
