//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BRIDGINGLAYER_H
#define __INET_BRIDGINGLAYER_H

#include "inet/common/IProtocolRegistrationListener.h"

namespace inet {

class INET_API BridgingLayer : public cModule, public TransparentProtocolRegistrationListener
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;
};

} // namespace inet

#endif

