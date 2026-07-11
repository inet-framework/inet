//
// Copyright (C) 2005 Vojtech Janota
// Copyright (C) 2003 Xuan Thang Nguyen
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_MPLS_H
#define __INET_MPLS_H

#include "inet/common/SimpleModule.h"
#include <vector>

#include "inet/common/IInterfaceRegistrationListener.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/mpls/IIngressClassifier.h"
#include "inet/networklayer/mpls/LibTable.h"
#include "inet/networklayer/mpls/MplsPacket_m.h"

namespace inet {

/**
 * Implements the MPLS protocol; see the NED file for more info.
 */
class INET_API Mpls : public SimpleModule, public DefaultProtocolRegistrationListener, public IInterfaceRegistrationListener
{
  public:
    // RFC 3443 TTL propagation model between the IP and MPLS layers
    enum TtlModel { TTL_MODEL_UNIFORM, TTL_MODEL_PIPE };

  protected:
    long numSent = 0;
    long numReceived = 0;

    TtlModel ttlModel = TTL_MODEL_UNIFORM;
    int defaultTtl = 255;
    bool writeTcBackOnPop = false;

    ModuleRefByPar<LibTable> lt;
    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<IIngressClassifier> pct;

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;

  protected:
    virtual void processPacketFromL3(Packet *msg);
    virtual void processPacketFromL2(Packet *msg);
    virtual void processMplsPacketFromL2(Packet *mplsPacket);

    virtual bool tryLabelAndForwardIpv4Datagram(Packet *ipdatagram);
    virtual void labelAndForwardIpv4Datagram(Packet *ipdatagram);

    virtual void sendToL2(Packet *msg);
    virtual void sendToL3(Packet *msg);

    void pushLabel(Packet *packet, Ptr<MplsHeader>& newMplsHeader);
    void swapLabel(Packet *packet, Ptr<MplsHeader>& newMplsHeader);

    // payloadProtocol is the protocol to stamp on the packet's PacketProtocolTag once
    // the bottom-of-stack label is popped (Workstream F3, IPv6): defaults to Ipv4,
    // matching every caller that pre-dates per-LIB-entry payload protocols.
    void popLabel(Packet *packet, const Protocol *payloadProtocol = &Protocol::ipv4);

    // returns the TTL to stamp on a freshly pushed label: the IP TTL (uniform) or
    // defaultTtl (pipe) when pushing onto bare IP, or the current outer label's TTL
    // when pushing onto an existing label stack
    uint8_t computePushTtl(const Packet *packet) const;

    // returns the Traffic Class to stamp on a freshly pushed label: the top 3 bits
    // of the IP header's DSCP when pushing onto bare IP, or the current outer
    // label's tc when pushing onto an existing label stack
    uint8_t computePushTc(const Packet *packet) const;

    // RFC 3443: the label TTL reached zero at a transit LSR; pop the entire label
    // stack, write the expired TTL back into the IP header, and hand the datagram
    // to L3 so that Ipv4's own hop-count check generates the ICMP Time Exceeded
    void handleTtlExpiry(Packet *packet, int outInterfaceId);

    // hands a packet that was label-switched off the packet's Ipv4 PacketProtocolTag
    // up to L3 as if it had just arrived from the network: replaces the stale
    // DispatchProtocolReq left over from being dispatched here as an MPLS packet
    // (otherwise the dispatcher between Mpls and Ipv4 loops it straight back here)
    void deliverToL3(Packet *packet);

    // returns false if the packet was already fully handled (TTL expiry or a
    // reserved label) and the caller must not touch it any further.
    // payloadProtocol (Workstream F3, IPv6) is forwarded to popLabel() for the
    // POP_OPER case; defaults to Ipv4 since the ingress call site
    // (tryLabelAndForwardIpv4Datagram()) has no LIB entry to ask.
    virtual bool doStackOps(Packet *packet, const LabelOpVector& outLabel, int outInterfaceId,
            const Protocol *payloadProtocol = &Protocol::ipv4);

    // IInterfaceRegistrationListener:
    virtual void handleRegisterInterface(const NetworkInterface& interface, cGate *in, cGate *out) override;

    // IProtocolRegistrationListener:
    virtual void handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;
};

} // namespace inet

#endif

