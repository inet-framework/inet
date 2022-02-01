//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_MESSAGEHANDLER_H
#define __INET_MESSAGEHANDLER_H

#include "inet/routing/ospfv2/interface/Ospfv2Interface.h"
#include "inet/routing/ospfv2/messagehandler/DatabaseDescriptionHandler.h"
#include "inet/routing/ospfv2/messagehandler/HelloHandler.h"
#include "inet/routing/ospfv2/messagehandler/IMessageHandler.h"
#include "inet/routing/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h"
#include "inet/routing/ospfv2/messagehandler/LinkStateRequestHandler.h"
#include "inet/routing/ospfv2/messagehandler/LinkStateUpdateHandler.h"

namespace inet {
namespace ospfv2 {

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

    void processPacket(Packet *packet, Ospfv2Interface *unused1 = nullptr, Neighbor *unused2 = nullptr) override;

    void sendPacket(Packet *packet, Ipv4Address destination, Ospfv2Interface *outputIf, short ttl = 1);
    void clearTimer(cMessage *timer);
    void startTimer(cMessage *timer, simtime_t delay);

    void printEvent(const char *eventString, const Ospfv2Interface *onInterface = nullptr, const Neighbor *forNeighbor = nullptr) const;
    void printHelloPacket(const Ospfv2HelloPacket *helloPacket, Ipv4Address destination, int outputIfIndex) const;
    void printDatabaseDescriptionPacket(const Ospfv2DatabaseDescriptionPacket *ddPacket, Ipv4Address destination, int outputIfIndex) const;
    void printLinkStateRequestPacket(const Ospfv2LinkStateRequestPacket *requestPacket, Ipv4Address destination, int outputIfIndex) const;
    void printLinkStateUpdatePacket(const Ospfv2LinkStateUpdatePacket *updatePacket, Ipv4Address destination, int outputIfIndex) const;
    void printLinkStateAcknowledgementPacket(const Ospfv2LinkStateAcknowledgementPacket *ackPacket, Ipv4Address destination, int outputIfIndex) const;

    // Authentication not implemented
    bool authenticatePacket(const Ospfv2Packet *packet) { return true; }
};

} // namespace ospfv2
} // namespace inet

#endif

