//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wired.ethernet;

//
// Implements an Ethernet PHY layer that uses streaming communication in both
// directions. Packets are received from the upper layer as a stream and signals
// are sent to the transmission medium as a stream. Similarly, signals are
// received from the medium as a stream and delivered to the upper layer as a
// stream. Supports signal preemption for frame preemption capabilities.
//
module EthernetStreamThroughPhyLayer extends EthernetPhyLayer
{
    parameters:
        transmitter.typename = default("StreamThroughTransmitter");
        receiver.typename = default("StreamThroughReceiver");
}
