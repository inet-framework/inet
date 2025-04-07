//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETFRAGMENTFCSINSERTER_H
#define __INET_ETHERNETFRAGMENTFCSINSERTER_H

#include "inet/protocolelement/checksum/base/ChecksumInserterBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API EthernetFragmentFcsInserter : public ChecksumInserterBase
{
  protected:
    uint64_t lastFragmentCompleteFcs = 0;
    mutable uint64_t currentFragmentCompleteFcs = 0;

  protected:
    virtual uint64_t computeComputedChecksum(const Packet *packet, ChecksumType checksumType) const override;
    virtual void processPacket(Packet *packet) override;
    virtual void handlePacketProcessed(Packet *packet) override;
};

} // namespace inet

#endif

