//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.transportlayer.tcp_common;

import inet.common.SimpleModule;

//
// Computes and inserts Cyclic Redundancy Check (CRC) values into TCP headers
// during packet processing. Implements a network filter hook that intercepts
// packets in the post-routing phase and handles different CRC modes including
// declared correct, declared incorrect, and computed checksums.
//
simple TcpChecksumInsertionHook extends SimpleModule
{
    parameters:
        @class(TcpChecksumInsertionHook);
}
