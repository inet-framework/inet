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
    const char *submoduleName = nullptr;
    cModuleType *moduleType = nullptr;
    std::map<int, int> classIndexToGateItMap;

  protected:
    virtual void initialize(int stage) override;
    virtual int classifyPacket(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif

