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

/**
 * A packet classifier that creates the branch for each traffic class on demand, the first
 * time a packet of that class is seen. Each branch is one element of a submodule vector
 * (submoduleName) of a configurable type (moduleType), wired between this classifier's output
 * and a downstream aggregator submodule (aggregatorSubmoduleName).
 *
 * The aggregator may be either a push ~PacketMultiplexer (the traditional use) or a pull
 * scheduler. An aggregator that needs to take notice of an input appearing at runtime picks
 * it up from the POST_MODEL_CHANGE notification of the connection itself (see
 * cPostPathCreateNotification), so no extra contract is needed between the two. This lets the
 * same classifier build both push demux/remux chains and pull per-class queue/scheduler
 * structures.
 */
class INET_API DynamicClassifier : public PacketClassifier
{
  protected:
    const char *submoduleName = nullptr; // submodule vector that holds the branches
    cModuleType *moduleType = nullptr; // type of the per-class branch module (may be a compound)
    const char *aggregatorSubmoduleName = nullptr; // downstream aggregator submodule (multiplexer or scheduler)
    std::map<int, int> classIndexToGateItMap;

  protected:
    virtual void initialize(int stage) override;
    virtual int classifyPacket(Packet *packet) override;

    virtual int createBranch();
};

} // namespace queueing
} // namespace inet

#endif
