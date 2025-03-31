//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.ieee80211.bitlevel;

import inet.physicallayer.wireless.common.base.packetlevel.FlatRadioBase;

//
// Radio model for IEEE 802.11 OFDM physical layer communications. Implements a
// detailed, layered representation of OFDM modulation used in standards like
// 802.11a/g/n/ac. Provides bit-level and symbol-level processing with specialized
// transmitter and receiver components. Supports different levels of simulation
// detail from packet-level to sample-level, allowing appropriate trade-offs
// between simulation accuracy and performance.
//
module Ieee80211OfdmRadio extends FlatRadioBase
{
    parameters:
        transmitter.typename = default("Ieee80211LayeredOfdmTransmitter");
        receiver.typename = default("Ieee80211LayeredOfdmReceiver");
        receiver.errorModel.typename = default("Ieee80211OfdmErrorModel");
        @class(Ieee80211OfdmRadio);
}
