//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.ethernet;

import ned.DatarateChannel;

//
// EthernetMultidropLink models a shared Ethernet medium in a bus topology where
// multiple nodes connect to a single cable. This channel type is used for
// simulating 10BASE-T1S with PLCA (Physical Layer Collision Avoidance)
// Ethernet that use a multidrop configuration.
//
// @see ~EthernetPlcaHost, ~EthernetLink
//
channel EthernetMultidropLink extends DatarateChannel
{
    double length @unit(m);
    delay = default(replaceUnit(length / 2e8, "s"));
    datarate = default(10Mbps);
}
