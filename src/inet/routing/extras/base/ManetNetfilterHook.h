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

#ifndef __INET_MANETNETFILTERHOOK_H
#define __INET_MANETNETFILTERHOOK_H

#include "inet/networklayer/common/INetfilter.h"

#include <vector>
#include <set>

namespace inet {

class IPv4;
class IInterfaceTable;
class IRoutingTable;
class INetworkDatagram;
class InterfaceEntry;

namespace inetmanet {

class INET_API ManetNetfilterHook : public INetfilter::IHook
{
  protected:
    cModule* module;    // Manet module
    IPv4 *ipLayer;      // IPv4 module
    IInterfaceTable *ift;
    IRoutingTable *rt;
    bool isReactive;    // true if it's a reactive routing

  public:
    ManetNetfilterHook() : module(NULL), ipLayer(NULL), isReactive(false) {}

  protected:
    void initHook(cModule* module);
    void finishHook();

  protected:
    // Helper functions
    /**
     * Sends a MANET_ROUTE_UPDATE packet to Manet. The datagram is
     * not transmitted, only its source and destination address is used.
     * About DSR datagrams no update message is sent.
     */
    virtual void sendRouteUpdateMessageToManet(INetworkDatagram *datagram);

    /**
     * Sends a MANET_ROUTE_NOROUTE packet to Manet. The packet
     * will encapsulate the given datagram, so this method takes
     * ownership.
     * DSR datagrams are transmitted as they are, i.e. without
     * encapsulation. (?)
     */
    virtual void sendNoRouteMessageToManet(INetworkDatagram *datagram);

    /**
     * Sends a packet to the Manet module.
     */
    virtual void sendToManet(cPacket *packet);

    /**
     *
     */
    virtual bool checkPacketUnroutable(INetworkDatagram* datagram, const InterfaceEntry* outIE);

  public:
    virtual IHook::Result datagramPreRoutingHook(INetworkDatagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, L3Address& nextHopAddr);
    virtual IHook::Result datagramForwardHook(INetworkDatagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, L3Address& nextHopAddr);
    virtual IHook::Result datagramPostRoutingHook(INetworkDatagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, L3Address& nextHopAddr);
    virtual IHook::Result datagramLocalInHook(INetworkDatagram* datagram, const InterfaceEntry* inIE);
    virtual IHook::Result datagramLocalOutHook(INetworkDatagram* datagram, const InterfaceEntry*& outIE, L3Address& nextHopAddr);
};

} // namespace inetmanet

} // namespace inet

#endif
