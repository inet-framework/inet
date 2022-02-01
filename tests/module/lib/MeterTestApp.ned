//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.test.moduletest.lib;

//
// This module can be used to test Diffserv meter components.
// It is a traffic generator for the meter, and it also
// receives the packets from the meter. The different gates
// of the meter (corresponding to the conformance levels)
// should be connected to the input gates of MetersTestApp.
// The names of the conformance levels (colors) must be
// specified as a parameter.
//
// A report is written for each packet into resultFile.
//
simple MeterTestApp
{
  parameters:
    double startTime @unit(s) = default(0s);
    double stopTime @unit(s) = default(-1s);
    int numPackets = default(0); // 0 = unlimited
    volatile double iaTime @unit(s);
    volatile int packetSize @unit(B);
    string colors; // specify here as many colors, as many input gate is connected (e.g. "green yellow red")
    string resultFile = default("result.txt");
  gates:
    output out;
    input in[];
}
