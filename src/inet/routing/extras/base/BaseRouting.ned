//
// Copyright (C) 2009 by Alfonso Ariza
// Malaga University
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.routing.extras.base;
import inet.applications.contract.IApp;

//
// Base module for mobile routing protocols
//
simple BaseRouting like IApp
{
    parameters:
        @class(ManetRoutingBase);
        @display("i=block/routing");
        
        string routingTableModule = default("^.ipv4.routingTable");
        string interfaceTableModule = default("^.interfaceTable");
        string networkProtocolModule = default("^.ipv4.ip");
        string icmpModule = default("^.ipv4.icmp");
        string arpModule = default("^.ipv4.arp");
        
        int UdpPort;
        
        string excludedInterfaces = default(""); // list of interfaces excluded by the manet routing protocol
        string interfaces = default("prefix(wlan)"); // list of routing interfaces were the routing protocol is active, the
                                                // prefix(namei) indicate all the interfaces with "namei" in his names
        bool useManetLabelRouting = default(true); // the routing entries are marked IPv4Route::MANET, this label force an exact search with independent of the mask
        bool useICMP = default(true); // By default the module use the ICMP but it's necessary to activate with the method setSendToICMP(true)
        bool setICMPSourceAddress = default(false); // set the datagram source address if the address is undefined to the address of node
                                                 // This parameter allow that the ICMP messages could arrive to the source applications
        bool manetPurgeRoutingTables = default(true);
        bool autoassignAddress = default(false); // assign IP adresses automatically to the interfaces
        string autoassignAddressBase = default("10.0.0.0");
        bool isStaticNode = default(false);
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}
