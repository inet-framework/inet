//
//   Copyright (C) 2009 by Alfonso Ariza
//   Malaga University
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

package inet.networklayer.manetrouting.base;

//
// Base module for mobile routing protocols
//
simple BaseRouting
{
    parameters:
        @display("i=block/routing");
        string excludedInterfaces = default(""); // list of interfaces excluded by the manet routing protocol
        string interfaces = default("prefix(wlan)"); // list of routing interfaces were the routing protocol is active, the
                                                // prefix(namei) indicate all the interfaces with "namei" in his names
        bool usetManetLabelRouting = default(true); // the routing entries are marked IPv4Route::MANET, this label force an exact search with independent of the mask
        bool useICMP = default(true); // By default the module use the ICMP but it's necessary to activate with the method setSendToICMP(true)
        bool setICMPSourceAddress = default(false); // set the datagram source address if the address is undefined to the address of node
                                                 // This parameter allow that the ICMP messages could arrive to the source applications
        bool manetPurgeRoutingTables = default(true);
        bool autoassignAddress = default(false); // assign IP adresses automatically to the interfaces
        string autoassignAddressBase = default("10.0.0.0");
}
