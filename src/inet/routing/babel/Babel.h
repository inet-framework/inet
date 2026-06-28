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
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/routing/babel/BabelBuffer.h"
#include "inet/routing/babel/BabelCost.h"
#include "inet/routing/babel/BabelInterfaceTable.h"
#include "inet/routing/babel/BabelMessage_m.h"
#include "inet/routing/babel/BabelNeighbourTable.h"
#include "inet/routing/babel/BabelPenSRTable.h"
#include "inet/routing/babel/BabelSourceTable.h"
#include "inet/routing/babel/BabelToAck.h"
#include "inet/routing/babel/BabelTopologyTable.h"
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
    ModuleRefByPar<IRoutingTable> rt4; ///< IPv4 routing table (may be absent)
    ModuleRefByPar<IRoutingTable> rt6; ///< IPv6 routing table (may be absent)

    // configuration
    int udpPort = defval::PORT;

    // state
    rid routerId;
    uint16_t seqno = 0;
    NetworkInterface *mainInterface = nullptr;
    UdpSocket socket;
    cMessage *triggeredUpdate = nullptr; ///< one-shot, damped triggered-update self message
    std::vector<BabelBuffer *> buffers; ///< per-destination output buffers
    cMessage *bufferGc = nullptr; ///< periodic garbage collection of idle buffers
    std::vector<BabelToAck *> ackwait; ///< messages sent reliably, awaiting acknowledgment

    BabelInterfaceTable bit;
    BabelNeighbourTable bnt;
    BabelTopologyTable btt;
    BabelSourceTable bst;
    BabelPenSRTable bpsrt;
    BabelCostKoutofj wiredCost; ///< link-cost strategy for wired links (k-out-of-j)
    BabelCostEtx wirelessCost; ///< link-cost strategy for wireless links (ETX)

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
    void processUpdateTimer(BabelInterface *iface);
    void processRouteExpiryTimer(BabelRoute *route);
    void processBefRouteExpiryTimer(BabelRoute *route);
    void processSourceGCTimer(BabelSource *source);
    void processRSResendTimer(BabelPenSR *request);

    // send (buffering layer + the actual packet send)
    void sendTLVs(const L3Address& dst, BabelInterface *iface, const std::vector<Ptr<BabelTlv>>& tlvs, double maxtime = SEND_URGENT, bool reliable = false);
    void sendBabelMessage(const L3Address& dst, BabelInterface *iface, const std::vector<Ptr<BabelTlv>>& tlvs);
    BabelBuffer *findOrCreateBuffer(const L3Address& dst, BabelInterface *iface);
    void flushBuffer(BabelBuffer *buff);
    void deleteUnusedBuffers();
    void deleteBuffers();
    void sendHello(BabelInterface *iface);
    void sendUpdateMessage(const L3Address& dst, BabelInterface *iface, const rid& originator, const L3Address& nexthop, const netPrefix<L3Address>& prefix, const routeDistance& dist, uint16_t interval, bool reliable = false);
    void sendUpdate(BabelInterface *iface, BabelRoute *route, const L3Address& dst, bool reliable = false);
    void sendUpdate(BabelInterface *iface, BabelRoute *route, bool reliable = false);
    void sendFullDump(BabelInterface *iface);
    void sendRouteReq(BabelInterface *iface, const L3Address& dst, int ae, const netPrefix<L3Address>& prefix);
    void triggerUpdate(); ///< schedule a damped triggered update to propagate a change quickly

    // receive
    void processUdpPacket(Packet *packet);
    void processHello(const Ptr<const BabelHelloTlv>& hello, BabelInterface *iface, const L3Address& src);
    void processIhu(const Ptr<const BabelIhuTlv>& ihu, BabelInterface *iface, const L3Address& src, const L3Address& dst);
    bool processUpdate(const Ptr<const BabelUpdateTlv>& update, BabelInterface *iface, const L3Address& src, const rid& originator, const L3Address& nexthop);
    void processRouteReq(const Ptr<const BabelRouteReqTlv>& req, BabelInterface *iface, const L3Address& src, const L3Address& dst);
    void processSeqnoReq(const Ptr<const BabelSeqnoReqTlv>& req, BabelInterface *iface, const L3Address& src);
    void processAckReq(const Ptr<const BabelAckReqTlv>& req, BabelInterface *iface, const L3Address& src);
    void processAck(const Ptr<const BabelAckTlv>& ack, const L3Address& src);

    // reliable delivery (acknowledgments)
    uint16_t generateNonce();
    BabelToAck *findToAck(uint16_t nonce);
    void deleteToAck(BabelToAck *toack);
    void checkAndResendToAck(BabelToAck *toack);
    void deleteToAcks();

    // seqno requests (loop-free recovery)
    void incSeqno();
    void sendSeqnoReq(BabelInterface *iface, const L3Address& dst, const netPrefix<L3Address>& prefix, const rid& orig, uint16_t reqSeqno, uint8_t hopcount, BabelNeighbour *recfrom);

    // route management
    void originateConnectedRoutes();
    bool isFeasible(const netPrefix<L3Address>& prefix, const rid& orig, const routeDistance& dist);
    bool addOrUpdateRoute(const netPrefix<L3Address>& prefix, BabelNeighbour *neigh, const rid& orig, const routeDistance& dist, const L3Address& nh, uint16_t interval);
    void addOrUpdateSource(const netPrefix<L3Address>& p, const rid& orig, const routeDistance& dist);
    void selectRoutes();
    bool removeRoutesByNeigh(BabelNeighbour *neigh);

    // routing-table install
    bool prepareToAdd(IRoutingTable *rt, IRoute *ro);
    void addToRT(BabelRoute *route);
    void removeFromRT(BabelRoute *route);
    void updateRT(BabelRoute *route);

    // helpers
    bool isMyAddressOnInterface(const L3Address& address, BabelInterface *iface) const;
    L3Address interfaceAddressForAf(BabelInterface *iface, int af) const;
    void multicastGroupsFor(BabelInterface *iface, std::vector<L3Address>& groups) const;

  public:
    Babel() {}
    virtual ~Babel();
};

} // namespace babel
} // namespace inet

#endif
