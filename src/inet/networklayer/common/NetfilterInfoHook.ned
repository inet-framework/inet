//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.common;

import inet.common.SimpleModule;

simple NetfilterInfoHook extends SimpleModule
{
    parameters:
        @class(NetfilterInfoHook);
        string networkProtocolModule;
}

