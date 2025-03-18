//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.generic;

//
// Radio model that uses dimensional representation for wireless transmissions.
// Represents signal power as a function of time and frequency rather than as a
// single scalar value. Provides more accurate modeling of signal propagation,
// interference, and reception compared to scalar or unit disk models. Part of
// the generic radio model family, suitable for simulations requiring detailed
// physical layer behavior.
//
module GenericDimensionalRadio extends GenericRadio
{
    parameters:
        signalAnalogRepresentation = default("dimensional");
}
