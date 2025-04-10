//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.common.SimpleModule;

simple EthernetSocketTable extends SimpleModule
{
    parameters:
        @class(EthernetSocketTable);
        @display("i=block/table");
}

