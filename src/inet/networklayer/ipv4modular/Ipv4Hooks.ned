//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4modular;

//
// TODO documentation
//
moduleinterface IIpv4Hook
{
    parameters:
        @display("i=old/hook");
    gates:
        input in;
        output out;
}

module Ipv4HookBypass like IIpv4Hook
{
    parameters:
        @display("i=old/hook");
    gates:
        input in;
        output out;
    connections:
        in --> out;
}
