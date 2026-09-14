//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Interface for packet queue modules. A packet queue
// is a passive module which has one passive input and one passive output.
// Packets pushed into the passive input are either stored or dropped. Packets
// pulled from the passive output are provided from the stored packets.
//
moduleinterface IPacketQueue extends IPassivePacketSink, IPassivePacketSource
{
    parameters:
        @signal[packetQueueDeparture](type=inet::Packet); // Details: PacketQueueRemovalDetails; source identifies the logical queue
        @omittedTypename(OmittedPacketQueue);
        @display("i=block/queue");
}
