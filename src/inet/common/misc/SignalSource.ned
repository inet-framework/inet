//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.common.misc;

//
// Emits double-valued signals in the specified interval.
// May be used for testing indicator figures.
//
simple SignalSource
{
    parameters:
        @display("i=block/cogwheel");
        @signal[signalSourceOutput];
        double startTime @unit(s) = default(0s);
        double endTime @unit(s) = default(-1s);
        volatile double interval @unit(s) = default(1s);
        string signalName = "signalSourceOutput";
        volatile double value;
}

