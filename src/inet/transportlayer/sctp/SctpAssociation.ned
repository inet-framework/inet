//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.sctp;

import inet.common.SimpleModule;

//
// Represents a single SCTP association in the SCTP protocol implementation.
// Implements the SCTP state machine and handles association establishment,
// data transfer, flow control, congestion control, and association termination.
// Provides extensive statistics collection for various SCTP metrics.
//
simple SctpAssociation extends SimpleModule {
    parameters:
        @class(SctpAssociation);
        @display("i=block/socket");
}
