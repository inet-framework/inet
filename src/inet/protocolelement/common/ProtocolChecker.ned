//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.common;

import inet.queueing.base.PacketFilterBase;

//
// Filters packets based on their protocol. This module maintains a set of
// registered protocols and checks if a packet has a protocol tag and if the
// protocol is in the registered set. It passes through packets that match a
// registered protocol and drops packets that don't have a protocol tag or have
// a protocol that's not registered. Protocols can be registered dynamically
// through the handleRegisterProtocol method. The module emits a signal when a
// packet is dropped due to protocol mismatch.
//
simple ProtocolChecker extends PacketFilterBase
{
    parameters:
        @class(ProtocolChecker);
        @display("i=block/checker");
        @signal[packetDropped](type=inet::Packet);
}
