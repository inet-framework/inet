//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.bitlevel;

import inet.physicallayer.wireless.common.base.packetlevel.FlatRadioBase;

module Ieee80211OfdmRadio extends FlatRadioBase
{
    parameters:
        transmitter.typename = default("Ieee80211LayeredOfdmTransmitter");
        receiver.typename = default("Ieee80211LayeredOfdmReceiver");
        receiver.errorModel.typename = default("Ieee80211OfdmErrorModel");
        @class(Ieee80211OfdmRadio);
}

