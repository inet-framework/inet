//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.diffserv;

//
// This module implements a Two Rate Three Color Meter (RFC 2698).
//
// It classifies IP packets based on two rates, Peak Information Rate (PIR)
// and Committed Information Rate (CIR), and their associated burst sizes
// to be either green, yellow or red.
//
// The green traffic is guaranteed to be under (in bits)
// <pre>
//   pir * (t1 - t0) + 8 * pbs and cir * (t1 - t0) + 8 * cbs
// </pre>
// and yellow traffic to be under
// <pre>
//   pir * (t1 - t0) + 8 * pbs
// </pre>
// in every [t0,t1] interval.
//
// When operating in color-aware mode, the meter does not recolor
// a packet that was colored by a previous marker to a lesser
// conforming color.
//
simple TwoRateThreeColorMeter
{
    parameters:
        @display("i=block/timer");
        string interfaceTableModule;   // The path to the InterfaceTable module
        string pir;       // peak information rate, either absolute bitrate (e.g. "100kbps"), or relative to the link's datarate (e.g. "20%")
        int pbs @unit(B); // peak burst size
        string cir;       // committed information rate, either absolute or relative bitrate; must be smaller than pir
        int cbs @unit(B); // committed burst size
        bool colorAwareMode = default(false); // enables color-aware mode
    gates:
        input in;
        output greenOut;
        output yellowOut;
        output redOut;
}

