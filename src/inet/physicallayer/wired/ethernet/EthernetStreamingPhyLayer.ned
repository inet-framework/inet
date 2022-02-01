//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wired.ethernet;

module EthernetStreamingPhyLayer extends EthernetPhyLayer
{
    parameters:
        transmitter.typename = default("StreamingTransmitter");
        receiver.typename = default("DestreamingReceiver");
}
