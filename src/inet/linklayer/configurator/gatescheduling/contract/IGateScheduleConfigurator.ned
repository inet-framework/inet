//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.configurator.gatescheduling.contract;

import inet.networklayer.configurator.contract.INetworkConfigurator;

//
// This module interface is implemented by gate scheduling configurator modules.
// A gate scheduling configurator is responsible for configuring all ~PeriodicGate
// modules of all traffic categories in all network interfaces in the network.
//
moduleinterface IGateScheduleConfigurator extends INetworkConfigurator
{
    parameters:
        @display("i=block/cogwheel");
}

