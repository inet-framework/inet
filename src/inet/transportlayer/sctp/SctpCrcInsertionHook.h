//
// Copyright 2017 OpenSim Ltd.
//
// This library is free software, you can redistribute it and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 3 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#ifndef __INET_SCTPCRCINSERTIONHOOK_H
#define __INET_SCTPCRCINSERTIONHOOK_H

#include "inet/common/INETDefs.h"
#include "inet/common/Protocol.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/transportlayer/common/CrcMode_m.h"
#include "inet/transportlayer/sctp/SctpCrcInsertionHook.h"
#include "inet/transportlayer/sctp/SctpHeader.h"

namespace inet {
namespace sctp {

class SctpCrcInsertion : public NetfilterBase::HookBase {
    CrcMode crcMode = CRC_MODE_UNDEFINED;
  public:
    SctpCrcInsertion() {}
    void setCrcMode(CrcMode crcModeP) { crcMode = crcModeP; }
    void insertCrc(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<SctpHeader>& sctpHeader, Packet *packet);
 // uint32 checksum(const std::vector<uint8_t>& buf, uint32 len);
    uint32 checksum(unsigned char const*, unsigned int);

  public:
    virtual Result datagramPreRoutingHook(Packet *packet) override { return ACCEPT; }
    virtual Result datagramForwardHook(Packet *packet) override { return ACCEPT; }
    virtual Result datagramPostRoutingHook(Packet *packet) override;
    virtual Result datagramLocalInHook(Packet *packet) override { return ACCEPT; }
    virtual Result datagramLocalOutHook(Packet *packet) override { return ACCEPT; }
};

} // namespace tcp
} // namespace inet

#endif // ifndef __INET_TCPCRCINSERTIONHOOK_H
