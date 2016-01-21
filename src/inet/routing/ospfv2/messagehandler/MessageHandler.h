//
// Copyright (C) 2006 Andras Babos and Andras Varga
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

#ifndef __INET_MESSAGEHANDLER_H
#define __INET_MESSAGEHANDLER_H

#include "inet/common/INETDefs.h"
#include "inet/routing/ospfv2/messagehandler/DatabaseDescriptionHandler.h"
#include "inet/routing/ospfv2/messagehandler/HelloHandler.h"
#include "inet/routing/ospfv2/messagehandler/IMessageHandler.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/routing/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h"
#include "inet/routing/ospfv2/messagehandler/LinkStateRequestHandler.h"
#include "inet/routing/ospfv2/messagehandler/LinkStateUpdateHandler.h"
#include "inet/routing/ospfv2/interface/OSPFInterface.h"
#include "inet/routing/ospfv2/OSPFTimer.h"

namespace inet {

namespace ospf {

class INET_API MessageHandler : public IMessageHandler
{
  private:
    cSimpleModule *ospfModule;

    HelloHandler helloHandler;
    DatabaseDescriptionHandler ddHandler;
    LinkStateRequestHandler lsRequestHandler;
    LinkStateUpdateHandler lsUpdateHandler;
    LinkStateAcknowledgementHandler lsAckHandler;

  public:
    MessageHandler(Router *containingRouter, cSimpleModule *containingModule);

    void messageReceived(cMessage *message);
    void handleTimer(cMessage *timer);

    void processPacket(OSPFPacket *packet, Interface *unused1 = nullptr, Neighbor *unused2 = nullptr) override;

    void sendPacket(OSPFPacket *packet, IPv4Address destination, int outputIfIndex, short ttl = 1);
    void clearTimer(cMessage *timer);
    void startTimer(cMessage *timer, simtime_t delay);

    void printEvent(const char *eventString, const Interface *onInterface = nullptr, const Neighbor *forNeighbor = nullptr) const;
    void printHelloPacket(const OSPFHelloPacket *helloPacket, IPv4Address destination, int outputIfIndex) const;
    void printDatabaseDescriptionPacket(const OSPFDatabaseDescriptionPacket *ddPacket, IPv4Address destination, int outputIfIndex) const;
    void printLinkStateRequestPacket(const OSPFLinkStateRequestPacket *requestPacket, IPv4Address destination, int outputIfIndex) const;
    void printLinkStateUpdatePacket(const OSPFLinkStateUpdatePacket *updatePacket, IPv4Address destination, int outputIfIndex) const;
    void printLinkStateAcknowledgementPacket(const OSPFLinkStateAcknowledgementPacket *ackPacket, IPv4Address destination, int outputIfIndex) const;

    // Authentication not implemented
    bool authenticatePacket(OSPFPacket *packet) { return true; }
};

} // namespace ospf

} // namespace inet

#endif // ifndef __INET_MESSAGEHANDLER_H

