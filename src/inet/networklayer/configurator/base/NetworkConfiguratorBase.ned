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
        @class(-);  //# set to '-' to prevent unintentional instantiation. Also disable the linter: suppress(missing-cpp-class)
        @display("i=block/cogwheel");
}

