//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.queue;

import inet.queueing.base.PacketQueueBase;
import inet.queueing.contract.IPacketQueue;

//
// Configurable packet queue suitable for use in MAC protocols, traffic conditioning,
// and quality of service implementations. This passive component receives packets
// pushed to its input and provides packets when pulled from its output.
//
// The queue can be used standalone or combined with other queueing components
// such as servers, classifiers, schedulers, and multiplexers to create more
// complex queueing behaviors through composition.
//
// By default, this module acts as a standard FIFO queue with an infinite
// internal buffer. Nevertheless, it can also be configured to limit the number
// of packets and the total data length of packets in the internal buffer.
// If the queue becomes overloaded by surpassing the limits of the internal
// buffer, an error is raised unless a packet drop algorithm is configured.
//
// When a packet drop algorithm is used, then one or more packets are dropped
// until the storage limits are met according to the dropping strategy. Packets
// are only dropped after the new packet has been inserted into the queue. This
// method allows higher priority packets to take precedence over already stored
// lower priority packets even if the queue is already full. If a packet dropper
// function is not used and any queue capacity parameter is specified then the
// queue provides back pressure towards its source.
//
// The queue can also keep the packets sorted according to a comparator function.
// If a comparator is not configured, then packets are pushed at the back of the
// queue and they are kept in this order. Packets are always pulled at the front
// of the queue.
//
// It's also possible to use an external buffer, which can be shared among multiple
// queues. In this case, the storage limits are configured on the packet buffer,
// which also takes care of dropping packets from either this queue or some
// other queues as necessary.
//
// Some often used packet queue variants such as ~DropTailQueue are implemented
// as derived modules.
//
// Some notable packet dropper functions are: ~PacketAtCollectionBeginDropper,
// ~PacketAtCollectionEndDropper, ~PacketWithHighestOwnerModuleIdDropper,
// ~PacketWithLowestOwnerModuleIdDropper.
//
// Some notable packet comparator functions are: ~PacketUserPriorityComparator,
// ~PacketCreationTimeComparator, ~PacketEligibilityTimeComparator.
//
// @see ~IPacketBuffer
//
simple PacketQueue extends PacketQueueBase like IPacketQueue
{
    parameters:
        int packetCapacity = default(-1); // Maximum number of packets in the queue, no limit by default
        int dataCapacity @unit(b) = default(-1b); // Maximum total length of packets in the queue, no limit by default
        string dropperClass = default(""); // Determines which packets are dropped when the queue is overloaded, packets are not dropped by default; the parameter must be the name of a C++ class which implements the IPacketDropperFunction C++ interface and is registered via Register_Class
        string comparatorClass = default(""); // Determines the order of packets in the queue, insertion order by default; the parameter must be the name of a C++ class which implements the IPacketComparatorFunction C++ interface and is registered via Register_Class
        string bufferModule = default(""); // Relative module path to the IPacketBuffer module used by this queue, implicit buffer by default
        displayStringTextFormat = default("contains %p pk (%l) pushed %u\npulled %o removed %r dropped %d"); // See ~PacketQueueBase for available format specifiers
        @class(PacketQueue);
        @signal[packetPushStarted](type=inet::Packet);
        @signal[packetPushEnded](type=inet::Packet?);
        @signal[packetPulled](type=inet::Packet);
        @signal[packetRemoved](type=inet::Packet);
        @signal[packetDropped](type=inet::Packet);
        // the statistical value is the number of packets in the queue
        @statistic[queueLength](title="queue length"; source=warmup(atomic(constant0(packetPushEnded) + count(packetPushStarted) - count(packetPulled) - count(packetRemoved) - count(packetDropped))); record=last,max,timeavg,vector; unit=pk; interpolationmode=sample-hold; autoWarmupFilter=false);
        // the statistical value is the total bit length of all packets in the queue
        @statistic[queueBitLength](title="queue bit length"; source=warmup(atomic(constant0(packetPushEnded) + sum(packetLength(packetPushStarted)) - sum(packetLength(packetPulled)) - sum(packetLength(packetRemoved)) - sum(packetLength(packetDropped)))); record=last,max,timeavg,vector; unit=b; interpolationmode=sample-hold; autoWarmupFilter=false);
        // the statistical value is the queueing time of packets
        @statistic[queueingTime](title="queueing times"; source=queueingTime(packetPulled); record=histogram,vector; unit=s; interpolationmode=none);
        // the statistical value is the incoming packet
        @statistic[incomingPackets](title="incoming packets"; source=packetPushStarted; record=count; unit=pk);
        // the statistical value is the length of the incoming packet
        @statistic[incomingPacketLengths](title="incoming packet lengths"; source=packetLength(packetPushStarted); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the data rate of the incoming packets
        @statistic[incomingDataRate](title="incoming datarate"; source=throughput(packetPushStarted); record=vector; unit=bps; interpolationmode=linear);
        // the statistical value is the outgoing packet
        @statistic[outgoingPackets](title="outgoing packets"; source=packetPulled; record=count; unit=pk);
        // the statistical value is the length of the outgoing packet
        @statistic[outgoingPacketLengths](title="outgoing packet lengths"; source=packetLength(packetPulled); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the data rate of the outgoing packets
        @statistic[outgoingDataRate](title="outgoing datarate"; source=throughput(packetPulled); record=vector; unit=bps; interpolationmode=linear);
        // the statistical value is the packet that is dropped due to queue overflow
        @statistic[droppedPacketsQueueOverflow](title="dropped packets: queue overflow"; source=packetDropReasonIsQueueOverflow(packetDropped); record=count; unit=pk; interpolationmode=none);
        // the statistical value is the length of the packet that is dropped due to queue overflow
        @statistic[droppedPacketLengthsQueueOverflow](title="dropped packet lengths: queue overflow"; source=packetLength(packetDropReasonIsQueueOverflow(packetDropped)); record=sum,vector; unit=b; interpolationmode=none);
        // the statistical value is the flow specific queueing time of packets
        @statistic[flowQueueingTime](title="flow queueing times"; source=queueingTime(demuxFlow(packetPulled)); record=histogram,vector; unit=s; interpolationmode=none);
        // the statistical value is the flow specific data rate of the incoming packets
        @statistic[flowIncomingDataRate](title="flow specific incoming data rate"; source=throughput(flowPacketLength(demuxFlow(packetPushStarted))); record=vector; unit=bps; interpolationmode=linear);
        // the statistical value is the flow specific data rate of the outgoing packets
        @statistic[flowOutgoingDataRate](title="flow specific outgoing data rate"; source=throughput(flowPacketLength(demuxFlow(packetPulled))); record=vector; unit=bps; interpolationmode=linear);
        @defaultStatistic(queueLength:vector);
}
