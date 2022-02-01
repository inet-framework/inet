//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

module EthernetStreamingMacLayer extends EthernetMacLayer
{
    parameters:
        server.typename = default("");
        streamer.typename = default("PacketStreamer");
        transmitter.typename = default("StreamingTransmitter");
        receiver.typename = default("DestreamingReceiver");
}
