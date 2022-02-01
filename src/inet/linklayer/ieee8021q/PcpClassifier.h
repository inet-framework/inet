//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PCPCLASSIFIER_H
#define __INET_PCPCLASSIFIER_H

#include "inet/queueing/base/PacketClassifierBase.h"

namespace inet {

class INET_API PcpClassifier : public queueing::PacketClassifierBase
{
  protected:
    const char *mode = nullptr;
    cValueArray *pcpToGateIndex = nullptr;
    int defaultGateIndex = -1;

  protected:
    virtual void initialize(int stage) override;
    virtual int classifyPacket(Packet *packet) override;
};

} // namespace inet

#endif

