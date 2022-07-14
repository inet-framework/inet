//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV4DEFRAGMENTER_H
#define __INET_IPV4DEFRAGMENTER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/contract/IArp.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/Icmp.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

// fragmentAndSend
class INET_API Ipv4Fragmenter : public queueing::PacketPusherBase, public cListener
{
  protected:
    ModuleRefByPar<IArp> arp;
    ModuleRefByPar<Icmp> icmp;
    ModuleRefByPar<IInterfaceTable> ift;
    CrcMode crcMode = CRC_MODE_UNDEFINED;
    int defaultTimeToLive = -1;
    int defaultMCTimeToLive = -1;
    uint16_t curFragmentId = -1; // counter, used to assign unique fragmentIds to datagrams

    // ARP related
    typedef std::map<Ipv4Address, cPacketQueue> PendingPackets;
    PendingPackets pendingPackets; // map indexed with IPv4Address for outbound packets waiting for ARP resolution
  protected:
    virtual void initialize(int stage) override;
    virtual void pushPacket(Packet *packet, cGate *gate) override;

    void sendDatagramToOutput(Packet *packet);
    void sendPacketToNIC(Packet *packet);

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    // utility: processing requested ARP resolution completed
    void arpResolutionCompleted(IArp::Notification *entry);

    // utility: processing requested ARP resolution timed out
    void arpResolutionTimedOut(IArp::Notification *entry);

    MacAddress resolveNextHopMacAddress(cPacket *packet, Ipv4Address nextHopAddr, const NetworkInterface *destIE);
};

} // namespace inet

#endif
