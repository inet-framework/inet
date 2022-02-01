//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CONTENTBASEDCLASSIFIER_H
#define __INET_CONTENTBASEDCLASSIFIER_H

#include "inet/common/packet/PacketFilter.h"
#include "inet/queueing/base/PacketClassifierBase.h"

namespace inet {
namespace queueing {

class INET_API ContentBasedClassifier : public PacketClassifierBase
{
  protected:
    int defaultGateIndex = -1;
    std::vector<PacketFilter *> filters;

  protected:
    virtual void initialize(int stage) override;
    virtual int classifyPacket(Packet *packet) override;

  public:
    virtual ~ContentBasedClassifier();
};

} // namespace queueing
} // namespace inet

#endif

