//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.lifetime;

import inet.common.SimpleModule;
import inet.protocolelement.contract.IPacketLifeTimer;

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
