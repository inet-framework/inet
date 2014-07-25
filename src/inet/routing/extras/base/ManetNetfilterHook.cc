//
// Copyright (C) 2012 OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
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
// @author: Zoltan Bojthe

#include "inet/routing/extras/base/ManetNetfilterHook.h"

#include "inet/routing/extras/base/ControlManetRouting_m.h"
#include "inet/networklayer/common/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/IRoutingTable.h"
#include "inet/networklayer/ipv4/IPv4.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"
#include "inet/common/ProtocolMap.h"
#include "inet/common/ModuleAccess.h"

#include "inet/routing/extras/dsr/dsr-pkt_omnet.h"

namespace inet {

namespace inetmanet {

void ManetNetfilterHook::initHook(cModule* _module)
{
    module = _module;
    ipLayer = check_and_cast<IPv4*>(findModuleWhereverInNode("ip", module));
    cProperties *props = module->getProperties();
    isReactive = props && props->getAsBool("reactive");

    ipLayer->registerHook(0, this);
}

void ManetNetfilterHook::finishHook()
{
    ipLayer->unregisterHook(0, this);
}

INetfilter::IHook::Result ManetNetfilterHook::datagramPreRoutingHook(INetworkDatagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, L3Address& nextHopAddr)
{
    if (isReactive)
    {
        if (!inIE->isLoopback() && !datagram->getDestinationAddress().isMulticast())
            sendRouteUpdateMessageToManet(datagram);

        if (checkPacketUnroutable(datagram, NULL))
        {
            delete dynamic_cast<cPacket *>(datagram)->removeControlInfo();
            sendNoRouteMessageToManet(datagram);
            return INetfilter::IHook::STOLEN;
        }
    }

    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result ManetNetfilterHook::datagramForwardHook(INetworkDatagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, L3Address& nextHopAddr)
{
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result ManetNetfilterHook::datagramPostRoutingHook(INetworkDatagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, L3Address& nextHopAddr)
{
    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result ManetNetfilterHook::datagramLocalInHook(INetworkDatagram* datagram, const InterfaceEntry* inIE)
{
    if (isReactive)
    {
        if (datagram->getTransportProtocol() == IP_PROT_DSR)
        {
            sendToManet(dynamic_cast<cPacket *>(datagram));
            return INetfilter::IHook::STOLEN;
        }
    }

    return INetfilter::IHook::ACCEPT;
}

INetfilter::IHook::Result ManetNetfilterHook::datagramLocalOutHook(INetworkDatagram* datagram, const InterfaceEntry*& outIE, L3Address& nextHopAddr)
{
    if (isReactive)
    {
        cPacket * packet = dynamic_cast<cPacket *>(datagram);
        // Dsr routing, Dsr is a HL protocol and send datagram
        if (datagram->getTransportProtocol()==IP_PROT_DSR)
        {
            INetworkProtocolControlInfo *controlInfo = check_and_cast<INetworkProtocolControlInfo*>(packet->getControlInfo());
            DSRPkt *dsrpkt = check_and_cast<DSRPkt *>(packet);
            outIE = ift->getInterfaceById(controlInfo->getInterfaceId());
            nextHopAddr = dsrpkt->nextAddress();
        }

        sendRouteUpdateMessageToManet(datagram);

        if (checkPacketUnroutable(datagram, outIE))
        {
            delete packet->removeControlInfo();
            sendNoRouteMessageToManet(datagram);
            return INetfilter::IHook::STOLEN;
        }
    }
    return INetfilter::IHook::ACCEPT;
}

// Helper functions:

void ManetNetfilterHook::sendRouteUpdateMessageToManet(INetworkDatagram *datagram)
{
    if (datagram->getTransportProtocol() != IP_PROT_DSR) // Dsr don't use update code, the Dsr datagram is the update.
    {
        ControlManetRouting *control = new ControlManetRouting();
        control->setOptionCode(MANET_ROUTE_UPDATE);
        control->setSrcAddress(datagram->getSourceAddress());
        control->setDestAddress(datagram->getDestinationAddress());
        sendToManet(control);
    }
}

void ManetNetfilterHook::sendNoRouteMessageToManet(INetworkDatagram *datagram)
{
    if (datagram->getTransportProtocol()==IP_PROT_DSR)
    {
        sendToManet(dynamic_cast<cPacket *>(datagram));
    }
    else
    {
        ControlManetRouting *control = new ControlManetRouting();
        control->setOptionCode(MANET_ROUTE_NOROUTE);
        control->setSrcAddress(datagram->getSourceAddress());
        control->setDestAddress(datagram->getDestinationAddress());
        control->encapsulate(dynamic_cast<cPacket *>(datagram));
        sendToManet(control);
    }
}

void ManetNetfilterHook::sendToManet(cPacket *packet)
{
    ipLayer->sendOnTransportOutGateByProtocolId(packet, IP_PROT_MANET);
}

bool ManetNetfilterHook::checkPacketUnroutable(INetworkDatagram* datagram, const InterfaceEntry* outIE)
{
    if (outIE != NULL)
        return false;

    L3Address destAddr = datagram->getDestinationAddress();

    if (destAddr.isMulticast() || destAddr.isBroadcast())
        return false;

    if (rt->isLocalAddress(destAddr))
        return false;

    return (rt->findBestMatchingRoute(destAddr) == NULL);
}

} // namespace inetmanet

} // namespace inet

