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

#include <omnetpp.h>
#include "PIMPacket_m.h"

#include "InterfaceTableAccess.h"
#include "NotifierConsts.h"
#include "PIMNeighborTable.h"
#include "PIMInterfaceTable.h"
#include "IPv4ControlInfo.h"
#include "IPv4InterfaceData.h"
#include "IPv4Route.h"
#include "PIMBase.h"

#define KAT 180.0                       /**< Keep alive timer, if RPT is disconnect */
#define KAT2 210.0                      /**< Keep alive timer, if RPT is connect */
#define RST 60.0                        /**< Register-stop Timer*/
#define JT 60.0                         /**< Join Timer */
#define REGISTER_PROBE_TIME 5.0         /**< Register Probe Time */
#define HOLDTIME 210.0                  /**< Holdtime for Expiry Timer */
#define HOLDTIME_HOST 180.0             /**< Holdtime for interface ET connected to host */
#define PPT 3.0                         /**< value for Prune-Pending Timer*/
#define MAX_TTL 255                     /**< Maximum TTL */
#define NO_INT_TIMER -1
#define CISCO_SPEC_SIM 1                /**< Enable Cisco specific simulation; 1 = enable, 0 = disable */

/**
 * @brief Class implements PIM-SM (sparse mode).
 */
class PIMSM : public PIMBase, protected cListener
{
    private:
        /**  Register machine States. */
        enum RegisterState
        {
            RS_NO_INFO = 0,
            RS_JOIN = 1,
            RS_PRUNE = 2,
            RS_JOIN_PENDING = 3
        };

        /** States of each outgoing interface. */
        enum InterfaceState
        {
            Forward,
            Pruned
        };

        /** Assert States of each outgoing interface. */
        enum AssertState
        {
            AS_NO_INFO = 0,
            AS_WINNER = 1,
            AS_LOSER = 2
        };

        struct PIMSMMulticastRoute;

        struct Interface
        {
            PIMSMMulticastRoute *owner;
            InterfaceEntry *ie;
            cMessage *expiryTimer;

            Interface(PIMSMMulticastRoute *owner, InterfaceEntry *ie) : owner(owner), ie(ie), expiryTimer(NULL)
            {
                ASSERT(owner);
                ASSERT(ie);
            }
            virtual ~Interface()
            {
                owner->owner->cancelAndDelete(expiryTimer); expiryTimer = NULL;
            }
            void startExpiryTimer(double holdTime);
        };

        /**
         * @brief Structure of incoming interface.
         * @details E.g.: GigabitEthernet1/4, RPF nbr 10.10.51.145
         */
        struct UpstreamInterface : public Interface
        {
            IPv4Address nextHop;            /**< RF neighbor */

            UpstreamInterface(PIMSMMulticastRoute *owner, InterfaceEntry *ie, IPv4Address nextHop)
                : Interface(owner, ie), nextHop(nextHop) {}
            int getInterfaceId() const { return ie->getInterfaceId(); }
        };

        struct DownstreamInterface : public Interface
        {
            InterfaceState          forwarding;         /**< Forward or Pruned */
            PIMInterface::PIMMode   mode;               /**< Dense, Sparse, ... */
            AssertState             assert;             /**< Assert state. */

            RegisterState           regState;           /**< Register state. */
            bool                    shRegTun;           /**< Show interface which is also register tunnel interface*/

            DownstreamInterface(PIMSMMulticastRoute *owner, InterfaceEntry *ie, InterfaceState forwarding)
                : Interface(owner, ie), forwarding(forwarding), mode(PIMInterface::SparseMode), assert(AS_NO_INFO),
                  regState(RS_NO_INFO), shRegTun(true) {}

            DownstreamInterface(PIMSMMulticastRoute *owner, InterfaceEntry *ie, InterfaceState forwarding, RegisterState regState, bool show)
                : Interface(owner, ie), forwarding(forwarding), mode(PIMInterface::SparseMode), assert(AS_NO_INFO),
                  regState(regState), shRegTun(show) {}

            int getInterfaceId() const { return ie->getInterfaceId(); }
            virtual bool isEnabled() { return forwarding != Pruned; } // XXX should be: ((has neighbor and not pruned) or has listener) and not assert looser
        };

        typedef std::vector<DownstreamInterface*> DownstreamInterfaceVector;

        class PIMSMOutInterface : public IMulticastRoute::OutInterface
        {
            DownstreamInterface *downstream;
            public:
                PIMSMOutInterface(DownstreamInterface *downstream)
                    : OutInterface(downstream->ie), downstream(downstream) {}
                virtual bool isEnabled() { return downstream->isEnabled(); }
        };

        enum JPMsgType
        {
            G,
            SG,
            SGrpt
        };

        // Holds (*,G), (S,G) or (S,G,rpt) state
        struct PIMSMMulticastRoute
        {
                /** Route flags. Added to each route. */
                enum Flag
                {
                    NO_FLAG = 0,
                    D       = 0x01,              /**< Dense */
                    S       = 0x02,              /**< Sparse */
                    C       = 0x04,              /**< Connected */ // XXX Are there any connected downstream receivers?
                    P       = 0x08,              /**< Pruned */
                    A       = 0x10,              /**< Source is directly connected */
                    F       = 0x20,              /**< Register flag*/
                    T       = 0x40               /**< SPT bit*/
                };

                PIMSM *owner;
                IPv4Address origin; // <unspec> in (*,G) routes
                IPv4Address group;
                IPv4Address rpAddr;                     /**< Randevous point */
                int flags;

                //Originated from destination.Ensures loop freeness.
                unsigned int sequencenumber;
                //Time of routing table entry creation
                simtime_t installtime; // XXX not used

                cMessage *keepAliveTimer;
                cMessage *registerStopTimer;
                cMessage *joinTimer;
                cMessage *prunePendingTimer;

                UpstreamInterface *upstreamInterface;      // may be NULL at RP and at DR
                DownstreamInterfaceVector downstreamInterfaces; ///< Out interfaces (downstream)

            public:
                PIMSMMulticastRoute(PIMSM *owner, IPv4Address origin, IPv4Address group);
                ~PIMSMMulticastRoute();
                virtual std::string info() const;

                void clearDownstreamInterfaces();
                void addDownstreamInterface(DownstreamInterface *outInterface);
                void removeDownstreamInterface(unsigned int i);

                bool isFlagSet(Flag flag) const { return (flags & flag) != 0; }     /**< Returns if flag is set to entry or not*/
                void setFlags(int flags)   { this->flags |= flags; }                /**< Add flag to ineterface */
                void clearFlag(Flag flag)  { flags &= (~flag); }                   /**< Remove flag from ineterface */
                static std::string flagsToString(int flags);

                DownstreamInterface *findDownstreamInterfaceByInterfaceId(int interfaceId);
                bool isOilistNull();                                                /**< Returns true if list of outgoing interfaces is empty, otherwise false*/

                void startKeepAliveTimer();
                void startRegisterStopTimer();
                void startJoinTimer();
                void startPrunePendingTimer();
        };

        typedef std::map<SourceAndGroup, PIMSMMulticastRoute*> SGStateMap;


        IPv4Address rpAddr;
        std::string sptThreshold;
        SGStateMap routes;

    private:
        void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
        void newMulticastRegisterDR(IPv4Address srcAddr, IPv4Address destAddr);
        void newMulticastReceiver(PIMInterface *pimInterface, IPv4Address multicastGroup);
        void removeMulticastReceiver(PIMInterface *pimInterface, IPv4Address multicastGroup);


        // process timers
        void processPIMTimer(cMessage *timer);
        void processKeepAliveTimer(cMessage *timer);
        void processRegisterStopTimer(cMessage *timer);
        void processExpiryTimer(cMessage *timer);
        void processJoinTimer(cMessage *timer);
        void processPrunePendingTimer(cMessage *timer);


        void restartExpiryTimer(PIMSMMulticastRoute *route, InterfaceEntry *originIntf, int holdTime);
        void dataOnRpf(PIMSMMulticastRoute *route);

        // pim messages
        void sendPIMRegister(IPv4Datagram *datagram);
        void sendPIMRegisterStop(IPv4Address source, IPv4Address dest, IPv4Address multGroup, IPv4Address multSource);
        void sendPIMRegisterNull(IPv4Address multSource, IPv4Address multDest);
        void sendPIMJoin(IPv4Address group, IPv4Address source, IPv4Address upstreamNeighbor, JPMsgType JPtype);
        void sendPIMPrune(IPv4Address group, IPv4Address source, IPv4Address upstreamNeighbor, JPMsgType JPtype);
        void sendToIP(PIMPacket *packet, IPv4Address source, IPv4Address dest, int outInterfaceId, short ttl);
        void forwardMulticastData(IPv4Datagram *datagram, int outInterfaceId);

        // process PIM messages
        void processPIMPkt(PIMPacket *pkt);
        void processRegisterPacket(PIMRegister *pkt);
        void processRegisterStopPacket(PIMRegisterStop *pkt);
        void processJoinPacket(PIMJoinPrune *pkt, IPv4Address multGroup, EncodedAddress encodedAddr);
        void processPrunePacket(PIMJoinPrune *pkt, IPv4Address multGroup, EncodedAddress encodedAddr);
        void processJoinPrunePacket(PIMJoinPrune *pkt);
        void processSGJoin(PIMJoinPrune *pkt,IPv4Address multOrigin, IPv4Address multGroup);
        void processJoinRouteGexistOnRP(IPv4Address multGroup, IPv4Address packetOrigin, int msgHoldtime);

        // helpers
        PIMInterface *getIncomingInterface(IPv4Datagram *datagram);
        bool deleteMulticastRoute(PIMSMMulticastRoute *route);
        void cancelAndDeleteTimer(cMessage *&timer);
        void restartTimer(cMessage *timer, double interval);

        // routing table access
        PIMSMMulticastRoute *getRouteFor(IPv4Address group, IPv4Address source);
        std::vector<PIMSMMulticastRoute*> getRouteFor(IPv4Address group);
        void addGRoute(PIMSMMulticastRoute *route);
        void addSGRoute(PIMSMMulticastRoute *route);
        bool removeRoute(PIMSMMulticastRoute *route);
        PIMSMMulticastRoute *findGRoute(IPv4Address group);
        PIMSMMulticastRoute *findSGRoute(IPv4Address source, IPv4Address group);
        IPv4MulticastRoute *createMulticastRoute(PIMSMMulticastRoute *route);
        IPv4MulticastRoute *findIPv4Route(IPv4Address source, IPv4Address group);

    public:
        PIMSM() : PIMBase(PIMInterface::SparseMode) {}
        ~PIMSM();
        //PIM-SM clear implementation
        void setRPAddress(std::string address);
        void setSPTthreshold(std::string address);
        IPv4Address getRPAddress () {return rpAddr;}
        std::string getSPTthreshold () {return sptThreshold;}
        virtual bool IamRP (IPv4Address RPaddress);
        bool IamDR (IPv4Address sourceAddr);

	protected:
		virtual int numInitStages() const  {return NUM_INIT_STAGES;}
		virtual void handleMessage(cMessage *msg);
		virtual void initialize(int stage);
};

#endif
