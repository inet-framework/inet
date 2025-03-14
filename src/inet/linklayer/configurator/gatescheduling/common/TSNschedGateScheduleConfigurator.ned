//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.configurator.gatescheduling.common;

import inet.linklayer.configurator.gatescheduling.base.GateScheduleConfiguratorBase;
import inet.linklayer.configurator.gatescheduling.contract.IGateScheduleConfigurator;

//
// Gate scheduling configurator that integrates with the external TSNsched tool to generate
// optimized Time-Sensitive Networking (TSN) gate schedules. Leverages advanced scheduling
// algorithms from TSNsched to efficiently allocate time slots for traffic streams while
// meeting timing constraints and maximizing network utilization.
//
// Requires the TSNsched tool (https://github.com/ACassimiro/TSNsched) to be installed
// along with a Java SDK. The module communicates with TSNsched through file I/O and
// system commands, using the TSNSCHED_ROOT environment variable to locate the tool.
// Tested with TSNsched revision: 3f3bf663d196ec6c03e81a1e1392d4aefd158e3e.
//
// @see GateScheduleConfiguratorBase, AlwaysOpenGateScheduleConfigurator, EagerGateScheduleConfigurator, Z3GateScheduleConfigurator
//
simple TSNschedGateScheduleConfigurator extends GateScheduleConfiguratorBase like IGateScheduleConfigurator
{
    parameters:
        @class(TSNschedGateScheduleConfigurator);
}
