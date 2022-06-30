//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MULTITOKENBUCKETCLASSIFIER_H
#define __INET_MULTITOKENBUCKETCLASSIFIER_H

#include "inet/queueing/base/MultiTokenBucketMixin.h"
#include "inet/queueing/base/PacketClassifierBase.h"
#include "inet/queueing/base/TokenBucketClassifierMixin.h"

namespace inet {
namespace queueing {

extern template class TokenBucketClassifierMixin<MultiTokenBucketMixin<PacketClassifierBase>>;


class INET_API MultiTokenBucketClassifier : public TokenBucketClassifierMixin<MultiTokenBucketMixin<PacketClassifierBase>>
{
  protected:
    virtual void initialize(int stage) override;

    virtual int classifyPacket(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif

