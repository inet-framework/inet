//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIMULATIONCPUUSAGEMEASURER_H
#define __INET_SIMULATIONCPUUSAGEMEASURER_H

#include "inet/common/INETMath.h"

namespace inet {

class SimulationCpuUsageMeasurer : public cISimulationLifecycleListener {
  protected:
    timespec startTime;
    timespec endTime;
    double elapsedTime = NaN;

    int cyclesFd = -1;
    int instructionsFd = -1;
    uint64_t numCycles = 0;
    uint64_t numInstructions = 0;

  protected:
    virtual void startMeasurement();
    virtual void stopMeasurement();

  public:
    virtual void lifecycleEvent(SimulationLifecycleEventType eventType, cObject *details) override;
};

}  // namespace inet

#endif
