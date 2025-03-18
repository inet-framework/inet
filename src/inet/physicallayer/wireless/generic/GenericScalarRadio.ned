//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.generic;

//
// Radio model that uses scalar representation for wireless transmissions.
// Represents signal power as a single scalar value rather than as a function of
// time and frequency. Provides a balance between accuracy and computational
// efficiency, suitable for network-level simulations where detailed physical
// layer behavior is not critical. Part of the generic radio model family along
// with unit disk and dimensional variants.
//
module GenericScalarRadio extends GenericRadio
{
    parameters:
        signalAnalogRepresentation = default("scalar");
}
