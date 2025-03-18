//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.transceiver;

import inet.protocolelement.transceiver.base.StreamingTransmitterBase;
import inet.protocolelement.transceiver.contract.IPacketTransmitter;

//
// Receives packets from the upper layer as a stream and also
// sends signals to the transmission medium as a stream. The stream start and
// stream end are sent separately allowing the preemption of signals.
//
simple StreamThroughTransmitter extends StreamingTransmitterBase like IPacketTransmitter
{
    parameters:
        @class(StreamThroughTransmitter);
}

