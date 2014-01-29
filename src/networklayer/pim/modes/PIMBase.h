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

#ifndef __INET_PIMBASE_H
#define __INET_PIMBASE_H

#include "INETDefs.h"

#include "IInterfaceTable.h"
#include "IIPv4RoutingTable.h"
#include "PIMNeighborTable.h"
#include "PIMInterfaceTable.h"
#include "PIMPacket_m.h"


/**
 * TODO
 */
class INET_API PIMBase : public cSimpleModule
{
    protected:

        struct AssertMetric
        {
            short rptBit;
            short preference;
            int metric;
            IPv4Address address;

            static const AssertMetric INFINITE;

            AssertMetric() : rptBit(1), preference(-1), metric(0) {}
            AssertMetric(int preference, int metric) : rptBit(0), preference(preference), metric(metric) { ASSERT(preference >= 0); }
            // AssertMetric(bool rptBit, int preference, int metric) : rptBit(rptBit?1:0), preference(preference), metric(metric) { ASSERT(preference >= 0); }
            AssertMetric(bool rptBit, int preference, int metric, IPv4Address address = IPv4Address::UNSPECIFIED_ADDRESS)
                : rptBit(rptBit?1:0), preference(preference), metric(metric), address(address) { ASSERT(preference >= 0); }
            bool isInfinite() const { return preference == -1; }
            bool operator==(const AssertMetric& other) const;
            bool operator!=(const AssertMetric& other) const;
            bool operator<(const AssertMetric& other) const;
            AssertMetric setAddress(IPv4Address address) const { return AssertMetric(rptBit, preference, metric, address); }
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

        enum PIMTimerKind
        {
            // global timers
           HelloTimer = 1,
           TriggeredHelloDelay,

           // timers for each interface and each source-group pair (S,G,I)
           AssertTimer,
           PruneTimer,
           PrunePendingTimer,

           // timers for each source-group pair (S,G)
           GraftRetryTimer,
           UpstreamOverrideTimer,
           PruneLimitTimer,
           SourceActiveTimer,
           StateRefreshTimer,

           //PIM-SM specific timers
           KeepAliveTimer,
           RegisterStopTimer,
           ExpiryTimer,
           JoinTimer,
        };

        static const IPv4Address ALL_PIM_ROUTERS_MCAST;

    protected:
        IIPv4RoutingTable *rt;
        IInterfaceTable *ift;
        PIMInterfaceTable *pimIft;
        PIMNeighborTable *pimNbt;

        const char *                hostname;

        // parameters
        double helloPeriod;

        PIMInterface::PIMMode mode;
        cMessage *helloTimer;

    public:
        PIMBase(PIMInterface::PIMMode mode) : mode(mode), helloTimer(NULL) {}
        virtual ~PIMBase();

    protected:
        virtual int numInitStages() const  {return NUM_INIT_STAGES;}
        virtual void initialize(int stage);

        void sendHelloPackets();
        void sendHelloPacket(PIMInterface *pimInterface);
        void processHelloTimer(cMessage *timer);
        void processHelloPacket(PIMHello *pkt);
};


#endif
