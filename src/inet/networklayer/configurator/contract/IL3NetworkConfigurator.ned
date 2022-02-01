//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.configurator.contract;

//
// This module interface is implemented by all layer 3 network configurators.
//
moduleinterface IL3NetworkConfigurator extends INetworkConfigurator
{
    parameters:
        @display("i=block/cogwheel");
}

