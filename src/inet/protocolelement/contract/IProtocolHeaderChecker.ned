//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.contract;

import inet.queueing.contract.IPacketFilter;

//
// Module interface for protocol header checker components. Modules that
// implement this interface verify protocol-specific headers in packets and
// typically remove them after successful validation. These components act as
// packet filters that can drop invalid packets based on header information,
// ensuring protocol compliance and integrity. They are commonly used in
// protocol stacks to process incoming packets before passing them to higher
// layers.
//
moduleinterface IProtocolHeaderChecker extends IPacketFilter
{
    parameters:
        @display("i=block/checker");
}

