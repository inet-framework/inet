//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.apsk.packetlevel.errormodel;

import inet.physicallayer.wireless.common.base.packetlevel.ErrorModelBase;

//
// This error model determines packet error rate, bit error rate, and symbol
// error rate by using the well-known formula that corresponds to the modulation.
// It assumes no forward error correction or any other techinque is used in the
// physical signal.
//
module ApskErrorModel extends ErrorModelBase
{
    parameters:
        @class(ApskErrorModel);
}

