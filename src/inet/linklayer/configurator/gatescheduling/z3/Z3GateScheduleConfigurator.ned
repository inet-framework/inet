//
// Copyright (C) 2021 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.configurator.gatescheduling.z3;

import inet.linklayer.configurator.gatescheduling.base.GateScheduleConfiguratorBase;
import inet.linklayer.configurator.gatescheduling.contract.IGateScheduleConfigurator;

//
// Advanced gate scheduling configurator that uses the Z3 SAT solver from Microsoft to
// generate optimal Time-Sensitive Networking (TSN) gate schedules. Formulates the scheduling
// problem as a constraint satisfaction problem, creating variables for transmission and
// reception times of each packet in each flow across all network interfaces.
//
// Translates network-specific constraints (transmission duration, propagation time, packet
// ordering, traffic class priorities) and user requirements (maximum end-to-end delay, jitter
// limits) into SAT constraints. The solver finds a solution satisfying all constraints, which
// is then configured in the network. Can optionally optimize the schedule to minimize total
// end-to-end delay across all flows.
//
// Requires the 'Z3 Gate Scheduling Configurator' feature to be enabled and the libz3-dev
// package to be installed.
//
// @see GateScheduleConfiguratorBase, AlwaysOpenGateScheduleConfigurator, EagerGateScheduleConfigurator, TSNschedGateScheduleConfigurator
//
simple Z3GateScheduleConfigurator extends GateScheduleConfiguratorBase like IGateScheduleConfigurator
{
    parameters:
        bool labelAsserts = default(false);
        bool optimizeSchedule = default(true);
        @class(Z3GateScheduleConfigurator);
}
