//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketDelayerBase;
import inet.queueing.contract.IPacketDelayer;

simple EthernetCutthroughBarrier extends PacketDelayerBase like IPacketDelayer
{
    parameters:
        @class(EthernetCutthroughBarrier);
}
