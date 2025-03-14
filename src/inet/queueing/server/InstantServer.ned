//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.server;

import inet.queueing.base.PacketServerBase;
import inet.queueing.contract.IPacketServer;

//
// Zero-delay packet server that immediately forwards packets from provider to
// consumer without introducing any processing time. This module models an ideal
// server with infinite processing capacity.
//
// The InstantServer is functionally equivalent to a direct connection between
// a packet provider and consumer, but maintains the server interface for
// architectural consistency. It pulls packets from its input and immediately
// pushes them to its output in the same simulation event.
//
// @see ~PacketServer, ~PreemptingServer, ~TokenBasedServer
//
simple InstantServer extends PacketServerBase like IPacketServer
{
    parameters:
        int serveSchedulingPriority = default(-1); // Specifies the FES scheduling priority for the extra event that is pulling the packet, -1 means no extra event
        @class(InstantServer);
}
