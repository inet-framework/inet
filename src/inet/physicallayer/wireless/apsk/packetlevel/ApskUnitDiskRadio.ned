//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.apsk.packetlevel;

//
// Radio model that uses the simplified unit disk signal representation with APSK
// modulation. Provides a fast and predictable physical layer behavior based on
// communication, interference, and detection ranges. Suitable for simulations
// where computational efficiency is more important than detailed physical layer
// modeling.
//
module ApskUnitDiskRadio extends ApskRadio
{
    parameters:
        signalAnalogRepresentation = default("unitDisk");
}
