//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.transceiver;

import inet.protocolelement.transceiver.base.StreamingTransmitterBase;
import inet.protocolelement.transceiver.contract.IPacketTransmitter;

//
// Receives packets from upper layers as whole units but transmits them to
// the physical medium as streams. Separates signal start and end events
// to enable streaming transmission.
//
// Operation:
// - Accepts complete packets from upper layers
// - Converts packets into signal streams
// - Sends signal start and end events separately
// - Enables incremental signal transmission
// - Maintains packet integrity while supporting streaming
//
// Particularly useful for:
// - Implementing streaming transmitters with packet-based inputs
// - Creating interfaces between packet and streaming domains
// - Supporting preemptible transmission systems
// - Building time-sensitive networking components
// - Simulating realistic signal transmission with timing control
//
// @see ~DestreamingReceiver, ~PacketTransmitter, ~StreamThroughTransmitter
//
simple StreamingTransmitter extends StreamingTransmitterBase like IPacketTransmitter
{
    parameters:
        @class(StreamingTransmitter);
}
