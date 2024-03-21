//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.queueing.filter;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;


// This module implements a packet filter that drops packets when the following module (consumer) does not allow packets
// to be pushed (i.e. consumer->canPushPacket() returns false).
// The module itself accepts all packets (i.e. canPushPacket() and canPushSomePacket() always return true).
// This is useful when the following module is a gate to filter out packets when the gate is closed.
simple BackPressureBasedFilter extends PacketFilterBase like IPacketFilter
{
    parameters:
        @class(BackPressureBasedFilter);
}
