//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.transceiver;

import inet.protocolelement.transceiver.base.StreamingReceiverBase;
import inet.protocolelement.transceiver.contract.IPacketReceiver;

//
// This module receives signals from the physical medium (wire) as a whole. It
// also sends up packets to the upper layer as a whole.
//
simple PacketReceiver extends StreamingReceiverBase like IPacketReceiver
{
    parameters:
        @class(PacketReceiver);
}

