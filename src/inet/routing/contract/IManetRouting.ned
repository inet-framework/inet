//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.routing.contract;

//
// TODO
//
moduleinterface IManetRouting
{
    parameters:
        @display("i=block/app");
    gates:
        input ipIn;
        output ipOut;
}

