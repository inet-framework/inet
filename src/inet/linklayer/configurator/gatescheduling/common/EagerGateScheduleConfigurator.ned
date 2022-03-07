//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.configurator.gatescheduling.common;

import inet.linklayer.configurator.gatescheduling.base.GateScheduleConfiguratorBase;
import inet.linklayer.configurator.gatescheduling.contract.IGateScheduleConfigurator;

//
// This module provides a gate scheduling algorithm that eagerly reserves time
// slots for the configured streams in the order of their priority (0 being the
// lowest). The allocation makes sure that only one gate (traffic category) is
// open in all network interfaces at any given moment of time. This strategy
// may result in wasting to much time of the gate cycle and thus end up failing.
//
// This configurator is primarily useful to produce statistical results that can
// be used as a baseline in comparison with more advanced configurators.
//
simple EagerGateScheduleConfigurator extends GateScheduleConfiguratorBase like IGateScheduleConfigurator
{
    parameters:
        @class(EagerGateScheduleConfigurator);
}

