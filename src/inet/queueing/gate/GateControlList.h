//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GATECONTROLLIST_H
#define __INET_GATECONTROLLIST_H

#include "inet/queueing/gate/PeriodicGate.h"

namespace inet {
namespace queueing {

class INET_API GateControlList : public cSimpleModule
{
  protected:
    int numGates = -1;
    cValueArray *durations = nullptr;
    cValueArray *gateStates = nullptr;
    std::vector<bool> initiallyOpens;
    std::vector<simtime_t> offsets;
    std::vector<cValueArray *> gateDurations;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual void parseGcl();
    virtual std::vector<bool> parseGclLine(const char *gateStates);
};

} // namespace queueing
} // namespace inet

#endif

