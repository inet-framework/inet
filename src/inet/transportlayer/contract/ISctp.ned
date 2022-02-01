//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2010 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.transportlayer.contract;

//
// Interface for SCTP protocol.
//
moduleinterface ISctp
{
    @display("i=block/wheelbarrow");
    gates:
        input appIn @labels(SctpCommand/down);
        input ipIn;
        output appOut @labels(SctpCommand/up);
        output ipOut;
}

