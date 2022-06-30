//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TOKENBUCKETCLASSIFIER_H
#define __INET_TOKENBUCKETCLASSIFIER_H

#include "inet/queueing/base/PacketClassifierBase.h"
#include "inet/queueing/base/TokenBucketClassifierMixin.h"
#include "inet/queueing/base/TokenBucketMixin.h"

namespace inet {
namespace queueing {

extern template class TokenBucketClassifierMixin<TokenBucketMixin<PacketClassifierBase>>;

class INET_API TokenBucketClassifier : public TokenBucketClassifierMixin<TokenBucketMixin<PacketClassifierBase>>
{
  protected:
    virtual void initialize(int stage) override;

    virtual int classifyPacket(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif

