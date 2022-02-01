//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021ae;

moduleinterface IIeee8021aeTagEpdHeaderInserter
{
    parameters:
        @display("i=block/cogwheel");
    gates:
        input in;
        output out;
}

