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



//
// Part of TCPTester.
//
module Client
{
    parameters:
        string client_name;
        volatile double l_addr;
        volatile double s_addr;
    gates:
        input from_switch;
        output to_switch;
    submodules:
        applclient: <client_name> like TCPApp {
            parameters:
                debug = default(false) @prompt("Enable debugging output (false/true)?");
                local_addr = l_addr;
                server_addr = s_addr;
                //default timeout according to RFC 793 5min (here 5s)
                timeout = default(5m) @prompt("Timeout: ");
                appl_timeout = default(10m) @prompt("Application timeout: ");
                conn_ia_time = default(1s) @prompt("Connection interarrival time: ");
                msg_ia_time = default(0.5s) @prompt("Data message interarrival time: ");
                processing_time = 0;
                message_length = default(8000) @prompt("Number of bits to be received: ");
        };
        tcp: TcpModule {
            parameters:
                TCPDelayedAck = default(true) @prompt("Delayed Acknowledge: ");
                TCPFastRetrans = default(true) @prompt("Fast Retransmission (TCP-Tahoe): ");
                TCPFastRecovery = default(false) @prompt("Fast Recovery: ");
                TCPNewReno = default(false) @prompt("New Reno: ");
        };
        ip: DummyIP {};
        ppp: DummyPPP {
            parameters:
                nw_length = default(18) @prompt("Number of bytes added by the network layer: ");
        };
    connections:
        applclient.out --> tcp.from_appl;
        applclient.in <-- tcp.to_appl;
        tcp.to_ip --> ip.from_tcp;
        tcp.from_ip <-- ip.to_tcp;
        ip.to_ppp --> ppp.from_ip;
        ip.from_ppp <-- ppp.to_ip;
        ppp.to_compound --> to_switch;
        ppp.from_compound <-- from_switch;
}


//
// Part of TCPTester.
//
module Server
{
    parameters:
        string server_name;
        volatile double l_addr;
        volatile double s_addr;
    gates:
        input from_switch;
        output to_switch;
    submodules:
        applserver: <server_name> like TCPApp {
            parameters:
                debug = default(false) @prompt("Enable debugging output (false/true)?");
                local_addr = l_addr;
                server_addr = s_addr;
                processing_time = default(0.2s) @prompt("Server processing time: ");
                //default  timeout according to RFC-793 5min (here 5s)
                timeout = default(5m) @prompt("Timeout: ");
                appl_timeout = default(10m) @prompt("Application timeout: ");
                conn_ia_time = 0;
                msg_ia_time = 0;
                message_length = 0;
        };
        tcp: TcpModule {
            parameters:
                TCPDelayedAck = default(true) @prompt("Delayed Acknowledge: ");
                TCPFastRetrans = default(true) @prompt("Fast Retransmission (TCP-Tahoe): ");
                TCPFastRecovery = default(false) @prompt("Fast Recovery: ");
                TCPNewReno = default(false) @prompt("New Reno: ");
        };
        ip: DummyIP {};
        ppp: DummyPPP {
            parameters:
                nw_length = default(18) @prompt("Number of bytes added by the network layer: ");
        };
    connections:
        applserver.out --> tcp.from_appl;
        applserver.in <-- tcp.to_appl;
        tcp.to_ip --> ip.from_tcp;
        tcp.from_ip <-- ip.to_tcp;
        ip.to_ppp --> ppp.from_ip;
        ip.from_ppp <-- ppp.to_ip;
        ppp.to_compound --> to_switch;
        ppp.from_compound <-- from_switch;
}


//
// Part of TCPTester.
//
channel connectSwitchStation extends DatarateChannel
{
    parameters:
        delay = normal (0.000001, 0.00000025); //mean, deviation
        datarate = 10000000; // 10 Mbit/s
}


//
// Simple test environment for TcpModule: n clients, and a server, connected
// via a switch. The switch models an unreliable network, and is
// able to lose packets randomly.
//
// author V.Boehm, June 19 1999
//
network TCPTester
{
    parameters:
        int num_client;
    submodules:
        client[num_client]: Client {
            parameters:
                client_name = "TCPClient";
                l_addr = index;
                s_addr = num_client;
        };
        server: Server {
            parameters:
                server_name = "TCPServer";
                l_addr = num_client;
                s_addr = num_client;
        };
        dropswitch: DropSwitch {
        //parameters:
         //  server_port = num_client,
         //  burst_delete_probability = input(0, " Enter prob that a burst will be deleted (in precentage!) "),
         //  error_burst_len = input(0, " Enter length of a burst error ");
            gates:
                in[num_client + 1];
                out[num_client + 1];
        };
    connections:
        for i=0..num_client-1 {
            client[i].to_switch --> connectSwitchStation --> dropswitch.in[i];
            client[i].from_switch <-- connectSwitchStation <-- dropswitch.out[i];
        };
        server.to_switch --> connectSwitchStation --> dropswitch.in[num_client];
        server.from_switch <-- connectSwitchStation <-- dropswitch.out[num_client];
}


//
// Simple test environment for TcpModule: n clients, and a server, connected
// via a switch. Here, the switch is reliable so this model can be used
// to test the case when everything it going right.
//
// author V.Boehm, June 19 1999
//
network TCPTesterB
{
    parameters:
        int num_client;
    submodules:
        client[num_client]: Client {
            parameters:
                client_name = "TCPClient";
                l_addr = index;
                s_addr = num_client;
        };
        server: Server {
            parameters:
                server_name = "TCPServer";
                l_addr = num_client;
                s_addr = num_client;
        };
        switch: Switch {
            gates:
                in[num_client + 1];
                out[num_client + 1];
        };
    connections:
        for i=0..num_client-1 {
            client[i].to_switch --> connectSwitchStation --> switch.in[i];
            client[i].from_switch <-- connectSwitchStation <-- switch.out[i];
        };
        server.to_switch --> connectSwitchStation --> switch.in[num_client];
        server.from_switch <-- connectSwitchStation <-- switch.out[num_client];
}


