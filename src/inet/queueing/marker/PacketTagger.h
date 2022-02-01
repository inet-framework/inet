//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETTAGGER_H
#define __INET_PACKETTAGGER_H

#include "inet/queueing/base/PacketTaggerBase.h"
#include "inet/queueing/contract/IPacketFilterFunction.h"

namespace inet {
namespace queueing {

class INET_API PacketTagger : public PacketTaggerBase
{
  protected:
    IPacketFilterFunction *packetFilterFunction = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual IPacketFilterFunction *createFilterFunction(const char *filterClass) const;
    virtual void markPacket(Packet *packet) override;

  public:
    virtual ~PacketTagger();
};

} // namespace queueing
} // namespace inet

#endif

