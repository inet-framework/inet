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

#include <omnetpp.h>
#include "NotificationBoard.h"
#include "LinkStatePacket_m.h"
#include "IntServ.h"


#define TED_TRAFFIC         1

class TED;
class IRoutingTable;
class IInterfaceTable;
class InterfaceEntry;
class NotificationBoard;


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
class LinkStateRouting : public cSimpleModule, public INotifiable
{
  protected:
    TED *tedmod;
    cMessage *announceMsg;
    IPAddress routerId;

    IPAddressVector peerIfAddrs; // addresses of interfaces towards neighbouring routers

  public:
    LinkStateRouting();
    virtual ~LinkStateRouting();

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const  {return 5;}
    virtual void handleMessage(cMessage *msg);

    virtual void processLINK_STATE_MESSAGE(LinkStateMsg* msg, IPAddress sender);

    // INotifiable method
    virtual void receiveChangeNotification(int category, const cPolymorphic *details);

    virtual void sendToPeers(const std::vector<TELinkStateInfo>& list, bool req, IPAddress exceptPeer);
    virtual void sendToPeer(IPAddress peer, const std::vector<TELinkStateInfo> & list, bool req);
    virtual void sendToIP(LinkStateMsg *msg, IPAddress destAddr);

};

#endif


