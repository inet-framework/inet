//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.apsk.packetlevel;

//
// Radio model that uses scalar signal representation with APSK modulation.
// Represents signal power as a single scalar value rather than as a function of
// time and frequency. Provides a balance between accuracy and computational
// efficiency, suitable for most wireless network simulations where detailed
// physical layer modeling is not critical.
//
module ApskScalarRadio extends ApskRadio
{
    parameters:
        signalAnalogRepresentation = default("scalar");
}
