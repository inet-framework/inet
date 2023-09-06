//
// Copyright (C) 2003 CTIE, Monash University
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.examples.ethernet.TenBaseT1S;

import inet.node.inet.StandardHost;

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
