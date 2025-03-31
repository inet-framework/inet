//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//TODO review
package inet.physicallayer.wireless.ieee802154.packetlevel;

import inet.physicallayer.wireless.common.base.packetlevel.NarrowbandTransmitterBase;

//
// Transmitter model for IEEE 802.15.4 narrowband PHY in the 2.4 GHz band.
// Implements the transmitter component of the IEEE 802.15.4 physical layer with
// DSSS-OQPSK-16 modulation as specified in the standard. Creates transmissions
// with appropriate header format, preamble duration, and other parameters
// according to the IEEE 802.15.4-2006 specification. Used by the
// Ieee802154NarrowbandRadio module as part of a complete IEEE 802.15.4 radio
// implementation.
//
module Ieee802154NarrowbandTransmitter extends NarrowbandTransmitterBase
{
    parameters:
        modulation = default("DSSS-OQPSK-16");
        @class(Ieee802154NarrowbandTransmitter);
}
