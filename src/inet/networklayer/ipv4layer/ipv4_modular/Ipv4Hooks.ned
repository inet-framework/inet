//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4layer.ipv4_modular;

//
// Interface for modules representing netfilter hooks, used e.g. in
// the modular IPv4 implementation ~Ipv4Modular.
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

//
// This module is a placeholder that does not perform any function and is removed
// from the module hierarchy (i.e. replaced by a connection) during initialization.
// It can be used instead of making a submodule of the given module interface type
// conditional.
//
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
