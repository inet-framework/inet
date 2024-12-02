//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifdef __linux__

#include "inet/common/SimulationCpuUsageMeasurer.h"

#include <asm/unistd.h>
#include <cstring>
#include <iostream>
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace inet {

Register_GlobalConfigOption(CFGID_MEASURE_CPU_USAGE, "measure-cpu-usage", CFG_BOOL, "false", "Enable or disable simulation CPU usage measurements during simulation.");

EXECUTE_ON_STARTUP(
    SimulationCpuUsageMeasurer *listener = new SimulationCpuUsageMeasurer();
    cSimulation::getActiveEnvir()->addLifecycleListener(listener);
);

static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid, int cpu, int group_fd, unsigned long flags)
{
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

void SimulationCpuUsageMeasurer::startMeasurement()
{
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startTime);

    struct perf_event_attr pe;
    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE;
    pe.size = sizeof(struct perf_event_attr);
    pe.exclude_kernel = 1;  // exclude kernel mode
    pe.exclude_hv = 1;      // exclude hypervisor
    pe.exclude_idle = 1;    // exclude idle time
    pe.disabled = 1;        // start disabled

    pe.config = PERF_COUNT_HW_CPU_CYCLES;
    cyclesFd = perf_event_open(&pe, 0, -1, -1, 0);
    if (cyclesFd == -1)
        throw cRuntimeError("Cannot open cycles counter: %s", strerror(errno));

    pe.config = PERF_COUNT_HW_INSTRUCTIONS;
    instructionsFd = perf_event_open(&pe, 0, -1, -1, 0);
    if (instructionsFd == -1)
        throw cRuntimeError("Cannot open instructions counter: %s", strerror(errno));

    ioctl(cyclesFd, PERF_EVENT_IOC_RESET, 0);
    ioctl(instructionsFd, PERF_EVENT_IOC_RESET, 0);
    ioctl(cyclesFd, PERF_EVENT_IOC_ENABLE, 0);
    ioctl(instructionsFd, PERF_EVENT_IOC_ENABLE, 0);
}

void SimulationCpuUsageMeasurer::stopMeasurement()
{
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endTime);
    elapsedTime = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_nsec - startTime.tv_nsec) / 1e9;

    ioctl(cyclesFd, PERF_EVENT_IOC_DISABLE, 0);
    ioctl(instructionsFd, PERF_EVENT_IOC_DISABLE, 0);

    if (read(cyclesFd, &numCycles, sizeof(numCycles)) == -1)
        throw cRuntimeError("Cannot read cycles counter: %s", strerror(errno));
    if (read(instructionsFd, &numInstructions, sizeof(numInstructions)) == -1)
        throw cRuntimeError("Cannot read instructions counter: %s", strerror(errno));

    close(cyclesFd);
    close(instructionsFd);
}

void SimulationCpuUsageMeasurer::lifecycleEvent(SimulationLifecycleEventType eventType, cObject *details)
{
    switch (eventType) {
        case LF_PRE_NETWORK_INITIALIZE: {
            if (cSimulation::getActiveEnvir()->getConfig()->getAsBool(CFGID_MEASURE_CPU_USAGE, false)) {
                startMeasurement();
                EV_INFO << "Simulation CPU usage measurement started" << std::endl;
            }
            break;
        }
        case LF_POST_NETWORK_FINISH: {
            if (cSimulation::getActiveEnvir()->getConfig()->getAsBool(CFGID_MEASURE_CPU_USAGE, false)) {
                stopMeasurement();
                EV_INFO << "Simulation CPU usage measurement stopped" << std::endl;
                std::cout << "Simulation CPU usage: "
                          << "elapsedTime = " << elapsedTime << ", "
                          << "numCycles = " << numCycles << ", "
                          << "numInstructions = " << numInstructions << std::endl;
                break;
            }
        }
        default:
            break;
    }
}

}  // namespace inet

#endif
