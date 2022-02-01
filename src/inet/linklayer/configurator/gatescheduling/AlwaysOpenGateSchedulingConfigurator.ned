//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.configurator.gatescheduling;

import inet.linklayer.configurator.gatescheduling.base.GateSchedulingConfiguratorBase;
import inet.linklayer.configurator.gatescheduling.contract.IGateSchedulingConfigurator;

//
// This module provides a trivial gate scheduling algorithm that makes all gates
// in the network to be open all the time.
//
simple AlwaysOpenGateSchedulingConfigurator extends GateSchedulingConfiguratorBase like IGateSchedulingConfigurator
{
    parameters:
        @class(AlwaysOpenGateSchedulingConfigurator);
}

