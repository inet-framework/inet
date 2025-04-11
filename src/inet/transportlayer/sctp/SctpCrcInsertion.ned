//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.transportlayer.sctp;

import inet.common.SimpleModule;

//
// Inserts SCTP CRC into packets.
//
simple SctpCrcInsertion extends SimpleModule
{
    parameters:
        @class(SctpCrcInsertion);
}

