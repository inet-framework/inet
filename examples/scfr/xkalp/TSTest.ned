//
// Copyright (C) 2012 Kyeong Soo (Joseph) Kim
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


package inet.examples.scfr.xkalp;

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.applications.udpapp.UDPVideoStreamSvrWithTrace2;
import inet.applications.udpapp.UDPVideoStreamCliWithSCFR;
import inet.applications.udpapp.UDPAppConnector;

//
// Defines a network for testing timestamp generation at UDP video server and client
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2012-12-31
//
network TSTest
{
    parameters:
        @display("bgb=348,132");
    submodules:
        //        configurator: FlatNetworkConfigurator {
        //            parameters:
        //                @display("p=52,42");
        //        }
        server: UDPVideoStreamSvrWithTrace2 {
            parameters:
                @display("i=block/source;p=300,50");
        }
        client: UDPVideoStreamCliWithSCFR {
            parameters:
                @display("i=block/sink;p=50,50");
        }
        filter: UDPAppConnector {
            @display("p=170,90");
        }
    connections:
        server.udpOut --> filter.in;
        filter.out --> client.udpIn;
        server.udpIn <-- client.udpOut;
}
