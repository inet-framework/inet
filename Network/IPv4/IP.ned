//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


//
// Implements the \IP protocol. This is the all-in-one version of IPProcessing.
//
// The \IP protocol header is represented by the IPDatagram message class.
//
// <b>Interfacing with higher layer protocols</b>
//
// To send a packet over IP from a higher layer protocol, the module should
// fill in an IPControlInfo object, attach it to the packet with cMessage's
// setControlInfo() method, the send it to the IP module.
//
// When IP sends up a packet to a higher layer protocol, it will also attach
// an IPControlInfo to the packet, with the source and destination IP address,
// etc. of the IP datagram in which the packet arrived.
//
// IP can serve several higher-layer protocols. When delivering packets
// to them, the output gate is determined from the Protocol field in the
// \IP header. The protocol-to-gateindex mapping must be given in the
// protocolMapping string parameter, in the following format:
// <pre>
// protocolnumber:gateindex, protocolnumber:gateindex, ...
// </pre>
//
// The recommended setting is:
// - TCP (6) : 0
// - UDP (17) : 1
// - \ICMP (1) : 2
// - \IGMP (2) : 3
// - \RSVP (46) : 4
//
// That is, <tt>protocolMapping="6:0,17:1,1:2,2:3,46:4"</tt>
//
// <b>Routing and interfacing with lower layers</b>
//
// The routing table is stored in the module RoutingTable. When a datagram
// needs to be routed, IP queries RoutingTable for the output interface
// (or "port") and next hop address of the packet. This is done by directly
// calling C++ methods (such as outputPortNo(destAddress)) of RoutingTable.
// No message exchange with RoutingTable takes place.
//
// A routed datagram will be sent to the given interface, either without
// control info or with an IPRoutingDecision attached; if the route entry
// contained a next hop address, it will be passed to the interface
// in the attached IPRoutingDecision.
//
// Routing protocol implementations (e.g. OSPF and ISIS) can also query
// and manipulate the route table by calling RoutingTable's methods in C++.
//
// <b>Performance model, QoS</b>
//
// In the current form, IP contains a FIFO which queues up \IP datagrams;
// datagrams are processed in order. The processing time is determined by the
// procDelay module parameter. 
//
// The current performance model comes from the QueueBase C++ base class.
// If you need a more sophisticated performance model, you may change the 
// module implementation (the IP class), and: (1) override the startService() 
// method which determines processing time for a packet, or (2) use a
// different base class. Or, if you want to implement QoS, you may try the
// QueueWithQoS base class, which allows you to extract forwarding behaviour
// in a separate "qosBehaviorClass", subclassed from the EnqueueHook class.
// QueueWithQoS allows you to switch between different EnqueueHook classes
// from omnetpp.ini.
//
// @see RoutingTable, IPProcessing, IPControlInfo, IPRoutingDecision
//
// @author Andras Varga
//
simple IP
    parameters:
        procDelay : numeric const,
        timeToLive : numeric const,
        multicastTimeToLive : numeric const,
        protocolMapping : string,
        fragmentTimeout : numeric const;
    gates:
        in: transportIn[];
        out: transportOut[];
        in: queueIn[];
        out: queueOut[];
endsimple

