//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.authentication;

import inet.protocolelement.contract.IProtocolHeaderChecker;
import inet.queueing.base.PacketFilterBase;

//
// Verifies and removes Message Authentication Code (MAC) headers from packets.
// Processes packets by removing a configurable-length header (default 4 bytes)
// that was previously inserted by MessageAuthenticationCodeInserter. Works as a
// packet filter that accepts all packets (no actual cryptographic verification
// is performed). Implements a simplified authentication mechanism without actual
// cryptographic operations, serving as a placeholder for more complex security
// implementations in network simulations.
//
simple MessageAuthenticationCodeChecker extends PacketFilterBase like IProtocolHeaderChecker
{
    parameters:
        int headerLength @unit(b) = default(4B);
        @class(MessageAuthenticationCodeChecker);
        @display("i=block/checker");
}
