//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPCHECKSUMINSERTIONHOOK_H
#define __INET_TCPCHECKSUMINSERTIONHOOK_H

#include "inet/common/Protocol.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/common/checksum/ChecksumMode_m.h"
#include "inet/transportlayer/tcp_common/TcpChecksumInsertionHook.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {
namespace tcp {

class INET_API TcpChecksumInsertionHook : public cSimpleModule, public NetfilterBase::HookBase
{
  public:
    static void insertChecksum(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<TcpHeader>& tcpHeader, Packet *tcpPayload);
    static uint16_t computeChecksum(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<const TcpHeader>& tcpHeader, const Ptr<const Chunk>& tcpData);

    virtual Result datagramPreRoutingHook(Packet *packet) override { return ACCEPT; }
    virtual Result datagramForwardHook(Packet *packet) override { return ACCEPT; }
    virtual Result datagramPostRoutingHook(Packet *packet) override;
    virtual Result datagramLocalInHook(Packet *packet) override { return ACCEPT; }
    virtual Result datagramLocalOutHook(Packet *packet) override { return ACCEPT; }
};

} // namespace tcp
} // namespace inet

#endif

