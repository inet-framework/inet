//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LABELCLASSIFIER_H
#define __INET_LABELCLASSIFIER_H

#include "inet/queueing/base/PacketClassifierBase.h"

namespace inet {
namespace queueing {

class INET_API LabelClassifier : public PacketClassifierBase
{
  protected:
    int defaultGateIndex = -1;
    std::map<std::string, int> labelsToGateIndexMap;

  protected:
    virtual void initialize(int stage) override;
    virtual int classifyPacket(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif

