//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// This is a base module for various packet processing modules which maintains
// a few statistics.
//
// Available display string text format directives:
// - %p - number of processed packets
// - %l - processed total packet length
//
simple PacketProcessorBase
{
    parameters:
        string displayStringTextFormat = default("processed %p pk (%l)"); // determines the text that is written on top of the submodule
        @display("i=block/star");
}
