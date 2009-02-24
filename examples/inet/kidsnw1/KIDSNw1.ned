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




package inet.examples.inet.kidsnw1;

import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import ned.DatarateChannel;


network KIDSNw1
{
    types:
        channel ethernet extends DatarateChannel
        {
            delay = normal(0.00015s, 0.00005s);
            datarate = 10Mbps;
        }
        channel fiberrouterline extends DatarateChannel
        {
            delay = normal(0.000001s, 0.0000001s);
            datarate = 512Mbps;
        }
    submodules:
        host[3]: StandardHost {
            parameters:
                IPForward = false;
                routingFile = (index==0 ? "host1.irt" : (index==1 ? "host2.irt" : (index==2 ? "host3.irt" : "PROBLEM")));
                @display("p=101,307,row;i=device/pc2");

            gates:
                pppg[1];
        }
        // First Hop Router
        fhrouter[2]: Router {
            parameters:
                routingFile = (index==0 ? "fhrouter1.irt" : (index==1 ? "fhrouter5.irt" : "PROBLEM"));
                @display("p=119,222,row,130");

            gates:
                pppg[(index==0 ? 3 : 2)];
        }
        // InteriorRouter
        irouter: Router {
            parameters:
                routingFile = "irouter2.irt";
                @display("p=115,144");
            gates:
                pppg[2];
        }
        // Border Gatgeway Router
        bgrouter[2]: Router {
            parameters:

                routingFile = (index==0 ? "bgrouter3.irt" : (index==1 ? "bgrouter4.irt" : "PROBLEM"));
                @display("p=135,64,row");
            gates:
                pppg[2];
        }
    connections:
        // host connections
        host[0].pppg[0] <--> ethernet <--> fhrouter[0].pppg[0];
        host[1].pppg[0] <--> ethernet <--> fhrouter[0].pppg[1];
        host[2].pppg[0] <--> ethernet <--> fhrouter[1].pppg[0];

        // router connections
        fhrouter[0].pppg[2] <--> fiberrouterline <--> irouter.pppg[0];
        irouter.pppg[1] <--> fiberrouterline <--> bgrouter[0].pppg[0];
        bgrouter[0].pppg[1] <--> fiberrouterline <--> bgrouter[1].pppg[1];
        bgrouter[1].pppg[0] <--> fiberrouterline <--> fhrouter[1].pppg[1];
}

