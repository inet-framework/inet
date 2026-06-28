//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_BABEL_H
#define __INET_BABEL_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/routing/babel/BabelCost.h"
#include "inet/routing/babel/BabelInterfaceTable.h"
#include "inet/routing/babel/BabelMessage_m.h"
#include "inet/routing/babel/BabelNeighbourTable.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {
namespace babel {

/**
 * Implementation of the Babel routing protocol (RFC 6126), ported from the
 * ANSAINET project to the modern INET packet (Chunk) API.
 *
 * A single Babel process is dual-stack and owns its databases as C++ member
 * objects. This phase implements neighbour discovery: per-interface Hello/IHU
 * exchange over UDP port 6696 and the resulting link-cost computation. The
 * route-selection algorithm and the data plane are added in later phases.
 */
class INET_API Babel : public RoutingProtocolBase, protected cListener
{
  protected:
    // environment
    cModule *host = nullptr;
    ModuleRefByPar<IInterfaceTable> ift;

    // configuration
    int udpPort = defval::PORT;

    // state
    rid routerId;
    uint16_t seqno = 0;
    NetworkInterface *mainInterface = nullptr;
    UdpSocket socket;

    BabelInterfaceTable bit;
    BabelNeighbourTable bnt;
    BabelCostKoutofj wiredCost; ///< default link-cost strategy for wired links

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    // setup
    void setMainInterface();
    void generateRouterId();
    void configureInterfaces();
    void activateInterface(BabelInterface *iface);
    void deactivateInterface(BabelInterface *iface);

    // timers
    cMessage *createTimer(short kind, void *context);
    void processTimer(cMessage *timer);
    void processNeighHelloTimer(BabelNeighbour *neigh);
    void processNeighIhuTimer(BabelNeighbour *neigh);

    // send
    void sendBabelMessage(const L3Address& dst, BabelInterface *iface, const std::vector<Ptr<BabelTlv>>& tlvs);
    void sendHello(BabelInterface *iface);

    // receive
    void processUdpPacket(Packet *packet);
    void processHello(const Ptr<const BabelHelloTlv>& hello, BabelInterface *iface, const L3Address& src);
    void processIhu(const Ptr<const BabelIhuTlv>& ihu, BabelInterface *iface, const L3Address& src, const L3Address& dst);

    // helpers
    bool isMyAddressOnInterface(const L3Address& address, BabelInterface *iface) const;

  public:
    Babel() {}
    virtual ~Babel();
};

} // namespace babel
} // namespace inet

#endif
