//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

moduleinterface IIeee8022LlcChecker
{
    parameters:
        @display("i=block/checker");
    gates:
        input in;
        output out;
}

