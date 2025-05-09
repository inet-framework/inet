//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.bitlevel;

//
// Interface for digital-to-analog converter modules that transform discrete-time
// signal samples into continuous-time analog signals. This module represents the
// final stage in the digital signal processing chain before transmission over
// the wireless medium. It takes a sample model containing discrete signal
// samples as input and produces an analog model representing the continuous
// waveform as output. The conversion process typically involves interpolation
// between sample points to create a smooth analog signal.
//
// @see IAnalogDigitalConverter
//
moduleinterface IDigitalAnalogConverter
{
    parameters:
        @display("i=block/control");
}

