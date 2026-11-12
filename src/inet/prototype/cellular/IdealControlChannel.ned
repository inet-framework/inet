//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.prototype.cellular;

import inet.common.SimpleModule;

//
// The out-of-band control plane of the dummy cellular model: ~GnbMac and ~UeMac modules
// register here and then exchange attachments, uplink grants and SNIR reports by direct method
// calls, without delay and without errors. Control channels (PDCCH, PUCCH) are thus not on the
// radio medium; only the data allocations are.
//
simple IdealControlChannel extends SimpleModule
{
    parameters:
        @class(IdealControlChannel);
        @display("i=block/control");
}

