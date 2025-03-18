//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.lifetime;

import inet.common.SimpleModule;
import inet.protocolelement.contract.IPacketLifeTimer;

//
// Monitors a packet collection and drops all packets when the network interface
// loses carrier. This module subscribes to interface state change signals and
// packet push signals. When the interface's carrier state changes to down or
// when a packet is pushed to the collection while the interface has no carrier,
// all packets in the collection are dropped with an "interface down" reason.
// This is useful for protocols that need to clear queued packets when a link
// becomes unavailable.
//
simple CarrierBasedLifeTimer extends SimpleModule like IPacketLifeTimer
{
    parameters:
        @class(CarrierBasedLifeTimer);
        string collectionModule;
        @display("i=block/timer");
        @class(CarrierBasedLifeTimer);
        @signal[packetDropped](type=inet::Packet);
        @statistic[packetDropInterfaceDown](title="packet drop: interface down"; source=packetDropReasonIsInterfaceDown(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
}
