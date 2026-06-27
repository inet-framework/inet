//
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.routing.contract;

//
// Interface for VRRP (Virtual Router Redundancy Protocol) modules connected
// to the IPv4 network layer (IP protocol number 112).
//
moduleinterface IVrrp
{
    parameters:
        @display("i=block/network2");
    gates:
        input ipIn @labels(Ipv4ControlInfo/up);
        output ipOut @labels(Ipv4ControlInfo/down);
}
