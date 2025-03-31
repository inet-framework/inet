//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wired.ethernet;

//
// Implements an Ethernet PHY layer that uses streaming communication for signal
// transmission. Packets are received from the upper layer as a whole, but signals
// are sent to the transmission medium as a stream with separate signal start and
// end events. Similarly, signals are received from the medium as a stream but
// delivered to the upper layer as complete packets.
//
module EthernetStreamingPhyLayer extends EthernetPhyLayer
{
    parameters:
        transmitter.typename = default("StreamingTransmitter");
        receiver.typename = default("DestreamingReceiver");
}
