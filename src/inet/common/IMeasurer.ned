//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.common;

//
// This module interface is implemented by all standalone measurement modules.
// These measurement modules can be used in various places such as network
// interfaces, network nodes, subnetworks or even the whole network. They most
// often collect statistical results by subscribing to signals of other modules
// and by doing their calculation internally.
//
moduleinterface IMeasurer
{
    parameters:
        @display("i=block/cogwheel");
}
