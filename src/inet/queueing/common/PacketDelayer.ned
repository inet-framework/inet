//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.base.PacketDelayerBase;
import inet.queueing.contract.IPacketDelayer;

//
// This module connects one packet producer to one packet consumer. It can be
// pushed with packets from the connected packet producer. When this happens,
// the packet is delayed and eventually pushed to the output. Note that the
// order of packets may change if the delay parameter is configured to be a
// distribution.
//
simple PacketDelayer extends PacketDelayerBase like IPacketDelayer
{
    parameters:
        volatile double delay @unit(s); // specifies delay per packet
        volatile double bitrate @unit(bps) = default(inf bps); // determines additional delay per bit
        @class(PacketDelayer);
}
