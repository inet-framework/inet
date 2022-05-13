//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCTPCRCINSERTIONHOOK_H
#define __INET_SCTPCRCINSERTIONHOOK_H

#include "inet/common/Protocol.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/transportlayer/common/CrcMode_m.h"
#include "inet/transportlayer/sctp/SctpCrcInsertionHook.h"
#include "inet/transportlayer/sctp/SctpHeader.h"

namespace inet {
namespace sctp {

class INET_API SctpCrcInsertion : public cSimpleModule, public NetfilterBase::HookBase
{
    CrcMode crcMode = CRC_MODE_UNDEFINED;

  public:
    SctpCrcInsertion() {}
    void setCrcMode(CrcMode crcModeP) { crcMode = crcModeP; }
    void insertCrc(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<SctpHeader>& sctpHeader, Packet *packet);
//    uint32_t checksum(const std::vector<uint8_t>& buf, uint32_t len);
    uint32_t checksum(unsigned char const *, unsigned int);

  public:
    virtual Result datagramPreRoutingHook(Packet *packet) override { return ACCEPT; }
    virtual Result datagramForwardHook(Packet *packet) override { return ACCEPT; }
    virtual Result datagramPostRoutingHook(Packet *packet) override;
    virtual Result datagramLocalInHook(Packet *packet) override { return ACCEPT; }
    virtual Result datagramLocalOutHook(Packet *packet) override { return ACCEPT; }
};

} // namespace tcp
} // namespace inet

#endif

