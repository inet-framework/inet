//
// Copyright (C) 2003 CTIE, Monash University
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.examples.ethernet.TenBaseT1S;

import inet.node.inet.StandardHost;

//
// This module implements an Ethernet PLCA network node suitable for use in
// Ethernet 10Base-T1S multidrop links. The Ethernet Phyisical Layer Collision
// Avoidance (PLCA) protocol is defined in the IEEE 802.3cg-2019 standard.
//
// This module is derived from ~StandardHost to inherit all standard network
// node features. In most cases Ethernet PLCA nodes use Ethernet applications,
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
