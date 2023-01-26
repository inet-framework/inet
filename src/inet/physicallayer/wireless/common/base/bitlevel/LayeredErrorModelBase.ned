//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.base.bitlevel;

import inet.physicallayer.wireless.common.contract.bitlevel.ILayeredErrorModel;

module LayeredErrorModelBase like ILayeredErrorModel
{
    parameters:
        string symbolCorruptionMode @enum("1bit", "uniform") = default("1bit");
        @display("i=block/broadcast");
}

