//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.buffer;

import inet.queueing.base.PacketBufferBase;
import inet.queueing.contract.IPacketBuffer;

//
// Shared packet buffer that can be used by multiple
// packet queues to optimize storage space. It provides centralized packet
// storage with configurable capacity limits and customizable packet dropping
// strategies.
//
// Key features:
// - Configurable packet capacity (maximum number of packets)
// - Configurable data capacity (maximum total bit length of all packets)
// - Customizable packet dropping strategy via the dropperClass parameter
// - Comprehensive statistics collection for monitoring buffer performance
//
// When the buffer becomes overloaded (exceeds capacity limits), the configured
// packet dropping strategy can selectively drop packets from any of the connected
// queues based on various criteria such as packet priority, size, or queue length.
//
// @see ~IPacketQueue, ~PriorityBuffer
//
simple PacketBuffer extends PacketBufferBase like IPacketBuffer
{
    parameters:
        int packetCapacity = default(-1); // Maximum number of packets in the queue, no limit by default
        int dataCapacity @unit(b) = default(-1b); // Maximum total length of packets in the queue, no limit by default
        string dropperClass = default(""); // Determines which packets are dropped when the buffer is overloaded, packets are not dropped by default; the parameter must be the name of a C++ class which implements the IPacketDropperFunction C++ interface and is registered via Register_Class
        @class(PacketBuffer);
        @display("i=block/buffer");
        @signal[packetAdded](type=inet::Packet);
        @signal[packetRemoved](type=inet::Packet);
        @signal[packetDropped](type=inet::Packet);
        // the statistical value is the added packet
        @statistic[addedPackets](title="added packets"; source=packetAdded; record=count; unit=pk);
        // the statistical value is the length of the added packet
        @statistic[addedPacketLengths](title="added packet lengths"; source=packetLength(packetAdded); record=sum,vector; unit=b; interpolationmode=none);
        // the statistical value is the removed packet
        @statistic[removedPackets](title="removed packets"; source=packetRemoved; record=count; unit=pk);
        // the statistical value is the length of the removed packet
        @statistic[removedPacketLengths](title="removed packet lengths"; source=packetLength(packetRemoved); record=sum,vector; unit=pk; interpolationmode=none);
        // the statistical value is the number of packets in the buffer
        @statistic[bufferLength](title="buffer length"; source=warmup(count(packetAdded) - count(packetRemoved) - count(packetDropped)); record=last,max,timeavg,vector; unit=pk; interpolationmode=sample-hold);
        // the statistical value is the total bit length of all packets in the buffer
        @statistic[bufferBitLength](title="buffer bit length"; source=warmup(sum(packetLength(packetAdded)) - sum(packetLength(packetRemoved)) - sum(packetLength(packetDropped))); record=last,max,timeavg,vector; unit=b; interpolationmode=sample-hold; autoWarmupFilter=false);
        // the statistical value is the packet that is dropped due to queue overflow
        @statistic[droppedPacketsQueueOverflow](title="dropped packets: queue overflow"; source=packetDropReasonIsQueueOverflow(packetDropped); record=count; unit=pk; interpolationmode=none);
        // the statistical value is the length of the packet that is dropped due to queue overflow
        @statistic[droppedPacketLengthsQueueOverflow](title="dropped packet lengths: queue overflow"; source=packetLength(packetDropReasonIsQueueOverflow(packetDropped)); record=sum,vector; unit=b; interpolationmode=none);
}
