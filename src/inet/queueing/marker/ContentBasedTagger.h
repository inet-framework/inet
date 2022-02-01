//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CONTENTBASEDTAGGER_H
#define __INET_CONTENTBASEDTAGGER_H

#include "inet/common/packet/PacketFilter.h"
#include "inet/queueing/base/PacketTaggerBase.h"

namespace inet {
namespace queueing {

class INET_API ContentBasedTagger : public PacketTaggerBase
{
  protected:
    PacketFilter filter;

  protected:
    virtual void initialize(int stage) override;
    virtual void markPacket(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif

