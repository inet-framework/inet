//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.packetlevel.errormodel;

import inet.physicallayer.wireless.common.base.packetlevel.ErrorModelBase;

module Ieee80211YansErrorModel extends ErrorModelBase
{
    parameters:
        @class(Ieee80211YansErrorModel);
        @display("i=block/broadcast");
}

