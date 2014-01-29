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
#include "IPv4RoutingTableAccess.h"
#include "PIMBase.h"

using namespace std;

const IPv4Address PIMBase::ALL_PIM_ROUTERS_MCAST("224.0.0.13");

const PIMBase::AssertMetric PIMBase::AssertMetric::INFINITE;

bool PIMBase::AssertMetric::operator==(const AssertMetric& other) const
{
    return rptBit == other.rptBit && preference == other.preference &&
            metric == other.metric && address == other.address;
}

bool PIMBase::AssertMetric::operator!=(const AssertMetric& other) const
{
    return rptBit != other.rptBit || preference != other.preference ||
            metric != other.metric || address != other.address;
}

bool PIMBase::AssertMetric::operator<(const AssertMetric& other) const
{
    if (isInfinite())
        return false;
    if (other.isInfinite())
        return true;
    if (rptBit != other.rptBit)
        return rptBit < other.rptBit;
    if (preference != other.preference)
        return preference < other.preference;
    if (metric != other.metric)
        return metric < other.metric;
    return address > other.address;
}

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
        rt = IPv4RoutingTableAccess().get();
        pimIft = check_and_cast<PIMInterfaceTable*>(getModuleByPath(par("pimInterfaceTableModule")));
        pimNbt = check_and_cast<PIMNeighborTable*>(getModuleByPath(par("pimNeighborTableModule")));

        cModule *host = findContainingNode(this);
        if (!host)
            throw cRuntimeError("PIMBase: containing node not found.");

        hostname = host->getName();

        helloPeriod = par("helloPeriod");
    }
    else if (stage == INITSTAGE_TRANSPORT_LAYER)
    {
        IPSocket ipSocket(gate("ipOut"));
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

            helloTimer = new cMessage("PIM HelloTimer", HelloTimer);
            scheduleAt(simTime() + par("triggeredHelloDelay").doubleValue(), helloTimer);
        }
    }
}

void PIMBase::processHelloTimer(cMessage *timer)
{
    ASSERT(timer == helloTimer);
    EV_DETAIL << "Hello Timer expired.\n";
    sendHelloPackets();
    scheduleAt(simTime() + helloPeriod, helloTimer);
}

void PIMBase::sendHelloPackets()
{
    for (int i = 0; i < pimIft->getNumInterfaces(); i++)
    {
        PIMInterface *pimInterface = pimIft->getInterface(i);
        if (pimInterface->getMode() == mode)
            sendHelloPacket(pimInterface);
    }
}

void PIMBase::sendHelloPacket(PIMInterface *pimInterface)
{
    EV_INFO << "Sending Hello packet on interface '" << pimInterface->getInterfacePtr()->getName() << "'\n";

    PIMHello *msg = new PIMHello("PIMHello");
    IPv4ControlInfo *ctrl = new IPv4ControlInfo();
    ctrl->setDestAddr(ALL_PIM_ROUTERS_MCAST);
    ctrl->setProtocol(IP_PROT_PIM);
    ctrl->setTimeToLive(1);
    ctrl->setInterfaceId(pimInterface->getInterfaceId());
    msg->setControlInfo(ctrl);

    send(msg, "ipOut");
}

void PIMBase::processHelloPacket(PIMHello *packet)
{
    IPv4ControlInfo *ctrl = dynamic_cast<IPv4ControlInfo *>(packet->getControlInfo());
    int interfaceId = ctrl->getInterfaceId();
    IPv4Address address = ctrl->getSrcAddr();
    int version = packet->getVersion();

    InterfaceEntry *ie = ift->getInterfaceById(interfaceId);

    EV_INFO << "Received PIM Hello from neighbor: interface=" << ie->getName() << " address=" << address << "\n";

    PIMNeighbor *neighbor = pimNbt->findNeighbor(interfaceId, address);
    if (neighbor)
        pimNbt->restartLivenessTimer(neighbor);
    else
    {
        pimNbt->addNeighbor(new PIMNeighbor(ie, address, version));

        // TODO If a Hello message is received from a new neighbor, the
        // receiving router SHOULD send its own Hello message after a random
        // delay between 0 and Triggered_Hello_Delay.
    }

    delete packet;
}
