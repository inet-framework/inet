//
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
    "tcpapp",
    "procserver",
    "tcpmodule",
    "dropswitch",
    "switch",
    "ipmodule",
    "ppp";


//
// Part of TCPTester.
//
module Client
    parameters:
      client_name : string,
      l_addr : numeric,
      s_addr : numeric;
    gates:
      in: from_switch;
      out: to_switch;
    submodules:
      applclient: client_name like TCPApp
       parameters:
        debug = input(false, "Enable debugging output (false/true)?"),
        local_addr = l_addr,
        server_addr = s_addr,
        //default timeout according to RFC 793 5min (here 5s)
        timeout =  input(5m, "Timeout: "),
        appl_timeout = input(10m, "Application timeout: "),
        conn_ia_time = input(1s, "Connection interarrival time: "),
        msg_ia_time = input(0.5s, "Data message interarrival time: "),
        processing_time = 0,
        message_length = input(8000, "Number of bits to be received: ");
      tcp: TcpModule
       parameters:
        TCPDelayedAck = input(true, "Delayed Acknowledge: "),
        TCPFastRetrans = input(true, "Fast Retransmission (TCP-Tahoe): "),
        TCPFastRecovery = input(false, "Fast Recovery: "),
        TCPNewReno = input(false, "New Reno: ");
      ip: DummyIP;
      ppp: DummyPPP;
       parameters:
        nw_length = input(18, "Number of bytes added by the network layer: ");
    connections:
       applclient.out --> tcp.from_appl,
       applclient.in <-- tcp.to_appl,
       tcp.to_ip --> ip.from_tcp,
       tcp.from_ip  <-- ip.to_tcp,
       ip.to_ppp --> ppp.from_ip,
       ip.from_ppp <-- ppp.to_ip,
       ppp.to_compound --> to_switch,
       ppp.from_compound <-- from_switch;
endmodule


//
// Part of TCPTester.
//
module Server
   parameters:
      server_name : string,
      l_addr : numeric,
      s_addr: numeric;
        gates:
      in: from_switch;
      out: to_switch;
    submodules:
       applserver: server_name like TCPApp
          parameters:
         debug = input(false, "Enable debugging output (false/true)?"),
         local_addr  = l_addr,
         server_addr = s_addr,
         processing_time = input(0.2s, "Server processing time: "),
         //default  timeout according to RFC-793 5min (here 5s)
         timeout = input(5m, "Timeout: "),
         appl_timeout = input(10m, "Application timeout: "),
         conn_ia_time = 0,
         msg_ia_time = 0,
         message_length = 0;
       tcp: TcpModule
         parameters:
            TCPDelayedAck = input(true, "Delayed Acknowledge: "),
            TCPFastRetrans = input(true, "Fast Retransmission (TCP-Tahoe): "),
            TCPFastRecovery = input(false, "Fast Recovery: "),
            TCPNewReno = input(false, "New Reno: ");
       ip: DummyIP;
       ppp: DummyPPP;
         parameters:
            nw_length = input(18, "Number of bytes added by the network layer: ");
    connections:
       applserver.out --> tcp.from_appl,
       applserver.in <-- tcp.to_appl,
       tcp.to_ip --> ip.from_tcp,
       tcp.from_ip <-- ip.to_tcp,
       ip.to_ppp --> ppp.from_ip,
       ip.from_ppp <-- ppp.to_ip,
       ppp.to_compound --> to_switch,
       ppp.from_compound <-- from_switch;
endmodule


//
// Part of TCPTester.
//
channel connectSwitchStation
    delay normal (0.000001, 0.00000025) //mean, deviation
    datarate 10000000 // 10 Mbit/s
endchannel


//
// Simple test environment for TcpModule: n clients, and a server, connected
// via a switch. The switch models an unreliable network, and is
// able to lose packets randomly.
//
// author V.Boehm, June 19 1999
//
module TCPTester
    parameters:
      const num_client;
    submodules:
      client: Client[num_client]
        parameters:
          client_name = "TCPClient",
          l_addr = index,
          s_addr = num_client;
      server: Server
        parameters:
          server_name = "TCPServer",
          l_addr = num_client,
          s_addr = num_client;
      dropswitch: DropSwitch;
        //parameters:
         //  server_port = num_client,
         //  burst_delete_probability = input(0, " Enter prob that a burst will be deleted (in precentage!) "),
         //  error_burst_len = input(0, " Enter length of a burst error ");
        gatesizes:
           in[num_client + 1],
           out[num_client + 1];
    connections:
        for i = 0 .. num_client-1 do
           client[i].to_switch --> connectSwitchStation --> dropswitch.in[i],
           client[i].from_switch <-- connectSwitchStation <-- dropswitch.out[i];
        endfor
      server.to_switch --> connectSwitchStation --> dropswitch.in[num_client],
      server.from_switch <-- connectSwitchStation <-- dropswitch.out[num_client];
endmodule


//
// Simple test environment for TcpModule: n clients, and a server, connected
// via a switch. Here, the switch is reliable so this model can be used
// to test the case when everything it going right.
//
// author V.Boehm, June 19 1999
//
module TCPTesterB
    parameters:
      const num_client;
    submodules:
      client: Client[num_client]
        parameters:
          client_name = "TCPClient",
          l_addr = index,
          s_addr = num_client;
      server: Server
        parameters:
          server_name = "TCPServer",
          l_addr = num_client,
          s_addr = num_client;
      switch: Switch;
        gatesizes:
           in[num_client + 1],
           out[num_client + 1];
    connections:
        for i = 0 .. num_client-1 do
           client[i].to_switch --> connectSwitchStation --> switch.in[i],
           client[i].from_switch <-- connectSwitchStation <-- switch.out[i];
        endfor
      server.to_switch --> connectSwitchStation --> switch.in[num_client],
      server.from_switch <-- connectSwitchStation <-- switch.out[num_client];
endmodule


network tcptester: TCPTester
    parameters:
        num_client = input(2, "Number of clients: ");
endnetwork

network tcptesterb: TCPTesterB
      parameters:
             num_client = input(2, "Number of clients: ");
endnetwork


