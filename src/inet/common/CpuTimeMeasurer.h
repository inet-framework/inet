//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CPUTIMEMEASURER_H
#define __INET_CPUTIMEMEASURER_H

#include "inet/common/INETDefs.h"

namespace inet {

class CpuTimeMeasurementListener : public cISimulationLifecycleListener {
  private:
    timespec startTime;
    timespec endTime;

  public:
    virtual void lifecycleEvent(SimulationLifecycleEventType eventType, cObject *details) override;
};

}  // namespace inet

#endif
