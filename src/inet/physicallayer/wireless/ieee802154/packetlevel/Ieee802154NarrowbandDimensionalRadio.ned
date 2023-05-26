//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee802154.packetlevel;

module Ieee802154DimensionalNarrowbandRadio extends Ieee802154NarrowbandRadio
{
    parameters:
        signalAnalogRepresentation = default("dimensional");
}

