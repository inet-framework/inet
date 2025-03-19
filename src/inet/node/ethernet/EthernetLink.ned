//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.ethernet;

import inet.common.Cable;

//
// EthernetLink provides a base channel type for Ethernet connections with configurable
// length and propagation delay. It extends the Cable type and calculates the delay
// based on the physical length of the cable and the speed of light in the medium.
//
// The EthernetLink serves as a base for specific Ethernet standards with different
// data rates, from 10 Mbps to 400 Gbps.
//
channel EthernetLink extends Cable
{
    double length @unit(m) = default(10m);
    delay = default(replaceUnit(length / 2e8, "s"));
}

//
// 10 megabit/sec Ethernet link
//
channel Eth10M extends EthernetLink
{
    datarate = 10Mbps;
}

//
// 100 megabit/sec Ethernet link
//
channel Eth100M extends EthernetLink
{
    datarate = 100Mbps;
}

//
// 1 gigabit/sec Ethernet link
//
channel Eth1G extends EthernetLink
{
    datarate = 1Gbps;
}

//
// 10 gigabit/sec Ethernet link
//
channel Eth10G extends EthernetLink
{
    datarate = 10Gbps;
}

//
// 40 gigabit/sec Ethernet link
//
channel Eth40G extends EthernetLink
{
    datarate = 40Gbps;
}

//
// 100 gigabit/sec Ethernet link
//
channel Eth100G extends EthernetLink
{
    datarate = 100Gbps;
}

//
// 200 gigabit/sec Ethernet link
//
channel Eth200G extends EthernetLink
{
    datarate = 200Gbps;
}

//
// 400 gigabit/sec Ethernet link
//
channel Eth400G extends EthernetLink
{
    datarate = 400Gbps;
}

