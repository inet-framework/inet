//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.packetlevel.errormodel;

import inet.physicallayer.wireless.common.base.packetlevel.ErrorModelBase;

module Ieee80211BerTableErrorModel extends ErrorModelBase
{
    parameters:
        string opMode @enum("a", "b", "g(erp)", "g(mixed)", "n", "p");
        string berTableFile;
        @class(Ieee80211BerTableErrorModel);
        @display("i=block/broadcast");
}

