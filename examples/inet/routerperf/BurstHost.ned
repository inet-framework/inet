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

import inet.applications.generic.IIPvXTrafficGenerator;
import inet.node.inet.NodeBase;


//
// Definition of an IP node with a transport generator that connects
// to IP directly, without TCP or UDP.
//
module BurstHost extends NodeBase
{
    parameters:
        @node();
        @display("i=device/pc3");
        string trafGenType;
    submodules:
        trafGen: <trafGenType> like IIPvXTrafficGenerator {
            parameters:
                @display("p=329,81");
        }
    connections allowunconnected:
        // transport connections
        networkLayer.transportOut++ --> trafGen.ipIn;
        networkLayer.transportIn++ <-- trafGen.ipOut;
}

