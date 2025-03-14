//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.configurator.gatescheduling.contract;

import inet.networklayer.configurator.contract.INetworkConfigurator;

//
// Module interface for Time-Sensitive Networking (TSN) gate scheduling configurators
// that compute and configure gate schedules across a network. Gate scheduling configurators
// determine when each traffic class gate should be open or closed in every network interface,
// enabling time-based traffic control for deterministic communication.
//
// Implementations are responsible for configuring all PeriodicGate modules of all traffic
// categories in all network interfaces, ensuring that time-critical traffic flows meet
// their timing requirements while efficiently utilizing network resources.
//
// @see GateScheduleConfiguratorBase, AlwaysOpenGateScheduleConfigurator, EagerGateScheduleConfigurator, TSNschedGateScheduleConfigurator, Z3GateScheduleConfigurator
//
moduleinterface IGateScheduleConfigurator extends INetworkConfigurator
{
    parameters:
        @display("i=block/cogwheel");
}
