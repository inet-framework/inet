//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.configurator.gatescheduling.common;

import inet.linklayer.configurator.gatescheduling.base.GateScheduleConfiguratorBase;
import inet.linklayer.configurator.gatescheduling.contract.IGateScheduleConfigurator;

//
// Gate scheduling configurator that implements a priority-based algorithm which eagerly
// reserves time slots for configured streams in priority order (0 being lowest priority).
// Ensures that only one traffic category gate is open in all network interfaces at any
// given time, providing strict traffic isolation but potentially wasting gate cycle time
// and failing to accommodate all streams due to inefficient resource allocation.
//
// Primarily useful as a baseline for comparison with more advanced Time-Sensitive
// Networking (TSN) scheduling algorithms that implement more efficient time slot allocation.
//
// @see GateScheduleConfiguratorBase, AlwaysOpenGateScheduleConfigurator, TSNschedGateScheduleConfigurator, Z3GateScheduleConfigurator
//
simple EagerGateScheduleConfigurator extends GateScheduleConfiguratorBase like IGateScheduleConfigurator
{
    parameters:
        @class(EagerGateScheduleConfigurator);
}
