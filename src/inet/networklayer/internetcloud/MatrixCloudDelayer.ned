//
// Copyright (C) 2012 OpenSim Ltd.
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
// @author Zoltan Bojthe
//

package inet.networklayer.internetcloud;

//
// Delay module for ~InternetCloud. This is essentially equivalent to a full graph
// with edges being differently configured DatarateChannels. It delays and/or drops
// incoming packets based on rules specified in an xml configuration.
//
// Rules are matched by incoming and outgoing neighbour node names, and specify
// delay, datarate or drop decision (all optional). Delay may be different for each
// packet, but the order of packet is preserved.
//
// Using the interface IDs the module will look up the names of the connected nodes
// and use those names for matching against the rule database.
//
// A possible xml configuration:
// <pre>
//    <internetCloud symmetric="false">
//      <parameters>
//        <traffic src="src_host*" dest="dest_host*" delay="0.012s+exponential(0.2s)" datarate="uniform(100kbps,1Mbps)" drop="uniform(0,1) &lt; 0.005" symmetric="true"/>
//        <traffic src="hosta" dest="hostb" delay="0.012s+exponential(0.2s)" datarate="uniform(500kbps,1Mbps)" drop="uniform(0,1) &lt; 0.005"/>
//        <traffic src="hostb" dest="hosta" delay="0.010s+exponential(0.2s)" datarate="uniform(100kbps,500kbps)" drop="uniform(0,1) &lt; 0.003"/>
//        <traffic src="*" dest="*" delay="0.02s+exponential(0.5s)" datarate="uniform(100kbps,200kbps)" drop="uniform(0,1) &lt; 0.03" />
//      </parameters>
//    </internetCloud>
// </pre>
//
// - The "delay","datarate" and "drop" attributes of <traffic> are NED expressions that
//   are evaluated for each packet. ("drop" must evaluate to boolean.)
// - The "symmetric" attribute of <traffic> specifies whether the rule applies to
//   both src->dest and dest->src packets.
// - The "symmetric" attribute of <internetCloud> specifies the default value for
//   the <traffic> entries.
//
// @see InternetCloud
//
simple MatrixCloudDelayer like ICloudDelayer
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        xml config;
}
