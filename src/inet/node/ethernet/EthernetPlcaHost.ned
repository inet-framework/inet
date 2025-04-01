//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.ethernet;

import inet.node.inet.StandardHost;

//
// A network node designed specifically for Ethernet 10BASE-T1S multidrop
// networks. It extends StandardHost with specialized Ethernet interfaces that
// support the Physical Layer Collision Avoidance (PLCA) protocol.
//
// 10BASE-T1S is a single-pair Ethernet standard defined in IEEE 802.3cg-2019 that
// allows multiple nodes to share a common bus topology. PLCA is a media access control
// mechanism that enables deterministic access to the shared medium without collisions,
// making it suitable for industrial automation and automotive applications.
//
// @see ~StandardHost, ~EthernetHost, ~EthernetMultidropLink
//
module EthernetPlcaHost extends StandardHost
{
    parameters:
        hasTcp = default(false);
        hasUdp = default(false);
        hasIpv4 = default(false);
        numLoInterfaces = default(0);
        eth[*].typename = default("EthernetPlcaInterface");
        @display("i=device/card");
}
