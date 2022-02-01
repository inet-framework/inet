//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_WRRCLASSIFIER_H
#define __INET_WRRCLASSIFIER_H

#include "inet/queueing/base/PacketClassifierBase.h"
#include "inet/queueing/contract/IPacketCollection.h"

namespace inet {
namespace queueing {

/**
 * This module implements a Weighted Round Robin Classifier.
 */
class INET_API WrrClassifier : public PacketClassifierBase
{
  protected:
    int *weights = nullptr;
    int *buckets = nullptr;

    std::vector<IPacketCollection *> collections;

  protected:
    virtual void initialize(int stage) override;
    virtual int classifyPacket(Packet *packet) override;

  public:
    virtual ~WrrClassifier();
};

} // namespace queueing
} // namespace inet

#endif

