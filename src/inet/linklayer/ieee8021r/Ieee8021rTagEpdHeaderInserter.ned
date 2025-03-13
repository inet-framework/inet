//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021r;

import inet.queueing.base.PacketFlowBase;

//
// Implements the IEEE 802.1r R-TAG header insertion functionality for the Frame
// Replication and Elimination for Reliability (FRER) protocol defined in IEEE 802.1CB.
// This module adds sequence numbers to outgoing frames, enabling receivers to
// identify and eliminate duplicate frames.
//
// The module inserts an R-TAG header in the EtherType Protocol Discrimination (EPD)
// format, which includes:
// - A reserved field (16 bits, set to 0)
// - A sequence number field (16 bits, incremented for each frame)
// - The EtherType/Length field (16 bits) for the encapsulated protocol
//
// The sequence numbers are essential for duplicate frame identification and
// elimination at the receiving end. By default, this module is configured to
// work with IEEE 802.1Q C-TAG headers as the next protocol in the stack, but
// this can be configured through the nextProtocol parameter.
//
// This module is a key component in implementing seamless redundancy for
// time-sensitive networking applications where zero recovery time is required
// in case of network failures.
//
// @see ~IIeee8021rTagEpdHeaderInserter, ~Ieee8021rTagEpdHeaderChecker
//
simple Ieee8021rTagEpdHeaderInserter extends PacketFlowBase like IIeee8021rTagEpdHeaderInserter
{
    parameters:
        string nextProtocol = default("ieee8021qctag");  // Protocol identifier for the next header in the stack
        @class(Ieee8021rTagEpdHeaderInserter);
        @display("i=block/inserter");
}
