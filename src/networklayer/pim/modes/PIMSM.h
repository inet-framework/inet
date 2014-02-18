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
        friend std::ostream& operator<<(std::ostream &out, const SourceAndGroup &sourceGroup);
        friend std::ostream& operator<<(std::ostream &out, const Route &sourceGroup);

        struct PimsmInterface : public Interface
        {
            cMessage *expiryTimer;

            // Assert flags
            bool couldAssert;
            bool assertTrackingDesired;

            PimsmInterface(Route *owner, InterfaceEntry *ie);
            virtual ~PimsmInterface();
            Route *route() const { return check_and_cast<Route*>(owner); }
            void startExpiryTimer(double holdTime);
        };

        /**
         * @brief Structure of incoming interface.
         * @details E.g.: GigabitEthernet1/4, RPF nbr 10.10.51.145
         */
        struct UpstreamInterface : public PimsmInterface
        {
            IPv4Address nextHop; // RPF nexthop, <unspec> at the DR in (S,G) routes

            UpstreamInterface(Route *owner, InterfaceEntry *ie, IPv4Address nextHop)
                : PimsmInterface(owner, ie), nextHop(nextHop) {}
            int getInterfaceId() const { return ie->getInterfaceId(); }
            IPv4Address rpfNeighbor() { return assertState == I_LOST_ASSERT ? winnerMetric.address : nextHop; }
        };

        struct DownstreamInterface : public PimsmInterface
        {
            /** States of each outgoing interface. */
            enum JoinPruneState { NO_INFO, JOIN, PRUNE_PENDING };

            JoinPruneState joinPruneState;
            cMessage *prunePendingTimer;

            bool                    shRegTun;           /**< Show interface which is also register tunnel interface*/

            DownstreamInterface(Route *owner, InterfaceEntry *ie, JoinPruneState joinPruneState, bool show = true)
                : PimsmInterface(owner, ie), joinPruneState(joinPruneState), prunePendingTimer(NULL), shRegTun(show) {}
            virtual ~DownstreamInterface();
            PIMSM *pimsm() const { return check_and_cast<PIMSM*>(owner->owner); }

            int getInterfaceId() const { return ie->getInterfaceId(); }
            bool isInOlist() { return joinPruneState != NO_INFO; } // XXX should be: ((has neighbor and not pruned) or has listener) and not assert looser
            bool isInImmediateOlist() const;
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
                virtual bool isEnabled() { return downstream->isInInheritedOlist(); }
        };

        enum RouteType
        {
            RP,    // (*,*,RP)
            G,     // (*,G)
            SG,    // (S,G)
            SGrpt  // (S,G,rpt)
        };

        // Holds (*,G), (S,G) or (S,G,rpt) state
        struct Route : public RouteEntry
        {
                // flags
                enum
                {
                    NO_FLAG      = 0x00,
                    CONNECTED    = 0x01,              /**< Connected */ // XXX Are there any connected downstream receivers?
                    PRUNED       = 0x02,              /**< Pruned */          // UpstreamJPState
                    REGISTER     = 0x04,              /**< Register flag*/
                    SPT_BIT      = 0x08,              /**< SPT bit*/          // used to distinguish whether to forward on (*,*,RP)/(*,G) or on (S,G) state
                    JOIN_DESIRED = 0x10
                };

                RouteType type;
                IPv4Address rpAddr;                     /**< Randevous point */

                // related routes
                Route *rpRoute;
                Route *gRoute;
                Route *sgrptRoute;

                //Originated from destination.Ensures loop freeness.
                unsigned int sequencenumber;
                //Time of routing table entry creation
                simtime_t installtime; // XXX not used

                cMessage *keepAliveTimer;  // only for (S,G) routes
                cMessage *joinTimer;

                // Register state (only for (S,G) at the DR)
                enum RegisterState { RS_NO_INFO, RS_JOIN, RS_PRUNE, RS_JOIN_PENDING };
                RegisterState registerState;
                cMessage *registerStopTimer;

                // interface specific state
                UpstreamInterface *upstreamInterface;      // may be NULL at RP and at DR
                DownstreamInterfaceVector downstreamInterfaces; ///< Out interfaces (downstream)

            public:
                Route(PIMSM *owner, RouteType type, IPv4Address origin, IPv4Address group);
                virtual ~Route();
                PIMSM *pimsm() const { return check_and_cast<PIMSM*>(owner); }

                void clearDownstreamInterfaces();
                void addDownstreamInterface(DownstreamInterface *outInterface);
                void removeDownstreamInterface(unsigned int i);

                DownstreamInterface *addNewDownstreamInterface(InterfaceEntry *ie);
                DownstreamInterface *findDownstreamInterfaceByInterfaceId(int interfaceId);
                int findDownstreamInterface(InterfaceEntry *ie);

                bool isOilistNull();                                                /**< Returns true if list of outgoing interfaces is empty, otherwise false*/
                bool isImmediateOlistNull();
                bool isInheritedOlistNull();
                bool joinDesired() const { return isFlagSet(JOIN_DESIRED); }

                void startKeepAliveTimer();
                void startRegisterStopTimer(double interval);
                void startJoinTimer();
        };

        typedef std::map<SourceAndGroup, Route*> RoutingTable;

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
        RoutingTable gRoutes;
        RoutingTable sgRoutes;

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
        void processAssertSG(PimsmInterface *interface, const AssertMetric &receivedMetric);
        void processAssertG(PimsmInterface *interface, const AssertMetric &receivedMetric);

        // computed intervals
        double joinPruneHoldTime() { return 3.5 * joinPrunePeriod; } // Holdtime in Join/Prune messages
        double effectivePropagationDelay() { return defaultPropagationDelay; }
        double effectiveOverrideInterval() { return defaultOverrideInterval; }
        double joinPruneOverrideInterval() { return effectivePropagationDelay() + effectiveOverrideInterval(); }

        // internal events
        void joinDesiredChanged(Route *route);

        // update actions
        void updateJoinDesired(Route *route);

        // helpers
        PIMInterface *getIncomingInterface(IPv4Datagram *datagram);
        bool deleteMulticastRoute(Route *route);
        void cancelAndDeleteTimer(cMessage *&timer);
        void restartTimer(cMessage *timer, double interval);
        void restartExpiryTimer(Route *route, InterfaceEntry *originIntf, int holdTime);


        // routing table access
        bool removeRoute(Route *route);
        Route *findRouteG(IPv4Address group);
        Route *findRouteSG(IPv4Address source, IPv4Address group);
        Route *addNewRouteG(IPv4Address group, int flags);
        Route *addNewRouteSG(IPv4Address source, IPv4Address group, int flags);
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
