//
// Copyright (C) 2000 Institut fuer Nachrichtentechnik, Universitaet Karlsruhe
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
    "NetworkLayers", 
    "UDPProcessing", 
    "SocketApp", 
    "socketlayer"; 
                   

// FIXME socketLayer should get a local_port parameter, UDPProcessing needs that (?!?!)
module UDPSocketHost 
    parameters: 
        client_name : string, 
        numOfPorts : numeric, 
        routingFile : string; 
    gates: 
        in: in[]; 
        out: out[]; 
                    
    submodules: 
        applClient: client_name like SocketApp; 
            display: "p=87,30;i=proc1";
        socketLayer: SocketLayer; 
            display: "p=87,90;i=prot1";
        udp: UDPProcessing; 
            display: "p=87,150;i=prot2";
        networkLayers: NetworkLayers; 
            parameters: 
                IPForward = false, 
                NWIName = "PPPModule", 
                numOfPorts = numOfPorts, 
                routingFile = routingFile; 
            gatesizes: 
                physIn[numOfPorts], 
                physOut[numOfPorts]; 
            display: "p=87,210;i=fork";
    connections nocheck: 
        // transport connections
        applClient.out --> socketLayer.from_appl[0]; 
        applClient.in <-- socketLayer.to_appl[0]; 
        socketLayer.to_udp --> udp.from_application; 
        socketLayer.from_udp <-- udp.to_application; 
        networkLayers.UDPOut --> udp.from_ip; 
        networkLayers.UDPIn <-- udp.to_ip; 
                                                 
        // connections to network outside
        for i=0..numOfPorts-1 do 
            in[i] --> networkLayers.physIn[i]; 
            out[i] <-- networkLayers.physOut[i]; 
        endfor; 
endmodule 

