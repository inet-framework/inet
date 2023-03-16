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
    int numGates;
    clocktime_t offset;
    cValueArray *durations = nullptr;
    cValueArray *gateStates = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

  private:
    // ownership
    std::vector<cValueArray *> gateDurations;

    void parseGcl();
    static std::vector<bool> retrieveGateStates(const char *gateStates, uint numGates);

  public:
    ~GateControlList();

};

} // namespace queueing
} // namespace inet

#endif

