//
// Copyright (C) 2005 Vojtech Janota
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_LINKSTATEROUTING_H
#define __INET_LINKSTATEROUTING_H

#include "inet/common/SimpleModule.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/rsvpte/IntServ_m.h"
#include "inet/networklayer/ted/LinkStatePacket_m.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"

namespace inet {

using namespace inet::queueing;

#define TED_TRAFFIC    1

class Ted;
class IIpv4RoutingTable;
class IInterfaceTable;
class NetworkInterface;

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
class INET_API LinkStateRouting : public SimpleModule, public cListener, public IPassivePacketSink
{
  protected:
    ModuleRefByPar<Ted> tedmod;
    cMessage *announceMsg = nullptr;
    Ipv4Address routerId;
    PassivePacketSinkRef ipOutSink;

    Ipv4AddressVector peerIfAddrs; // addresses of interfaces towards neighbouring routers

  public:
    LinkStateRouting();
    virtual ~LinkStateRouting();

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;

    virtual void processLINK_STATE_MESSAGE(Packet *msg, Ipv4Address sender);

    // cListener method
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    virtual void sendToPeers(const std::vector<TeLinkStateInfo>& list, bool req, Ipv4Address exceptPeer);
    virtual void sendToPeer(Ipv4Address peer, const std::vector<TeLinkStateInfo>& list, bool req);
    virtual void sendToIP(Packet *msg, Ipv4Address destAddr);

  public:
    virtual bool canPushSomePacket(const cGate *gate) const override { return true; }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return true; }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }
};

} // namespace inet

#endif

