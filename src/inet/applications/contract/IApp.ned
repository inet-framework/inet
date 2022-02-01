//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.contract;

//
// Generic application interface.
//
moduleinterface IApp
{
    parameters:
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
}

