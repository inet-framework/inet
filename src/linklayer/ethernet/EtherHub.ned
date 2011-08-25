//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
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


package inet.linklayer.ethernet;

//
// A generic wiring hub model.
//
// ethg[i] gates represent ports. Messages arriving on a port are broadcast
// to every other port.
//
// Messages are not interpreted by the hub model in any way, thus the hub
// model is not specific to Ethernet. Messages may represent anything, from
// the beginning of a frame transmission to end (or abortion) of transmission.
//
// The module checks equivalence of input/output datarates only in initialize() function.
// The module don't work correctly when you modify some datarates or add new connections
// with different datarates.
//

simple EtherHub
{
    parameters:
        @node();
        @labels(node,ethernet-node);
        @display("i=device/hub");
        @signal[pk](type=cMessage);
        @statistic[pk](title="packets"; source=pk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        inout ethg[] @labels(EtherFrame-conn);  // to stations; each one represents a port
}
