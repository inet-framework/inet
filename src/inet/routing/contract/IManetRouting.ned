//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.routing.contract;

//
// Defines a module interface for Mobile Ad-hoc Network (MANET) routing protocols.
// MANET routing protocols are designed for wireless networks where nodes can
// move and the topology changes dynamically. Implementations include GPSR, DSDV,
// and DYMO protocols.
//
moduleinterface IManetRouting
{
    parameters:
        @display("i=block/app");
    gates:
        input ipIn;
        output ipOut;
}
