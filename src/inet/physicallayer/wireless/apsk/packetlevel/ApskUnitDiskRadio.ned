//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.apsk.packetlevel;

//
// Specializes ~ApskRadio to use the simplified signal representation of the
// unit disk signal propagation model. Suitable for simulations where
// computational efficiency is more important than detailed physical layer
// modeling.
//
// @see UnitDiskRadioMedium
//
module ApskUnitDiskRadio extends ApskRadio
{
    parameters:
        signalAnalogRepresentation = default("unitDisk");
}

