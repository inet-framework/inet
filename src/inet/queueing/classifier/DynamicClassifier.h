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
 * Creates the branch of each traffic class on demand, the first time a packet of
 * that class is seen. See the NED file for what the branches are built from and
 * how they are wired.
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
    virtual cModule *createBranchModule(int index, cGate *classifierOutputGate);
};

} // namespace queueing
} // namespace inet

#endif
