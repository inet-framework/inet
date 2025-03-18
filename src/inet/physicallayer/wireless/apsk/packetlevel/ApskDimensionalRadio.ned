//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.apsk.packetlevel;

//
// Radio model that uses dimensional (time and frequency domain) signal
// representation with APSK modulation. Provides more accurate modeling of signal
// propagation by representing power as a function of time and frequency. Suitable
// for simulations requiring detailed physical layer behavior and realistic
// interference modeling.
//
module ApskDimensionalRadio extends ApskRadio
{
    parameters:
        signalAnalogRepresentation = default("dimensional");
}
