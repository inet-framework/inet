//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GPTPDOMAINCLASSIFIER_H
#define __INET_GPTPDOMAINCLASSIFIER_H

#include "inet/queueing/base/PacketClassifierBase.h"

namespace inet {

class INET_API GptpDomainClassifier : public queueing::PacketClassifierBase
{
  protected:
    std::map<int, int> gptpDomainToGateIndex;

  protected:
    virtual void initialize(int stage) override;
    virtual int classifyPacket(Packet *packet) override;
};

} // namespace inet

#endif

