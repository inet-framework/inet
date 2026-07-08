//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIMULATIONTASKEXAMPLE_H
#define __INET_SIMULATIONTASKEXAMPLE_H

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API SimulationTaskExample : public cSimpleModule
{
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void runSimulationTaskExample();
};

} // namespace inet

#endif
