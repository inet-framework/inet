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
// Authors: Veronika Rybova, Tomas Prochazka (mailto:xproch21@stud.fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)

#ifndef __INET_PIMSM_H
#define __INET_PIMSM_H

#include "INETDefs.h"

#include "IPv4Route.h"
#include "PIMBase.h"

#define KAT 180.0                       /**< Keep alive timer, if RPT is disconnect */
#define HOLDTIME_HOST 180.0             /**< Holdtime for interface ET connected to host */
#define MAX_TTL 255                     /**< Maximum TTL */
#define CISCO_SPEC_SIM 0                /**< Enable Cisco specific simulation; 1 = enable, 0 = disable */

/**
 * @brief Class implements PIM-SM (sparse mode).
 */
class INET_API PIMSM : public PIMBase, protected cListener
{
    private:
        struct Route;

        struct Interface
        {
            Route *owner;
            InterfaceEntry *ie;
            cMessage *expiryTimer;

            // Assert state
            enum AssertState { NO_ASSERT_INFO, I_LOST_ASSERT, I_WON_ASSERT };
            AssertState assertState;
            cMessage *assertTimer;
            AssertMetric winnerMetric;

            // Assert flags
            bool couldAssert;
            bool assertTrackingDesired;

            Interface(Route *owner, InterfaceEntry *ie);
            virtual ~Interface();
            void startExpiryTimer(double holdTime);
            void startAssertTimer(double assertTime);
            void deleteAssertInfo();
        };

        /**
         * @brief Structure of incoming interface.
         * @details E.g.: GigabitEthernet1/4, RPF nbr 10.10.51.145
         */
        struct UpstreamInterface : public Interface
        {
            IPv4Address nextHop; // RPF nexthop, <unspec> at the DR in (S,G) routes

            UpstreamInterface(Route *owner, InterfaceEntry *ie, IPv4Address nextHop)
                : Interface(owner, ie), nextHop(nextHop) {}
            int getInterfaceId() const { return ie->getInterfaceId(); }
            IPv4Address rpfNeighbor() { return assertState == I_LOST_ASSERT ? winnerMetric.address : nextHop; }
        };

        struct DownstreamInterface : public Interface
        {
            /** States of each outgoing interface. */
            enum JoinPruneState { NO_INFO, JOIN, PRUNE_PENDING };

            JoinPruneState joinPruneState;
            cMessage *prunePendingTimer;

            bool                    shRegTun;           /**< Show interface which is also register tunnel interface*/

            DownstreamInterface(Route *owner, InterfaceEntry *ie, JoinPruneState joinPruneState, bool show = true)
                : Interface(owner, ie), joinPruneState(joinPruneState), prunePendingTimer(NULL), shRegTun(show) {}
            virtual ~DownstreamInterface();

            int getInterfaceId() const { return ie->getInterfaceId(); }
            bool isInOlist() { return joinPruneState != NO_INFO; } // XXX should be: ((has neighbor and not pruned) or has listener) and not assert looser
            bool isInImmediateOlist() const { return joinPruneState != NO_INFO && assertState != I_LOST_ASSERT; }
            bool isInInheritedOlist() const;
            void startPrunePendingTimer();
        };

        typedef std::vector<DownstreamInterface*> DownstreamInterfaceVector;

        class PIMSMOutInterface : public IMulticastRoute::OutInterface
        {
            DownstreamInterface *downstream;
            public:
                PIMSMOutInterface(DownstreamInterface *downstream)
                    : OutInterface(downstream->ie), downstream(downstream) {}
                virtual bool isEnabled() { return downstream->isInOlist(); }
        };

        enum RouteType
        {
            RP,    // (*,*,RP)
            G,     // (*,G)
            SG,    // (S,G)
            SGrpt  // (S,G,rpt)
        };

        // Holds (*,G), (S,G) or (S,G,rpt) state
        struct Route
        {
                enum Flag
                {
                    NO_FLAG = 0,
                    C       = 0x01,              /**< Connected */ // XXX Are there any connected downstream receivers?
                    P       = 0x02,              /**< Pruned */          // UpstreamJPState
                    F       = 0x04,              /**< Register flag*/
                    T       = 0x08               /**< SPT bit*/          // used to distinguish whether to forward on (*,*,RP)/(*,G) or on (S,G) state
                };

                PIMSM *owner;
                RouteType type;
                IPv4Address origin; // <unspec> in (*,G) routes
                IPv4Address group;
                IPv4Address rpAddr;                     /**< Randevous point */
                int flags;

                //Originated from destination.Ensures loop freeness.
                unsigned int sequencenumber;
                //Time of routing table entry creation
                simtime_t installtime; // XXX not used

                cMessage *keepAliveTimer;  // only for (S,G) routes
                cMessage *joinTimer;

                // our metric
                AssertMetric metric;           // metric of the unicast route to the source (if type==SG) or RP (if type==G)

                // Register state (only for (S,G) at the DR)
                enum RegisterState { RS_NO_INFO, RS_JOIN, RS_PRUNE, RS_JOIN_PENDING };
                RegisterState registerState;
                cMessage *registerStopTimer;

                // interface specific state
                UpstreamInterface *upstreamInterface;      // may be NULL at RP and at DR
                DownstreamInterfaceVector downstreamInterfaces; ///< Out interfaces (downstream)

                // computed values
                bool joinDesired;

            public:
                Route(PIMSM *owner, RouteType type, IPv4Address origin, IPv4Address group);
                ~Route();

                void clearDownstreamInterfaces();
                void addDownstreamInterface(DownstreamInterface *outInterface);
                void removeDownstreamInterface(unsigned int i);

                bool isFlagSet(Flag flag) const { return (flags & flag) != 0; }     /**< Returns if flag is set to entry or not*/
                void setFlags(int flags)   { this->flags |= flags; }                /**< Add flag to ineterface */
                void clearFlag(Flag flag)  { flags &= (~flag); }                   /**< Remove flag from ineterface */

                DownstreamInterface *addNewDownstreamInterface(InterfaceEntry *ie);
                DownstreamInterface *findDownstreamInterfaceByInterfaceId(int interfaceId);
                int findDownstreamInterface(InterfaceEntry *ie);

                bool isOilistNull();                                                /**< Returns true if list of outgoing interfaces is empty, otherwise false*/
                bool isImmediateOlistNull();
                bool isInheritedOlistNull();
                void updateJoinDesired();

                void startKeepAliveTimer();
                void startRegisterStopTimer(double interval);
                void startJoinTimer();
        };

        typedef std::map<SourceAndGroup, Route*> SGStateMap;

        // parameters
        IPv4Address rpAddr;
        std::string sptThreshold;
        double joinPrunePeriod;
        double defaultOverrideInterval;
        double defaultPropagationDelay;
        double keepAlivePeriod;
        double rpKeepAlivePeriod;
        double registerSuppressionTime;
        double registerProbeTime;
        double assertTime;
        double assertOverrideInterval;

        // state
        SGStateMap routes;

    private:
        // process signals
        void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
        void unroutableMulticastPacketArrived(IPv4Address srcAddr, IPv4Address destAddr);
        void multicastPacketArrivedOnRpfInterface(Route *route);
        void multicastPacketArrivedOnNonRpfInterface(Route *route, int interfaceId);
        void multicastPacketForwarded(IPv4Datagram *datagram);
        void multicastReceiverAdded(InterfaceEntry *ie, IPv4Address group);
        void multicastReceiverRemoved(InterfaceEntry *ie, IPv4Address group);

        // process timers
        void processPIMTimer(cMessage *timer);
        void processKeepAliveTimer(cMessage *timer);
        void processRegisterStopTimer(cMessage *timer);
        void processExpiryTimer(cMessage *timer);
        void processJoinTimer(cMessage *timer);
        void processPrunePendingTimer(cMessage *timer);
        void processAssertTimer(cMessage *timer);

        // send pim messages
        void sendPIMRegister(IPv4Datagram *datagram, IPv4Address dest, int outInterfaceId);
        void sendPIMRegisterStop(IPv4Address source, IPv4Address dest, IPv4Address multGroup, IPv4Address multSource);
        void sendPIMRegisterNull(IPv4Address multSource, IPv4Address multDest);
        void sendPIMJoin(IPv4Address group, IPv4Address source, IPv4Address upstreamNeighbor, RouteType JPtype);
        void sendPIMPrune(IPv4Address group, IPv4Address source, IPv4Address upstreamNeighbor, RouteType JPtype);
        void sendPIMAssert(IPv4Address source, IPv4Address group, AssertMetric metric, InterfaceEntry *ie, bool rptBit);
        void sendToIP(PIMPacket *packet, IPv4Address source, IPv4Address dest, int outInterfaceId, short ttl);
        void forwardMulticastData(IPv4Datagram *datagram, int outInterfaceId);

        // process PIM messages
        void processPIMPacket(PIMPacket *pkt);
        void processRegisterPacket(PIMRegister *pkt);
        void processRegisterStopPacket(PIMRegisterStop *pkt);
        void processJoinPrunePacket(PIMJoinPrune *pkt);
        void processAssertPacket(PIMAssert *pkt);

        void processJoinG(IPv4Address group, IPv4Address rp, IPv4Address upstreamNeighborField, int holdTime, InterfaceEntry *inInterface);
        void processJoinSG(IPv4Address origin, IPv4Address group, IPv4Address upstreamNeighborField, int holdTime, InterfaceEntry *inInterface);
        void processJoinSGrpt(IPv4Address origin, IPv4Address group, IPv4Address upstreamNeighborField, int holdTime, InterfaceEntry *inInterface);
        void processPruneG(IPv4Address multGroup, IPv4Address upstreamNeighborField, InterfaceEntry *inInterface);
        void processPruneSG(IPv4Address source, IPv4Address group, IPv4Address upstreamNeighborField, InterfaceEntry *inInterface);
        void processPruneSGrpt(IPv4Address source, IPv4Address group, IPv4Address upstreamNeighborField, InterfaceEntry *inInterface);
        void processAssertSG(Interface *interface, const AssertMetric &receivedMetric);
        void processAssertG(Interface *interface, const AssertMetric &receivedMetric);

        // computed intervals
        double joinPruneHoldTime() { return 3.5 * joinPrunePeriod; } // Holdtime in Join/Prune messages
        double effectivePropagationDelay() { return defaultPropagationDelay; }
        double effectiveOverrideInterval() { return defaultOverrideInterval; }
        double joinPruneOverrideInterval() { return effectivePropagationDelay() + effectiveOverrideInterval(); }

        // internal events
        void joinDesiredChanged(Route *route);

        // helpers
        PIMInterface *getIncomingInterface(IPv4Datagram *datagram);
        bool deleteMulticastRoute(Route *route);
        void cancelAndDeleteTimer(cMessage *&timer);
        void restartTimer(cMessage *timer, double interval);
        void restartExpiryTimer(Route *route, InterfaceEntry *originIntf, int holdTime);


        // routing table access
        void addGRoute(Route *route);
        void addSGRoute(Route *route);
        bool removeRoute(Route *route);
        Route *findGRoute(IPv4Address group);
        Route *findSGRoute(IPv4Address source, IPv4Address group);
        Route *createRouteG(IPv4Address group, int flags);
        Route *createRouteSG(IPv4Address source, IPv4Address group, int flags);
        IPv4MulticastRoute *createIPv4Route(Route *route);
        IPv4MulticastRoute *findIPv4Route(IPv4Address source, IPv4Address group);

    public:
        PIMSM() : PIMBase(PIMInterface::SparseMode) {}
        ~PIMSM();
        //PIM-SM clear implementation
        void setRPAddress(std::string address);
        void setSPTthreshold(std::string address);
        IPv4Address getRPAddress () {return rpAddr;}
        std::string getSPTthreshold () {return sptThreshold;}
        virtual bool IamRP (IPv4Address rpAddr) { return rt->isLocalAddress(rpAddr); }
        bool IamDR (IPv4Address sourceAddr);

	protected:
		virtual int numInitStages() const  {return NUM_INIT_STAGES;}
		virtual void handleMessage(cMessage *msg);
		virtual void initialize(int stage);
};

#endif
