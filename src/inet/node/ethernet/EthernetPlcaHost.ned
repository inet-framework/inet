//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.ethernet;

import inet.node.inet.StandardHost;

//
// This module implements a network node that is suitable for use in Ethernet
// 10BASE-T1S multidrop links. Such a multidrop link uses the Ethernet Phyisical
// Layer Collision Avoidance (PLCA) protocol. The protocol is defined in the IEEE
// 802.3cg-2019 standard.
//
// This module is derived from ~StandardHost to inherit all standard network node
// features. In most cases Ethernet 10BASE-T1S nodes only use Ethernet applications,
// so all higher layer protocols are disabled by default.
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
