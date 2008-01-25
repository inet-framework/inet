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





package org.omnetpp.inet.Nodes.INET;

import org.omnetpp.inet.Applications.Generic.IPTrafficGenerator;
import org.omnetpp.inet.Base.NotificationBoard;
import org.omnetpp.inet.Network.Contract.InterfaceTable;
import org.omnetpp.inet.Network.IPv4.RoutingTable;
import org.omnetpp.inet.NetworkInterfaces.PPP.PPPInterface;


//
// Definition of an IP node with a transport generator that connects
// to IP directly, without TCP or UDP.
//
module BurstHost
{
    parameters:
        string trafGenType;
        bool IPForward = default(false);
        string routingFile = default("");
        @display("i=device/pc3");
    gates:
        inout pppg[];
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=83,66");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=83,136");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = IPForward;
                routerId = "";
                routingFile = routingFile;
                @display("p=83,206");
        }
        trafGen: <trafGenType> like IPTrafficGenerator {
            parameters:
                @display("p=198,86");
        }
        networkLayer: NetworkLayer {
            parameters:
                @display("p=198,173;q=queue");
            gates:
                ifIn[1];
                ifOut[1];
        }
        ppp[1]: PPPInterface {
            parameters:
                @display("p=198,262,row,90;q=txQueue");
        }
    connections allowunconnected:
        // transport connections
        networkLayer.UDPOut --> trafGen.from_ip;
        networkLayer.UDPIn <-- trafGen.to_ip;

        // connections to network outside
        for i=0..0 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].netwOut --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- networkLayer.ifOut[i];
        }

}

