// $Header$
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

import
  "NetworkLayers",
  "ProcessorManager",
  "UDP",
  "ErrorHandling",
  "SocketApp",
  "UDPModule",
  "socketlayer";

module UDPSocketHost
       parameters:
         client_name : string,
     nodename   : string,
     // network information
     numOfProcessors : numeric,
     numOfPorts  : numeric,
     routingFile     : string;
       gates:
         in: in[];
     out: out[];

       submodules:
        applClient: client_name like SocketApp
      parameters:
        debug = input(false, "Enable debugging output (false/true)?");
          display: "p=87,30;i=proc1";
        socketLayer: SocketLayer;
          parameters:
        debug = input(false, "Enable debugging output (false/true)?");
          display: "p=87,90;i=prot1";
    udpModule: UDPModule;
      parameters:
        debug = input(false, "Enable debugging output (false/true)?"),
        nodename = nodename;
          display: "p=87,150;i=prot2";

     errorHandling: ErrorHandling;
           parameters:
             nodename = routingFile;
           display: "p=180,210;i=sink";
         networkLayers: NetworkLayers;
           parameters:
         // waiting times
         icmp_delay = 0,
         fragmentation_delay = 0.2 us,
         multicast_delay = 0.5 us,
         ipoutput_delay = 0,
         ipsend_delay = 0,
         iptunneling_delay = 0,
         localdeliver_delay = 0,
         prerouting_delay = 0.2 us,
         routing_delay = 1 us,
         enqueue_delay = 0,
         dequeue_delay = 0,
         inputqueue_delay = 0,
         nwi_delay = 0,

         // hook names
         preRoutingHookName = "NF_IP_PRE_ROUTING",
             routingHookName = "NF_IP_FORWARD",
             localDeliverHookName = "NF_IP_LOCAL_IN",
             ipSendHookName = "NF_IP_LOCAL_OUT",
             ipOutputHookName = "NF_IP_POST_ROUTING",
         enqueueHookName = "L2_EnqueueHook",
         dequeueHookName = "L2_DequeueHook",

         // other network parameters
         IPForward = false,
             NWIName = "PPPModule",
             numOfPorts = numOfPorts,
             routingFile = routingFile;
           gatesizes:
             physIn[numOfPorts],
             physOut[numOfPorts];
           display: "p=87,210;i=fork";
     //# processorManager: ProcessorManager;
    connections nocheck:
      // transport connections
    applClient.out --> socketLayer.from_appl[0];
    applClient.in  <-- socketLayer.to_appl[0];
    socketLayer.to_udp --> udpModule.from_socket;
    socketLayer.from_udp <-- udpModule.to_socket;
      networkLayers.UDPOut --> udpModule.from_ip;
      networkLayers.UDPIn <-- udpModule.to_ip;
      networkLayers.errorOut --> errorHandling.in;

      // connections to network outside
          for i=0..numOfPorts-1 do
            in[i] --> networkLayers.physIn[i];
            out[i] <-- networkLayers.physOut[i];
          endfor;
endmodule
