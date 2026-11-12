//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.prototype.cellular;

import inet.physicallayer.wireless.common.base.packetlevel.ErrorModelBase;

//
// A dummy transport block error model for ~CellularReceiver. The effective SNIR is the linear
// mean of the SNIR over the (symbol, PRB) cells of the allocation, and the block error rate is
// 1 / (1 + exp(slope * margin)), where the margin is the effective SNIR minus the required SNIR
// of the MCS, in dB. The decision is one per transport block; regions with a frequency
// dimension (change C14) would allow one per code block group.
//
module CellularErrorModel extends ErrorModelBase
{
    parameters:
        double slope = default(1.5); // Steepness of the block error rate curve, per dB
        @class(CellularErrorModel);
}

