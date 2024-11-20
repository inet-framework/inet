//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/common/CpuTimeMeasurer.h"

namespace inet {

Register_GlobalConfigOption(CFGID_CPU_TIME_MEASUREMENT, "cpu-time-measurement", CFG_BOOL, "false", "Enable or disable CPU time measurement during simulation.");

EXECUTE_ON_STARTUP(
    CpuTimeMeasurementListener *listener = new CpuTimeMeasurementListener();
    cSimulation::getActiveEnvir()->addLifecycleListener(listener);
);

void CpuTimeMeasurementListener::lifecycleEvent(SimulationLifecycleEventType eventType, cObject *details)
{
    switch (eventType) {
        case LF_PRE_NETWORK_INITIALIZE: {
            if (cSimulation::getActiveEnvir()->getConfig()->getAsBool(CFGID_CPU_TIME_MEASUREMENT, false)) {
                std::cerr << "Measurement started" << std::endl;
                clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startTime);
            }
            break;
        }
        case LF_POST_NETWORK_FINISH: {
            if (cSimulation::getActiveEnvir()->getConfig()->getAsBool(CFGID_CPU_TIME_MEASUREMENT, false)) {
                clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endTime);
                double elapsedTime = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_nsec - startTime.tv_nsec) / 1e9;
                std::cerr << "Elapsed CPU time: " << elapsedTime << std::endl;
                break;
            }
        }
        default:
            break;
    }
}

}  // namespace inet
