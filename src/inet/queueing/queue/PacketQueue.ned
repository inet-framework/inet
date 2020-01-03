//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

package inet.queueing.queue;

import inet.queueing.base.PacketQueueBase;
import inet.queueing.contract.IPacketQueue;

//
// This module implements a widely configurable packet queue, which is suitable
// among others, to be used in MAC protocols, traffic conditioning, and quality
// of services. This module can be used on its own, but it's also often supplemented
// by additional queueing components such as servers, classifiers, schedulers,
// multiplexers, etc. This kind of composition allows to form a larger module
// which can act as a packet queue with more complex behavior.
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
// lower priority packets even if the queue is already full.
//
// The queue can also keep the packets sorted according to a comparator function.
// If a comparator is not configured, then packets are pushed at the back of the
// queue and they are kept in this order. Packets are always popped at the front
// of the queue.
//
// It's also possible to use an external buffer, which can be shared among multiple
// queues. In this case, the storage limits are configured on the packet buffer,
// which also takes care of dropping packets from either this queue or some
// other queues as necessary.
//
// @see ~IPacketBuffer
//
simple PacketQueue extends PacketQueueBase like IPacketQueue
{
    parameters:
        int packetCapacity = default(-1); // maximum number of packets in the queue, no limit by default
        int dataCapacity @unit(b) = default(-1b); // maximum total length of packets in the queue, no limit by default
        string dropperClass = default(""); // determines which packets are dropped when the queue is overloaded, packets are not dropped by default; the parameter must be the name of a C++ class which implements the IPacketDropperFunction C++ interface and is registered via Register_Class
        string comparatorClass = default(""); // determines the order of packets in the queue, insertion order by default; the parameter must be the name of a C++ class which implements the IPacketComparatorFunction C++ interface and is registered via Register_Class
        string bufferModule = default(""); // relative module path to the IPacketBuffer module used by this queue, implicit buffer by default
        displayStringTextFormat = default("contains %p pk (%l) pushed %u\npopped %o removed %r dropped %d");
        @class(PacketQueue);
        @signal[packetPushed](type=inet::Packet);
        @signal[packetPopped](type=inet::Packet);
        @signal[packetRemoved](type=inet::Packet);
        @signal[packetDropped](type=inet::Packet);
        @statistic[packetPushed](title="packet pushed"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetPopped](title="packet popped"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetRemoved](title="packets removed"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropQueueOverflow](title="packet drops: queue overflow"; source=packetDropReasonIsQueueOverflow(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[queueingTime](title="queueing times"; source=queueingTime(packetPopped); record=histogram,vector; unit=s; interpolationmode=none);
        @statistic[queueLength](title="queue length"; source=count(packetPushed) - count(packetPopped) - count(packetRemoved) - count(packetDropped); record=max,timeavg,vector; interpolationmode=sample-hold);
}
