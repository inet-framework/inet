//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETBUFFERBASE_H
#define __INET_PACKETBUFFERBASE_H

#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IPacketQueue.h"

namespace inet {
namespace queueing {

class INET_API PacketBufferBase : public PacketProcessorBase, public virtual IPacketCollection
{
  protected:
    int numAddedPackets = -1;
    int numRemovedPackets = -1;
    int numDroppedPackets = -1;

  protected:
    virtual void initialize(int stage) override;
    virtual void emit(simsignal_t signal, cObject *object, cObject *details = nullptr) override;
    virtual std::string resolveDirective(char directive) const override;
};

} // namespace queueing
} // namespace inet

#endif

