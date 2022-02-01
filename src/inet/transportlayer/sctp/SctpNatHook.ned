//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.transportlayer.sctp;

//
// SctpNatHook module.
//
simple SctpNatHook
{
    parameters:
        string interfaceTableModule;
        string networkProtocolModule;
        string routingTableModule;
        string natTableModule;
}

