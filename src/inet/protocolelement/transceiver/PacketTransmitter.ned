//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.transceiver;

import inet.protocolelement.transceiver.base.PacketTransmitterBase;
import inet.protocolelement.transceiver.contract.IPacketTransmitter;

//
// This module receives packets from the upper layer as a whole. It also sends
// signals to the transmission medium (wire) as a whole.
//
simple PacketTransmitter extends PacketTransmitterBase like IPacketTransmitter
{
    parameters:
        @class(PacketTransmitter);
}
