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

#include "IInterfaceTable.h"
#include "IIPv4RoutingTable.h"
#include "PIMNeighborTable.h"
#include "PIMInterfaceTable.h"
#include "PIMMulticastRoute.h"
#include "PIMPacket_m.h"
#include "PIMTimer_m.h"


/**
 * TODO
 */
class PIMBase : public cSimpleModule
{
    protected:
        static const IPv4Address ALL_PIM_ROUTERS_MCAST;

    protected:
        IIPv4RoutingTable *rt;
        IInterfaceTable *ift;
        PIMInterfaceTable *pimIft;
        PIMNeighborTable *pimNbt;

        const char *                hostname;

        PIMInterface::PIMMode mode;
        cMessage *helloTimer;

    public:
        PIMBase(PIMInterface::PIMMode mode) : mode(mode), helloTimer(NULL) {}
        virtual ~PIMBase();

    protected:
        virtual int numInitStages() const  {return NUM_INIT_STAGES;}
        virtual void initialize(int stage);

        void sendHelloPackets();
        void processHelloTimer(cMessage *timer);
        void processHelloPacket(PIMHello *pkt);

        // routing table access
        PIMMulticastRoute *getRouteFor(IPv4Address group, IPv4Address source);
        std::vector<PIMMulticastRoute*> getRouteFor(IPv4Address group);
        std::vector<PIMMulticastRoute*> getRoutesForSource(IPv4Address source);
};


#endif
