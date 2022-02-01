//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.lifetime;

import inet.protocolelement.contract.IPacketLifeTimer;

simple CarrierBasedLifeTimer like IPacketLifeTimer
{
    parameters:
        string collectionModule;
        @display("i=block/timer");
        @class(CarrierBasedLifeTimer);
        @signal[packetDropped](type=inet::Packet);
        @statistic[packetDropInterfaceDown](title="packet drop: interface down"; source=packetDropReasonIsInterfaceDown(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
}
