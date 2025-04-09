//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCTPCHECKSUMINSERTIONHOOK_H
#define __INET_SCTPCHECKSUMINSERTIONHOOK_H

#include "inet/common/Protocol.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/common/checksum/ChecksumMode_m.h"
#include "inet/transportlayer/sctp/SctpChecksumInsertionHook.h"
#include "inet/transportlayer/sctp/SctpHeader.h"

namespace inet {
namespace sctp {

class INET_API SctpChecksumInsertion : public cSimpleModule, public NetfilterBase::HookBase
{
    ChecksumMode checksumMode = CHECKSUM_MODE_UNDEFINED;

  public:
    SctpChecksumInsertion() {}
    void setChecksumMode(ChecksumMode checksumModeP) { checksumMode = checksumModeP; }
    void insertChecksum(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<SctpHeader>& sctpHeader, Packet *packet);
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

