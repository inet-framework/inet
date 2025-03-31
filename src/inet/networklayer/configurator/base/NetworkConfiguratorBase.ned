//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.networklayer.configurator.base;

//
// Base module for network configurators in the INET framework. Provides
// fundamental functionality for extracting network topology by identifying
// network nodes and their connections. Serves as a foundation for more
// specialized configurators that handle tasks like address assignment and
// routing table configuration.
//
simple NetworkConfiguratorBase
{
    parameters:
        @display("i=block/cogwheel");
}
