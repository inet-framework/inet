//
// Copyright (C) OpenSim Ltd.
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

package inet.node.base;

import inet.applications.contract.IApp;
import inet.common.MessageDispatcher;

module ApplicationLayerNodeBase extends TransportLayerNodeBase
{
    parameters:
        int numApps = default(0);
        @figure[applicationLayer](type=rectangle; pos=250,0; size=600,200; fillColor=#ffff00; cornerRadius=12; fillOpacity=0.1);
        @figure[applicationLayer.title](type=text; pos=740,10; text="application layer");

    submodules:
        app[numApps]: <> like IApp {
            parameters:
                @display("p=550,100,row,100");
        }
        at: MessageDispatcher {
            parameters:
                @display("b=580,5,,,,1;p=550,200");
        }

    connections allowunconnected:
        for i=0..numApps-1 {
            app[i].socketOut --> at.upperLayerIn++;
            app[i].socketIn <-- at.upperLayerOut++;
        }

        at.lowerLayerOut++ --> udp.appIn if hasUdp;
        at.lowerLayerIn++ <-- udp.appOut if hasUdp;

        at.lowerLayerOut++ --> tcp.appIn if hasTcp;
        at.lowerLayerIn++ <-- tcp.appOut if hasTcp;

        at.lowerLayerOut++ --> sctp.from_appl++ if hasSctp;
        at.lowerLayerIn++ <-- sctp.to_appl++ if hasSctp;

        at.lowerLayerOut++ --> tn.upperLayerIn++;
        at.lowerLayerIn++ <-- tn.upperLayerOut++;
}
