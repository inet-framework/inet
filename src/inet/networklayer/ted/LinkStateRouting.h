//
// (C) 2005 Vojtech Janota
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#ifndef __INET_LINKSTATEROUTING_H
#define __INET_LINKSTATEROUTING_H

#include "inet/common/INETDefs.h"

#include "inet/networklayer/ted/LinkStatePacket_m.h"
#include "inet/networklayer/rsvp_te/IntServ.h"

namespace inet {

#define TED_TRAFFIC    1

class TED;
class IIPv4RoutingTable;
class IInterfaceTable;
class InterfaceEntry;

/**
 * Implements a minimalistic link state routing protocol that employs flooding.
 * Flooding works like this:
 *
 * When a router receives a link state packet, it merges the packet contents
 * into its own link state database (ted). If the packet contained new information
 * (ted got updated), the router broadcasts the ted contents to all its other
 * neighbours; otherwise (when the packet didn't contain any new info), nothing
 * happens.
 *
 * Also: when the announceMsg timer expires, LinkStateRouting sends out an initial link state
 * message. (Currently this happens only once, at the beginning of the simulation).
 * The "request" bit in the message is set then, asking neighbours to send back
 * their link state databases. (FIXME why's this? redundant messaging: same msg
 * is often sent twice: both as reply and as voluntary "announce").
 *
 * TODO discover peers by "hello". Peers are those from which the router has received
 * a Hello in the last X seconds. Link info to all peers are maintained;
 * links to ex-peers (those haven't heard of for more than X seconds) are assumed
 * to be down.
 *
 * See NED file for more info.
 */
class LinkStateRouting : public cSimpleModule, public cListener
{
  protected:
    TED *tedmod = nullptr;
    cMessage *announceMsg = nullptr;
    IPv4Address routerId;

    IPAddressVector peerIfAddrs;    // addresses of interfaces towards neighbouring routers

  public:
    LinkStateRouting();
    virtual ~LinkStateRouting();

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;

    virtual void processLINK_STATE_MESSAGE(LinkStateMsg *msg, IPv4Address sender);

    // cListener method
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj) override;

    virtual void sendToPeers(const std::vector<TELinkStateInfo>& list, bool req, IPv4Address exceptPeer);
    virtual void sendToPeer(IPv4Address peer, const std::vector<TELinkStateInfo>& list, bool req);
    virtual void sendToIP(LinkStateMsg *msg, IPv4Address destAddr);
};

} // namespace inet

#endif // ifndef __INET_LINKSTATEROUTING_H

