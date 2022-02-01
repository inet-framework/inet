//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wired.ethernet;

module EthernetStreamThroughPhyLayer extends EthernetPhyLayer
{
    parameters:
        transmitter.typename = default("StreamThroughTransmitter");
        receiver.typename = default("StreamThroughReceiver");
}
