//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.examples.ethernet.TenBaseT1S;

import inet.node.inet.StandardHost;

//
// This module implements an Ethernet network node that is suitable for use in
// Ethernet 10BASE-T1S multidrop links. Such a multidrop link uses the Ethernet
// Phyisical Layer Collision Avoidance (PLCA) protocol. The protocol is defined
// in the IEEE 802.3cg-2019 standard.
//
module EthernetPlcaNode extends StandardHost
{
    parameters:
        numEthInterfaces = default(1);
        eth[*].typename = default("EthernetPlcaInterface");
        @display("i=device/card");
}
