//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DYNAMICCLASSIFIER_H
#define __INET_DYNAMICCLASSIFIER_H

#include <vector>

#include "inet/queueing/classifier/PacketClassifier.h"

namespace inet {
namespace queueing {

using namespace inet::queueing;

/**
 * A packet classifier that creates the branch for each traffic class on demand, the first
 * time a packet of that class is seen. Each branch is one submodule of a configurable type
 * (moduleType), wired between this classifier's output and a downstream aggregator submodule
 * (aggregatorSubmoduleName).
 *
 * The aggregator may be either a push ~PacketMultiplexer (the traditional use) or a pull
 * scheduler; if it implements ~IDynamicInputScheduler the newly created input is registered
 * with it via addInput(). This lets the same classifier build both push demux/remux chains
 * and pull per-class queue/scheduler structures.
 *
 * If spliceBranchSubmodules is set and moduleType is a compound module forming a linear
 * chain (in -> a -> b -> ... -> out), its inner submodules are *spliced* directly into this
 * classifier's parent (as vector elements named after the inner submodules) instead of being
 * kept behind the compound's boundary -- so a downstream matched-pair scheduler can address
 * them directly. See the corresponding NED file for more details.
 */
class INET_API DynamicClassifier : public PacketClassifier
{
  protected:
    const char *submoduleName = nullptr; // submodule vector for the branch module (non-splice); unused when splicing
    cModuleType *moduleType = nullptr; // type of the per-class branch module (may be a compound)
    const char *aggregatorSubmoduleName = nullptr; // downstream aggregator submodule (multiplexer or scheduler)
    bool spliceBranchSubmodules = false; // flatten the (compound) branch module's submodules into the branch
    std::map<int, int> classIndexToGateItMap;

  protected:
    virtual void initialize(int stage) override;
    virtual int classifyPacket(Packet *packet) override;

    virtual int createBranch();
    virtual void forwardMatchingParams(cModule *module);
    virtual cGate *createModuleBranch(int index, cGate *classifierOutputGate, std::vector<cModule *>& modulesToInitialize);
    virtual cGate *spliceBranch(int index, cGate *classifierOutputGate, std::vector<cModule *>& modulesToInitialize);
};

} // namespace queueing
} // namespace inet

#endif
