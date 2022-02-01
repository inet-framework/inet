//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.apsk.packetlevel;

import inet.physicallayer.wireless.common.base.packetlevel.DimensionalTransmitterBase;

//
// This transmitter model produces transmissions that use dimensional
// transmission power (that changes over time and/or frequency) in their analog
// representation and the configured modulation.
//
// @see ~ApskDimensionalReceiver, ~ApskDimensionalRadio,
// ~ApskDimensionalRadioMedium.
//
module ApskDimensionalTransmitter extends DimensionalTransmitterBase
{
    parameters:
        @class(ApskDimensionalTransmitter);
}

