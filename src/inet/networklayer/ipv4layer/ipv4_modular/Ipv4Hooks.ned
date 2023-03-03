//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4layer.ipv4_modular;

//
// TODO documentation
//
moduleinterface INetfilterHook
{
    parameters:
        @display("i=old/hook");
        @ommittedTypename("OmittedNetfilterHook");
    gates:
        input in;
        output out;
}

module OmittedNetfilterHook like INetfilterHook
{
    parameters:
        @class(::inet::OmittedModule);
        @display("i=old/hook,black,50");
    gates:
        input in;
        output out;
    connections:
        in --> out;
}

module BypassNetfilterHook like INetfilterHook
{
    parameters:
        @display("i=old/hook,black,50");
    gates:
        input in;
        output out;
    connections:
        in --> out;
}
