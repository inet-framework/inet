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
        // per (S,G) state
        struct SourceGroupState;

        struct UpstreamInterface
        {
            enum GraftPruneState
            {
                FORWARDING, // oiflist != NULL
                PRUNED,     // olist is empty
                ACK_PENDING // waiting for a Graft Ack
            };

            SourceGroupState *owner;
            InterfaceEntry *ie;
            IPv4Address nextHop;

            // graft prune state
            GraftPruneState graftPruneState;
            cMessage* graftRetryTimer;
            cMessage* overrideTimer; // XXX not used
            simtime_t lastPruneSentTime; // for rate limiting prune messages, 0 if no prune was sent

            // originator state
            cMessage* sourceActiveTimer; // route will be deleted when this timer expires
            cMessage* stateRefreshTimer;

            UpstreamInterface(SourceGroupState *owner, InterfaceEntry *ie, IPv4Address neighbor)
                : owner(owner), ie(ie), nextHop(neighbor),
                  graftPruneState(FORWARDING), graftRetryTimer(NULL), overrideTimer(NULL), lastPruneSentTime(0.0),
                  sourceActiveTimer(NULL), stateRefreshTimer(NULL)
                { ASSERT(owner); ASSERT(ie); }
            virtual ~UpstreamInterface();
            int getInterfaceId() const { return ie->getInterfaceId(); }

            void startGraftRetryTimer();
            void startOverrideTimer();
            void startSourceActiveTimer();
            void startStateRefreshTimer();
            void startPruneLimitTimer() { lastPruneSentTime = simTime(); }
            void cancelOverrideTimer();
            void stopPruneLimitTimer() { lastPruneSentTime = 0; }
            bool isPruneLimitTimerRunning() { return lastPruneSentTime > 0.0 && simTime() < lastPruneSentTime + owner->owner->pruneLimitInterval; }
        };

        struct DownstreamInterface
        {
            enum PruneState
            {
                NO_INFO,       // no prune info, neither pruneTimer or prunePendingTimer is running
                PRUNE_PENDING, // received a prune from a downstream neighbor, waiting for an override
                PRUNED         // received a prune from a downstream neighbor and it was not overridden
            };

            enum AssertState { NO_ASSERT_INFO, I_LOST_ASSERT, I_WON_ASSERT };

            SourceGroupState *owner;
            InterfaceEntry *ie;

            // prune state
            PruneState pruneState;
            cMessage *pruneTimer;
            cMessage *prunePendingTimer;

            // assert winner state XXX not used
            AssertState assertState;
            cMessage *assertTimer;
            IPv4Address winnerAddress;
            // TODO winnerMetric

            DownstreamInterface(SourceGroupState *owner, InterfaceEntry *ie)
                : owner(owner), ie(ie),
                  pruneState(NO_INFO), pruneTimer(NULL), prunePendingTimer(NULL),
                  assertState(NO_ASSERT_INFO), assertTimer(NULL)
                { ASSERT(owner), ASSERT(ie);}
            ~DownstreamInterface();
            bool isInOlist() const;
            void startPruneTimer(double holdTime);
            void stopPruneTimer();
            void startPrunePendingTimer(double overrideInterval);
            void stopPrunePendingTimer();
        };

        struct SourceAndGroup
        {
            IPv4Address source;
            IPv4Address group;

            SourceAndGroup(IPv4Address source, IPv4Address group) : source(source), group(group) {}
            bool operator==(const SourceAndGroup &other) const { return source == other.source && group == other.group; }
            bool operator!=(const SourceAndGroup &other) const { return source != other.source || group != other.group; }
            bool operator<(const SourceAndGroup &other) const { return source < other.source || (source == other.source && group < other.group); }
        };

        struct SourceGroupState
        {
            PIMDM *owner;
            IPv4Address source;
            IPv4Address group;
            int flags;
            UpstreamInterface *upstreamInterface;
            std::vector<DownstreamInterface*> downstreamInterfaces;

            SourceGroupState() : owner(NULL), flags(0), upstreamInterface(NULL) {}
            ~SourceGroupState();
            bool isFlagSet(PIMMulticastRoute::Flag flag) const { return (flags & flag) != 0; }
            void setFlags(int flags)   { this->flags |= flags; }
            void clearFlag(PIMMulticastRoute::Flag flag)  { flags &= (~flag); }
            DownstreamInterface *findDownstreamInterfaceByInterfaceId(int interfaceId) const;
            DownstreamInterface *createDownstreamInterface(InterfaceEntry *ie);
            void removeDownstreamInterface(int interfaceId);
            bool isOilistNull();
        };

        typedef std::map<SourceAndGroup, SourceGroupState> SGStateMap;

        // KLUDGE
        class PIMDMOutInterface : public IMulticastRoute::OutInterface
        {
            DownstreamInterface *downstream;

            public:
                PIMDMOutInterface(InterfaceEntry *ie, DownstreamInterface *downstream)
                    : IMulticastRoute::OutInterface(ie), downstream(downstream) {}
                virtual bool isEnabled() { return downstream->isInOlist(); }
        };

    private:
        // parameters
        double pruneInterval;
        double pruneLimitInterval;
        double overrideInterval;
        double graftRetryInterval;
        double sourceActiveInterval;
        double stateRefreshInterval;

        // state
        SGStateMap sgStates;

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
	    void processPrunePendingTimer(cMessage *timer);
	    void processGraftRetryTimer(cMessage *timer);
	    void processOverrideTimer(cMessage *timer);
	    void processSourceActiveTimer(cMessage *timer);
	    void processStateRefreshTimer(cMessage *timer);

	    // process PIM packets
	    void processPIMPacket(PIMPacket *pkt);
        void processJoinPrunePacket(PIMJoinPrune *pkt);
        void processGraftPacket(PIMGraft *pkt);
        void processGraftAckPacket(PIMGraftAck *pkt);
	    void processStateRefreshPacket(PIMStateRefresh *pkt);
	    void processAssertPacket(PIMAssert *pkt);

        void processPrune(SourceGroupState *sgState, int intId, int holdTime, int numRpfNeighbors);
        void processJoin(SourceGroupState *sgState, int intId, int numRpfNeighbors);
        void processGraft(IPv4Address source, IPv4Address group, IPv4Address sender, int intId);

        // process olist changes
        void processOlistEmptyEvent(SourceGroupState *sgState);
        void processOlistNonEmptyEvent(SourceGroupState *sgState);

	    // create and send PIM packets
	    void sendPrunePacket(IPv4Address nextHop, IPv4Address src, IPv4Address grp, int intId);
	    void sendJoinPacket(IPv4Address nextHop, IPv4Address source, IPv4Address group, int interfaceId);
	    void sendGraftPacket(IPv4Address nextHop, IPv4Address src, IPv4Address grp, int intId);
	    void sendGraftAckPacket(PIMGraft *msg);
	    void sendStateRefreshPacket(IPv4Address originator, IPv4Address src, IPv4Address grp, int intId, bool P);
	    void sendToIP(PIMPacket *packet, IPv4Address source, IPv4Address dest, int outInterfaceId);

	    // helpers
        void restartTimer(cMessage *timer, double interval);
	    PIMInterface *getIncomingInterface(IPv4Datagram *datagram);
        PIMMulticastRoute *getRouteFor(IPv4Address group, IPv4Address source);
        SourceGroupState *getSourceGroupState(IPv4Address source, IPv4Address group);
        void deleteSourceGroupState(IPv4Address source, IPv4Address group);

	protected:
		virtual int numInitStages() const  {return NUM_INIT_STAGES;}
		virtual void handleMessage(cMessage *msg);
		virtual void initialize(int stage);
};

#endif
