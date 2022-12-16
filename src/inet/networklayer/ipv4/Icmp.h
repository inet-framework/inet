//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_ICMP_H
#define __INET_ICMP_H

// Cleanup and rewrite: Andras Varga, 2004

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/IcmpHeader.h"
#include "inet/transportlayer/common/CrcMode_m.h"

namespace inet {

class Ipv4Header;

/**
 * Icmp module.
 */
class INET_API Icmp : public cSimpleModule, public DefaultProtocolRegistrationListener
{
  protected:
    std::set<int> transportProtocols; // where to send up packets
    CrcMode crcMode = CRC_MODE_UNDEFINED;
    B quoteLength;
    ModuleRefByPar<IIpv4RoutingTable> rt;
    ModuleRefByPar<IInterfaceTable> ift;
    uint64_t& ctr = SIMULATION_SHARED_COUNTER(ctr);

  protected:
    virtual void processIcmpMessage(Packet *);
    virtual void errorOut(Packet *);
    virtual void processEchoRequest(Packet *);
    virtual void sendToIP(Packet *, const Ipv4Address& dest);
    virtual void sendToIP(Packet *msg);
    virtual bool possiblyLocalBroadcast(const Ipv4Address& addr, int interfaceId);
    virtual void handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) override;

  public:
    /**
     * This method can be called from other modules to send an ICMP error packet
     * in response to a received bogus packet. It will not send ICMP error in response
     * to broadcast or multicast packets -- in that case it will simply delete the packet.
     * KLUDGE if inputInterfaceId cannot be determined, pass in -1.
     */
    virtual void sendErrorMessage(Packet *packet, int inputInterfaceId, IcmpType type, IcmpCode code);
    virtual void sendPtbMessage(Packet *packet, int mtu);
    static void insertCrc(CrcMode crcMode, const Ptr<IcmpHeader>& icmpHeader, Packet *payload);
    void insertCrc(const Ptr<IcmpHeader>& icmpHeader, Packet *payload) { insertCrc(crcMode, icmpHeader, payload); }
    bool verifyCrc(const Packet *packet);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void parseQuoteLengthParameter();
    virtual bool maySendErrorMessage(Packet *packet, int inputInterfaceId);
    virtual void sendOrProcessIcmpPacket(Packet *packet, Ipv4Address origSrcAddr);
};

} // namespace inet

#endif

