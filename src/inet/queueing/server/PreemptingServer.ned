//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.server;

import inet.queueing.base.PacketServerBase;
import inet.queueing.contract.IPacketServer;

//
// Preemptive packet server that can interrupt the processing of the current packet
// to handle a higher priority packet. This module models servers that can suspend
// ongoing work to process more urgent tasks.
//
// Operation mechanism:
// - When a new packet arrives while another is being processed, the server
//   can preempt (interrupt) the current packet's processing
// - The preempted packet's processing is suspended and its state is saved
// - The new packet is processed at the configured datarate
// - After the preempting packet is completed, the server can resume processing
//   of previously preempted packets
//
// The datarate parameter determines how fast packets are processed, affecting
// the processing time of each packet based on its length.
//
// @see ~PacketServer, ~InstantServer, ~TokenBasedServer
//
simple PreemptingServer extends PacketServerBase like IPacketServer
{
    parameters:
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        double datarate @unit(bps);
        @class(PreemptingServer);
}
