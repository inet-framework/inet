//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.bitlevel;

import inet.common.SimpleModule;
import inet.physicallayer.wireless.common.contract.bitlevel.IDemodulator;

//
// @see ~Ieee80211OfdmModulator
//
simple Ieee80211OfdmDemodulator extends SimpleModule like IDemodulator
{
    parameters:
        @class(Ieee80211OfdmDemodulatorModule);
        int numSubcarriers = default(52);
        string subcarrierModulation;
        @display("i=block/mac");
}

