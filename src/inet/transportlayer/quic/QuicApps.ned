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

package inet.transportlayer.quic;

import inet.applications.contract.IApp;
import inet.common.MessageDispatcher;

module QuicApps like IApp
{
    parameters:
        int numQuicApps = default(0);
        @figure[applicationLayer](type=rectangle; pos=250,6; size=1000,130; lineColor=#808080; cornerRadius=5; fillColor=#ffff00; fillOpacity=0.1);
        @figure[applicationLayer.title](type=text; pos=1245,11; anchor=ne; text="application layer");

    gates:
        input socketIn;
        output socketOut;

    submodules:
        quicApp[numQuicApps]: <> like IApp {
            parameters:
                @display("p=100,50,row,150");
        }
        aq: MessageDispatcher {
            parameters:
                @display("p=100,125;b=120,5,,,,1");
        }
        quic: Quic {
            parameters:
                @display("p=100,200");
        }

    connections allowunconnected:
        for i=0..numQuicApps-1 {
            quicApp[i].socketOut --> aq.in++;
            quicApp[i].socketIn <-- aq.out++;
        }

        aq.out++ --> quic.appIn;
        aq.in++ <-- quic.appOut;

        quic.udpOut --> socketOut;
        quic.udpIn <-- socketIn;
}
