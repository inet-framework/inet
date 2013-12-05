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

#include "IPv4Datagram.h"
#include "IPSocket.h"
#include "InterfaceTableAccess.h"
#include "IPv4ControlInfo.h"
#include "IPv4InterfaceData.h"
#include "InterfaceTableAccess.h"
#include "InterfaceTable.h"
#include "IPv4Address.h"
#include "PIMRoute.h"
#include "PIMRoutingTableAccess.h"
#include "PIMTimer_m.h"
#include "PIMBase.h"

using namespace std;

#define HT 30.0                                     /**< Hello Timer = 30s. */

const IPv4Address PIMBase::ALL_PIM_ROUTERS_MCAST("224.0.0.13");

PIMBase::~PIMBase()
{
    cancelAndDelete(helloTimer);
}

void PIMBase::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        ift = InterfaceTableAccess().get();
        rt = PIMRoutingTableAccess().get();
        pimIft = PIMInterfaceTableAccess().get();
        pimNbt = PIMNeighborTableAccess().get();

        cModule *host = findContainingNode(this);
        if (!host)
            throw cRuntimeError("PIMBase: containing node not found.");

        hostname = host->getName();
    }
    else if (stage == INITSTAGE_TRANSPORT_LAYER)
    {
        IPSocket ipSocket(gate("spiltterOut"));
        ipSocket.registerProtocol(IP_PROT_PIM);
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS)
    {
        // to receive PIM messages, join to ALL_PIM_ROUTERS multicast group
        int numInterfaces = 0;
        for (int i = 0; i < pimIft->getNumInterfaces(); i++)
        {
            PIMInterface *pimInterface = pimIft->getInterface(i);
            if (pimInterface->getMode() == mode)
            {
                pimInterface->getInterfacePtr()->ipv4Data()->joinMulticastGroup(ALL_PIM_ROUTERS_MCAST);
                numInterfaces++;
            }
        }

        if (numInterfaces > 0)
        {
            EV_INFO << "PIM is enabled on device " << hostname << endl;

            helloTimer = new PIMTimer("PIM Hello", HelloTimer);
            helloTimer->setTimerKind(HelloTimer);
            scheduleAt(simTime() + uniform(0,5), helloTimer);
        }
    }
}

void PIMBase::processHelloTimer(PIMTimer *timer)
{
    ASSERT(timer == helloTimer);
    sendHelloPackets();
    scheduleAt(simTime() + HT, helloTimer);
}

void PIMBase::sendHelloPackets()
{
    EV_INFO << "Sending hello packets\n";

    for (int i = 0; i < pimIft->getNumInterfaces(); i++)
    {
        PIMInterface *pimInterface = pimIft->getInterface(i);
        if (pimInterface->getMode() == mode)
        {
            PIMHello *msg = new PIMHello("PIMHello");
            IPv4ControlInfo *ctrl = new IPv4ControlInfo();
            ctrl->setDestAddr(ALL_PIM_ROUTERS_MCAST);
            ctrl->setProtocol(IP_PROT_PIM);
            ctrl->setTimeToLive(1);
            ctrl->setInterfaceId(pimInterface->getInterfaceId());
            msg->setControlInfo(ctrl);

            send(msg, "spiltterOut");
        }
    }
}

void PIMBase::processHelloPacket(PIMHello *packet)
{
    IPv4ControlInfo *ctrl = dynamic_cast<IPv4ControlInfo *>(packet->getControlInfo());
    InterfaceEntry *ie = ift->getInterfaceById(ctrl->getInterfaceId());
    IPv4Address address = ctrl->getSrcAddr();
    int version = packet->getVersion();

    EV_INFO << "Received PIM Hello from neighbor: interface=" << ie->getName() << " address=" << address << "\n";

    PIMNeighbor *neighbor = pimNbt->findNeighbor(ie->getInterfaceId(), address);
    if (neighbor)
        pimNbt->restartLivenessTimer(neighbor);
    else
        pimNbt->addNeighbor(new PIMNeighbor(ie, address, version));

    delete packet;
}

void PIMBase::newMulticastReceived(IPv4Address destAddr, IPv4Address srcAddr)
{
    EV << "PimSplitter::newMulticast - group: " << destAddr << ", source: " << srcAddr << endl;

    // find RPF interface for new multicast stream
    InterfaceEntry *inInt = rt->getInterfaceForDestAddr(srcAddr);
    if (inInt == NULL)
    {
        EV << "ERROR: PimSplitter::newMulticast(): cannot find RPF interface, routing information is missing.";
        return;
    }
    int rpfId = inInt->getInterfaceId();
    PIMInterface *pimInt = pimIft->getInterfaceById(rpfId);

    // if it is interface configured with PIM, create new route
    if (pimInt != NULL)
    {
        if (pimInt->getMode() != mode)
            return;

        // create new multicast route
        PIMMulticastRoute *newRoute = new PIMMulticastRoute();
        newRoute->setMulticastGroup(destAddr);
        newRoute->setOrigin(srcAddr);
        newRoute->setOriginNetmask(IPv4Address::ALLONES_ADDRESS);

        if (pimInt->getMode() == PIMInterface::DenseMode)
        {
            // Directly connected routes to source does not have next hop
            // RPF neighbor is source of packet
            IPv4Address rpf;
            const IPv4Route *routeToSrc = rt->findBestMatchingRoute(srcAddr);
            if (routeToSrc->getSourceType() == IPv4Route::IFACENETMASK)
            {
                newRoute->addFlag(PIMMulticastRoute::A);
                rpf = srcAddr;
            }
            // Not directly connected, next hop address is saved in routing table
            else
                rpf = rt->getGatewayForDestAddr(srcAddr);

            newRoute->setInInt(inInt, inInt->getInterfaceId(), rpf);

            emit(NF_IPv4_NEW_MULTICAST_DENSE, newRoute);
        }
        if (pimInt->getMode() == PIMInterface::SparseMode)
        {
            newRoute->setInInt(inInt, inInt->getInterfaceId(), IPv4Address("0.0.0.0"));
            emit(NF_IPv4_NEW_MULTICAST_SPARSE, newRoute);
        }
    }
}

