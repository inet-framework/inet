//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.configurator.gatescheduling.common;

import inet.linklayer.configurator.gatescheduling.base.GateScheduleConfiguratorBase;
import inet.linklayer.configurator.gatescheduling.contract.IGateScheduleConfigurator;

//
// Gate scheduling configurator that implements a trivial algorithm which keeps all
// gates in the network open all the time. This provides a baseline configuration where
// Time-Sensitive Networking (TSN) gates never block traffic, effectively disabling
// the time-based traffic control while maintaining the TSN infrastructure.
//
// @see GateScheduleConfiguratorBase, EagerGateScheduleConfigurator, TSNschedGateScheduleConfigurator, Z3GateScheduleConfigurator
//
simple AlwaysOpenGateScheduleConfigurator extends GateScheduleConfiguratorBase like IGateScheduleConfigurator
{
    parameters:
        @class(AlwaysOpenGateScheduleConfigurator);
}
