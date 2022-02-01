//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.transceiver;

import inet.protocolelement.transceiver.base.StreamingTransmitterBase;
import inet.protocolelement.transceiver.contract.IPacketTransmitter;

//
// This module receives packets from the upper layer as a whole. It sends signals
// to the transmission medium (wire) as a stream, it sends the signal start and
// signal end separately.
//
simple StreamingTransmitter extends StreamingTransmitterBase like IPacketTransmitter
{
    parameters:
        @class(StreamingTransmitter);
}

