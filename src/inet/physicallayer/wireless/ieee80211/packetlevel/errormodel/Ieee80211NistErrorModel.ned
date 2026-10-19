//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.packetlevel.errormodel;

import inet.physicallayer.wireless.common.base.packetlevel.ErrorModelBase;
import inet.physicallayer.wireless.ieee80211.contract.packetlevel.IIeee80211AmpduErrorModel;

module Ieee80211NistErrorModel extends ErrorModelBase like IIeee80211AmpduErrorModel
{
    parameters:
        @class(Ieee80211NistErrorModel);
        @display("i=block/broadcast");
}

