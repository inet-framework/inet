//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.configurator.base;

import inet.common.SimpleModule;

simple NetworkConfiguratorBase extends SimpleModule
{
    parameters:
        @class(NetworkConfiguratorBase);
        @display("i=block/cogwheel");
}

