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




import  
    "Router", 
    "BurstHost"; 
                 
                 
 
module PerformanceNetwork 
    parameters: 
        packetNo : numeric const, 
        nodeNo : numeric const; 
    submodules: 
                
        // * Send Hosts *
        sender: BurstHost[nodeNo]; 
            parameters: 
                nodenr = index, 
                burstPackets = packetNo, 
                packetSize = 1000; 
                                   
            parameters if index==0: 
                nodename = "sender0", 
                routingFile = "sender0.irt"; 
            parameters if index==1: 
                nodename = "sender1", 
                routingFile = "sender1.irt"; 
            parameters if index==2: 
                nodename = "sender2", 
                routingFile = "sender2.irt"; 
            parameters if index==3: 
                nodename = "sender3", 
                routingFile = "sender3.irt"; 
            parameters if index==4: 
                nodename = "sender4", 
                routingFile = "sender4.irt"; 
            parameters if index==5: 
                nodename = "sender5", 
                routingFile = "sender5.irt"; 
            parameters if index==6: 
                nodename = "sender6", 
                routingFile = "sender6.irt"; 
            parameters if index==7: 
                nodename = "sender7", 
                routingFile = "sender7.irt"; 
            parameters if index==8: 
                nodename = "sender8", 
                routingFile = "sender8.irt"; 
            parameters if index==9: 
                nodename = "sender9", 
                routingFile = "sender9.irt"; 
                                               
            gatesizes: 
                in[1], 
                out[1]; 
                        
            display: "p=61,205,row;b=40,24,rect";
        // * Receive Hosts *
        recip: BurstHost[nodeNo]; 
            parameters: 
                nodenr = index+100, 
                burstPackets = 0, 
                packetSize = 10; 
                                
            parameters if index==0: 
                nodename = "recip0", 
                routingFile = "recip0.irt"; 
            parameters if index==1: 
                nodename = "recip1", 
                routingFile = "recip1.irt"; 
            parameters if index==2: 
                nodename = "recip2", 
                routingFile = "recip2.irt"; 
            parameters if index==3: 
                nodename = "recip3", 
                routingFile = "recip3.irt"; 
            parameters if index==4: 
                nodename = "recip4", 
                routingFile = "recip4.irt"; 
            parameters if index==5: 
                nodename = "recip5", 
                routingFile = "recip5.irt"; 
            parameters if index==6: 
                nodename = "recip6", 
                routingFile = "recip6.irt"; 
            parameters if index==7: 
                nodename = "recip7", 
                routingFile = "recip7.irt"; 
            parameters if index==8: 
                nodename = "recip8", 
                routingFile = "recip8.irt"; 
            parameters if index==9: 
                nodename = "recip9", 
                routingFile = "recip9.irt"; 
                                                  
            gatesizes: 
                in[1], 
                out[1]; 
            display: "p=63,68,row;b=40,24,rect";
        router: Router; 
            parameters: 
                nodename = "Router", 
                // network parameters
                numOfPorts = nodeNo*2, 
                routingFile = "router.irt", 
                numOfProcessors = 1; 
            gatesizes: 
                in[nodeNo*2], 
                out[nodeNo*2]; 
            display: "p=146,134;b=40,24,rect";
    connections nocheck: 

        for i=0..nodeNo-1 do 
            sender[i].out[0] --> router.in[2*i]; 
            sender[i].in[0] <-- router.out[2*i]; 
            recip[i].out[0] --> router.in[2*i+1]; 
            recip[i].in[0] <-- router.out[2*i+1]; 
        endfor; 

    display: "p=10,10;b=494,319,rect";
endmodule 

network performanceNetwork : PerformanceNetwork 
endnetwork 

