//
//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


package inet.networklayer.ipv4;

//
// ICMP implementation
//
// Receive ICMP message from localDeliver and process it:
//   - echo/timestamp reply: strip ICMP header and send to pingOut
//   - echo/timestamp request: create reply and send to IPSend
//   - destination unreachable: ignore/send to IPRouting (static)?
//   - time exceeded: ignore ?
//   - parameter problem: ignore ?
//   - redirect ?
//#FIXME question marks above...
//
// Receive echo/timestamp message from pingIn: encapsulate in ICMP header
// and send to IPSend
//
// author: Jochen Reber
// date: 2.5.00, 9.5.00, 20.5.00, 9.6.00
// redesign: Andras Varga 1.3.2004
//
simple ICMP
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        @display("i=block/control_s");
    gates:
        input localIn @labels(IPv4ControlInfo/up);  // delivered ICMP packets
        input pingIn[];  // ping requests from app
        output pingOut[];  // result of ping
        output sendOut @labels(IPv4ControlInfo/down);  // towards network
        output errorOut;  // errors
}

