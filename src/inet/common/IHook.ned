//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.common;

//
// Generic module that can be inserted in some points in the model
//
moduleinterface IHook
{
    @display("i=block/socket");
    gates:
        input in;
        output out;
}

