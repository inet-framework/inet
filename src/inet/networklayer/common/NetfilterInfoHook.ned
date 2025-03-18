//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.networklayer.common;

import inet.common.SimpleModule;

//
// Hooks into a network protocol's netfilter mechanism and logs information
// about packets at different processing stages (pre-routing, forwarding,
// post-routing, local-in, local-out). Useful for debugging and monitoring
// network traffic flow through protocol layers.
//
simple NetfilterInfoHook extends SimpleModule
{
    parameters:
        @class(NetfilterInfoHook);
        string networkProtocolModule;
}
