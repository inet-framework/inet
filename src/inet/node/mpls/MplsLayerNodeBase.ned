//
// Copyright (C) 2015 OpenSim Ltd.
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

package inet.node.mpls;

import inet.common.MessageDispatcher;
import inet.networklayer.mpls.MPLS;
import inet.node.inet.LinkLayerNodeBase;


//
// This module contains the network layer infrastructure for network nodes.
//
// See derived modules: ~StandardHost, ~WirelessHost, ~AdhocHost, ~Router, etc.
//
module MplsLayerNodeBase extends LinkLayerNodeBase
{
    parameters:
        @figure[networkLayer](type=rectangle; pos=250,400; size=600,200; fillColor=#00ff00; cornerRadius=12; fillOpacity=0.1);
        @figure[networkLayer.title](type=text; pos=740,410; text="network layer");

    submodules:
        mpls: MPLS {
            parameters:
                classifierModule = "^.ldp";
                @display("p=400,500;q=queue");
        }
        ml: MessageDispatcher {
            parameters:
                @display("b=580,5,,,,1;p=550,600");
        }

    connections allowunconnected:
        mpls.ifIn <-- ml.upperLayerOut++;
        mpls.ifOut --> ml.upperLayerIn++;

        for i=0..numLoInterfaces-1 {
            ml.lowerLayerOut++ --> lo[i].upperLayerIn;
            lo[i].upperLayerOut --> ml.lowerLayerIn++;
        }

        for i=0..sizeof(radioIn)-1 {
            wlan[i].upperLayerOut --> ml.lowerLayerIn++;
            wlan[i].upperLayerIn <-- ml.lowerLayerOut++;
        }

        for i=0..sizeof(ethg)-1 {
            eth[i].upperLayerOut --> ml.lowerLayerIn++;
            eth[i].upperLayerIn <-- ml.lowerLayerOut++;
        }

        for i=0..sizeof(pppg)-1 {
            ppp[i].upperLayerOut --> ml.lowerLayerIn++;
            ppp[i].upperLayerIn <-- ml.lowerLayerOut++;
        }

        for i=0..numExtInterfaces-1 {
            ext[i].upperLayerOut --> ml.lowerLayerIn++;
            ext[i].upperLayerIn <-- ml.lowerLayerOut++;
        }

        for i=0..numTunInterfaces-1 {
            tun[i].upperLayerOut --> ml.lowerLayerIn++;
            tun[i].upperLayerIn <-- ml.lowerLayerOut++;
        }
}

