//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


package inet.examples.inet.routerperf;

import inet.applications.generic.IIpvxTrafficGenerator;
import inet.node.base.TransportLayerNodeBase;

//
// Definition of an IP node with a transport generator that connects
// to IP directly, without TCP or UDP.
//
module BurstHost extends TransportLayerNodeBase
{
    parameters:
        @networkNode();
        @display("i=device/pc3");
        hasUdp = false;
        hasTcp = false;
        *.routingTableModule = default("^.ipv4.routingTable");
    submodules:
        trafGen: <> like IIpvxTrafficGenerator {
            parameters:
                @display("p=329,81");
        }
    connections allowunconnected:
        // transport connections
        trafGen.ipOut --> tn.in++;
        trafGen.ipIn <-- tn.out++;
}

