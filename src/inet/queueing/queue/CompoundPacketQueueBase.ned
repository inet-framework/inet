//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.queue;

import inet.queueing.base.PacketQueueBase;
import inet.queueing.contract.IPacketQueue;

//
// This compound module serves as a base module for complex packet queues formed
// by combining several queueing components.
//
module CompoundPacketQueueBase extends PacketQueueBase like IPacketQueue
{
    parameters:
        int packetCapacity = default(-1); // maximum number of packets in the queue, no limit by default
        int dataCapacity @unit(b) = default(-1b); // maximum total length of packets in the queue, no limit by default
        string dropperClass = default(""); // determines which packets are dropped when the queue is overloaded, packets are not dropped by default; the parameter must be the name of a C++ class which implements the IPacketDropperFunction C++ interface and is registered via Register_Class
        displayStringTextFormat = default("contains %p pk (%l) pushed %u created %c\n pulled %o removed %r dropped %d");
        @class(CompoundPacketQueueBase);
        @defaultStatistic(queueLength);
        @signal[packetPushStarted](type=inet::Packet);
        @signal[packetPushEnded](type=inet::Packet?);
        @signal[packetPulled](type=inet::Packet);
        @signal[packetRemoved](type=inet::Packet);
        @signal[packetDropped](type=inet::Packet);
        // the statistical value is the number of packets in the queue
        @statistic[queueLength](title="queue length"; source=warmup(atomic(constant0(localSignal(packetPushEnded)) + count(localSignal(packetPushStarted)) - count(localSignal(packetPulled)) - count(localSignal(packetRemoved)) - count(localSignal(packetDropped)))); record=last,max,timeavg,vector; interpolationmode=sample-hold; unit=pk; autoWarmupFilter=false);
        // the statistical value is the total bit length of all packets in the queue
        @statistic[queueBitLength](title="queue bit length"; source=warmup(atomic(constant0(localSignal(packetPushEnded)) + sum(packetLength(localSignal(packetPushStarted))) - sum(packetLength(localSignal(packetPulled))) - sum(packetLength(localSignal(packetRemoved))) - sum(packetLength(localSignal(packetDropped))))); record=last,max,timeavg,vector; unit=b; interpolationmode=sample-hold; autoWarmupFilter=false);
        // the statistical value is the queueing time of packets
        @statistic[queueingTime](title="queueing times"; source=queueingTime(localSignal(packetPulled)); record=histogram,vector; unit=s; interpolationmode=none);
        // the statistical value is the incoming packet
        @statistic[incomingPackets](title="incoming packets"; source=localSignal(packetPushStarted); record=count; unit=pk);
        // the statistical value is the length of the incoming packet
        @statistic[incomingPacketLengths](title="incoming packet lengths"; source=packetLength(localSignal(packetPushStarted)); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the data rate of the incoming packets
        @statistic[incomingDataRate](title="incoming datarate"; source=throughput(localSignal(packetPushStarted)); record=vector; unit=bps; interpolationmode=linear);
        // the statistical value is the outgoing packet
        @statistic[outgoingPackets](title="outgoing packets"; source=localSignal(packetPulled); record=count; unit=pk);
        // the statistical value is the length of the outgoing packet
        @statistic[outgoingPacketLengths](title="outgoing packet lengths"; source=packetLength(localSignal(packetPulled)); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the data rate of the outgoing packets
        @statistic[outgoingDataRate](title="outgoing datarate"; source=throughput(localSignal(packetPulled)); record=vector; unit=bps; interpolationmode=linear);
        // the statistical value is the packet that is dropped due to queue overflow
        @statistic[droppedPacketsQueueOverflow](title="dropped packets: queue overflow"; source=packetDropReasonIsQueueOverflow(localSignal(packetDropped)); record=count; unit=pk; interpolationmode=none);
        // the statistical value is the length of the packet that is dropped due to queue overflow
        @statistic[droppedPacketLengthsQueueOverflow](title="dropped packet lengths: queue overflow"; source=packetLength(packetDropReasonIsQueueOverflow(localSignal(packetDropped))); record=sum,vector; unit=b; interpolationmode=none);
        // the statistical value is the flow specific queueing time of packets
        @statistic[flowQueueingTime](title="flow queueing times"; source=queueingTime(demuxFlow(localSignal(packetPulled))); record=histogram,vector; unit=s; interpolationmode=none);
        // the statistical value is the flow specific data rate of the incoming packets
        @statistic[flowIncomingDataRate](title="flow specific incoming data rate"; source=throughput(flowPacketLength(demuxFlow(localSignal(packetPushStarted)))); record=vector; unit=bps; interpolationmode=linear);
        // the statistical value is the flow specific data rate of the outgoing packets
        @statistic[flowOutgoingDataRate](title="flow specific outgoing data rate"; source=throughput(flowPacketLength(demuxFlow(localSignal(packetPulled)))); record=vector; unit=bps; interpolationmode=linear);
        @defaultStatistic(queueLength:vector);
}
