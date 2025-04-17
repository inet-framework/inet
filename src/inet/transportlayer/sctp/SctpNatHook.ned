//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.transportlayer.sctp;

import inet.common.SimpleModule;

//
// SCTP NAT hook module.
//
simple SctpNatHook extends SimpleModule
{
    parameters:
        @class(SctpNatHook);
        string interfaceTableModule;
        string networkProtocolModule;
        string routingTableModule;
        string natTableModule;
}

