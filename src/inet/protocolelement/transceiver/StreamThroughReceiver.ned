//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.transceiver;

import inet.protocolelement.transceiver.base.StreamingReceiverBase;
import inet.protocolelement.transceiver.contract.IPacketReceiver;

//
// This module receives signals from the transmission medium (wire) as a stream and
// also sends packets to the upper layer as a stream. The stream start and stream
// end are sent seperately allowing the preemption of signals.
//
simple StreamThroughReceiver extends StreamingReceiverBase like IPacketReceiver
{
    parameters:
        @class(StreamThroughReceiver);
}

