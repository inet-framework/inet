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

import inet.networklayer.internetcloud.ICloudDelayer;

//
// Delay module for InternetCloudNetworkLayer
//
// Calculates delay for incoming packets based on src and dest interfaces
// and send packet with calculated delay. The src info was set by TaggerChannel
// between INICs and NetworkLayer.
// Should use TaggerChannel for connect interfaces for network layer.
//
// @see InternetCloudNetwokLayer, InternetCloud, InternetCloudNetworkConfigurator
//
// A possible xml configuration:
//    <internetCloud symmetric="false">
//      <parameters name="good">
//        <traffic src="src_host*" dest="dest_host*" delay="0.012s+exponential(0.2s)" datarate="uniform(100kbps,1mbps)" drop="0.005" symmetric="true"/>
//        <traffic src="hosta" dest="hostb" delay="0.012s+exponential(0.2s)" datarate="uniform(500kbps,1mbps)" drop="uniform(0,1) &lt; 0.005"/>
//        <traffic src="hostb" dest="hosta" delay="0.010s+exponential(0.2s)" datarate="uniform(100kbps,500kbps)" drop="uniform(0,1) &lt; 0.003"/>
//        <traffic src="*" dest="*" delay="0.02s+exponential(0.5s)" datarate="uniform(100kbps,200kbps)" drop="uniform(0,1) &lt; uniform(0.01,0.03)" />
//      </parameters>
//    </internetCloud>
//
//TODO: I think: need a datarate attribute for traffic entries, too
//      calculated_delay := delay + (8 * packet_length_in_bytes) / datarate_in_bps
//
simple MatrixCloudDelayer like ICloudDelayer
{
    parameters:
        xml config;
    gates:
        input ipIn;
        output ipOut;
}
