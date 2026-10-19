//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DYNAMICCLASSIFIER_H
#define __INET_DYNAMICCLASSIFIER_H

#include "inet/queueing/classifier/PacketClassifier.h"

namespace inet {
namespace queueing {

using namespace inet::queueing;

class INET_API DynamicClassifier : public PacketClassifier
{
  protected:
    const char *submoduleName = nullptr; // submodule vector that holds the branches
    cModuleType *moduleType = nullptr; // type of the per-class branch module (may be a compound)
    const char *aggregatorSubmoduleName = nullptr; // downstream aggregator submodule (multiplexer or scheduler)
    std::map<int, int> classIndexToGateItMap;

  protected:
    virtual void initialize(int stage) override;
    virtual int getClassIndex(Packet *packet) const;
    virtual int classifyPacket(Packet *packet) const override;
    virtual void createBranchIfNeeded(Packet *packet);
    virtual void startPacketStreaming(Packet *packet) override;

  public:
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override;
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
};

} // namespace queueing
} // namespace inet

#endif

