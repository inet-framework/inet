//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IActivePacketSource;

//
// This module connects multiple packet producers to one packet consumer. When
// a packet is pushed by one of the connected packet producers, then the multiplexer
// simply pushes the packet to the connected packet consumer.
//
simple PacketMultiplexer extends PacketProcessorBase like IActivePacketSource // TODO IPassivePacketSink // in gate is vector
{
    parameters:
        bool forwardServiceRegistration = default(true);
        bool forwardProtocolRegistration = default(true);
        displayStringTextFormat = default("passed %p pk (%l)"); // %p number of packet, %l total length
        @class(PacketMultiplexer);
        @display("i=-;b=10,10,oval,grey,,1");
        @signal[packetPushed](type=inet::Packet);
        @signal[packetPulled](type=inet::Packet);
        // the statistical value is the multiplexed packet
        @statistic[packets](title="packets"; source=merge(packetPushed, packetPulled); record=count; unit=pk);
        // the statistical value is the length of the multiplexed packet
        @statistic[packetLengths](title="packet lengths"; source=packetLength(merge(packetPushed, packetPulled)); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the data rate of the multiplexed packets
        @statistic[dataRate](title="data rate"; source=throughput(merge(packetPushed, packetPulled)); record=vector; unit=bps; interpolationmode=linear);
    gates:
        input in[] @labels(push);
        output out @labels(push);
}
