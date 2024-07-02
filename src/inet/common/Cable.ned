//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.common;

import ned.DatarateChannel;

channel Cable extends DatarateChannel
{
    parameters:
        string enabledLineStyle = default("solid");
        string disabledLineStyle = default("dashed");
        string disabledIcon = default("status/cross");
        @class(Cable);
}
