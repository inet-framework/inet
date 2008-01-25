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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//





//
// Definition of an IPNode with transport layers.
//
module UDPHost
{
    parameters:
        // Network information
        bool IPForward;
        string routingFile;
    gates:
        input in[];
        output out[];
    submodules:
        blackboard: Blackboard {
            parameters:
                @display("p=60,60");
        };
        udpApp: UDPUpperLayers {
            parameters:
                udpClient1Name = "UDPClientApp";
                udpServer1Name = "UDPServerApp";
                @display("p=89,68;b=40,24,rect");
        };
        networkLayer: NetworkLayer {
            parameters:
                // other Network parameters
                IPForward = IPForward;
                routingFile = routingFile;
                @display("p=87,155");
            gates:
                physIn[sizeof(out)];
                physOut[sizeof(out)];
        };
        ppp[sizeof(out)]: PPPInterface {
            parameters:
                @display("p=80,220,row;q=queue");
        };
    connections allowunconnected:
        // transport connections
        networkLayer.UDPOut --> udpApp.from_ip;
        networkLayer.UDPIn <-- udpApp.to_ip;

        // connections to network outside
        for i=0..sizeof(out)-1 {
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;
            ppp[i].netwOut --> networkLayer.physIn[i];
            ppp[i].netwIn <-- networkLayer.physOut[i];
        };
}

