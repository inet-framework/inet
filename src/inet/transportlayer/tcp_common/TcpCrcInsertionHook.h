//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPCRCINSERTIONHOOK_H
#define __INET_TCPCRCINSERTIONHOOK_H

#include "inet/common/Protocol.h"
#include "inet/networklayer/contract/netfilter/NetfilterHookDefs_m.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/common/CrcMode_m.h"
#include "inet/transportlayer/tcp_common/TcpCrcInsertionHook.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {
namespace tcp {

class INET_API TcpCrcInsertionHook
{
  public:
    static void insertCrc(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<TcpHeader>& tcpHeader, Packet *tcpPayload);
    static uint16_t computeCrc(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<const TcpHeader>& tcpHeader, const Ptr<const Chunk>& tcpData);

    // datagramPostRoutingHook:
    static NetfilterHook::NetfilterResult datagramCrcCalculator(Packet *packet);

    static NetfilterHook::NetfilterHandler netfilterHandler;
};

} // namespace tcp
} // namespace inet

#endif

