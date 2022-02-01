//
// Copyright (C) 2021 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.configurator.gatescheduling.z3;

import inet.linklayer.configurator.gatescheduling.base.GateSchedulingConfiguratorBase;
import inet.linklayer.configurator.gatescheduling.contract.IGateSchedulingConfigurator;

//
// This module provides a gate scheduling algorithm that uses the open source
// z3 SAT solver from Microsoft. In order to be able to use this module, the
// corresponding INET feature must be enabled and the libz3-dev package must
// be installed.
//
// The source was copied from https://github.com/ACassimiro/TSNsched with the
// permission of the original authors. The code is adapted to INET interfaces,
// simplified and cleaned up a bit. Further changes to the original source are
// not going to be taken over. See ~TSNschedGateSchedulingConfigurator for using
// the latest version of the state of the art original work.
//
simple Z3GateSchedulingConfigurator extends GateSchedulingConfiguratorBase like IGateSchedulingConfigurator
{
    parameters:
        @class(Z3GateSchedulingConfigurator);
}
