//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.bitlevel;

//
// Converts analog signal representations to digital sample representations in
// wireless communication systems. This module transforms continuous analog
// models into discrete sample models with a configurable sample rate, handling
// both header and data portions of the signal separately. It serves as a bridge
// between the analog domain and the digital processing domain, enabling detailed
// bit-level simulation of receiver signal processing chains.
//
// @see IDigitalAnalogConverter
//
moduleinterface IAnalogDigitalConverter
{
    parameters:
        @display("i=block/control");
}

