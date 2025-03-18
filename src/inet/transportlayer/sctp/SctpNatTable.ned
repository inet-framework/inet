//
// Copyright (C) 2008 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//TODO review
package inet.transportlayer.sctp;

import inet.common.SimpleModule;

//
// Maintains a table of Network Address Translation (NAT) entries for SCTP
// connections. Each entry stores local, global, and natted addresses and ports,
// along with verification tags. Provides functionality to find, retrieve, and
// remove entries, facilitating address translation for SCTP connections passing
// through NAT devices.
//
simple SctpNatTable extends SimpleModule
{
    parameters:
        @class(SctpNatTable);
}
