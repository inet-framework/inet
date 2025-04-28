//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.transportlayer.sctp;

import inet.common.SimpleModule;

//
// Inserts SCTP checksum into packets.
//
simple SctpChecksumInsertion extends SimpleModule
{
    parameters:
        @class(SctpChecksumInsertion);
}

