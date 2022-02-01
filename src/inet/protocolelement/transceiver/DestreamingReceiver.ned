//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.transceiver;

import inet.protocolelement.transceiver.base.PacketReceiverBase;
import inet.protocolelement.transceiver.contract.IPacketReceiver;

//
// This module receives signals from the transmission medium (wire) as a stream.
// It receives the signal start and signal end separately. It sends packets to
// the upper layer as a whole.
//
simple DestreamingReceiver extends PacketReceiverBase like IPacketReceiver
{
    parameters:
        @class(DestreamingReceiver);
}

