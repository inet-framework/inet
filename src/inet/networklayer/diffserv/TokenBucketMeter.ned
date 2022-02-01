//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.diffserv;

//
// Simple token bucket meter.
//
// IP packets are classified according to two parameters,
// Committed Information Rate (cir), Committed Burst Size (cbs),
// to be either green, or red.
//
// Green traffic is guaranteed to be under (in bits)
// <pre>
//     cir * (t1 - t0) + 8 * cbs
// </pre>
// in every [t0,t1] interval.
//
// When operating in color-aware mode, the meter does not classify
// a packet to be green, if it was marked with a higher color
// by a previous meter.
//
simple TokenBucketMeter
{
    parameters:
        @display("i=block/timer");
        string interfaceTableModule;   // The path to the InterfaceTable module
        string cir;       // committed information rate, either absolute bitrate (e.g. "100kbps"), or relative to the link's datarate (e.g. "20%")
        int cbs @unit(B); // committed burst size
        bool colorAwareMode = default(false); // enables color-aware mode
    gates:
        input in;
        output greenOut;
        output redOut;
}

