//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

moduleinterface IIeee8022SnapInserter
{
    parameters:
        @display("i=block/inserter");
    gates:
        input in;
        output out;
}

