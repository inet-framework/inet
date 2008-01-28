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
// TCP Upper layers
//
// - upper layers of TCP interfacing with the IP architecture
// - TCP application <-> TCP <-> TCP2IPInterface <-> IP
//
// author V.Boehm, June 19 1999
// author J.Reber, Sept. 2000
//
module TCPUpperLayers
{
    parameters:
        string tcpApplType;
        volatile double local_addr;
        volatile double server_addr;
        @display("p=10,10;b=150,236,rect");
    gates:
        input in;
        output out;
    submodules:
        application: <tcpApplType> like TCPApp {
            parameters:


                //              default timeout according to RFC 793 5min (here 5s)
                //              timeout =  input(50s, "Timeout: "),
                //              appl_timeout = input(80s, "Application timeout: "),
                //              conn_ia_time = input(1s, "Connection interarrival time: "),
                //              msg_ia_time = input(0.5s, "Data message interarrival time: "),
                //              processing_time = input(0.2s, "Server processing time: "),
                debug = false;
                local_addr = local_addr;
                server_addr = server_addr;
                timeout = 50s;
                appl_timeout = 80s;
                conn_ia_time = 1s;
                msg_ia_time = 0.5s;
                processing_time = 0.2s;

                //rec_pks = input(1, "Rec. no of data packets p. issued RECEIVE: "),
                //message_length = intuniform(1, 64000);
                //message_length = input(8000, Number of bits to be received: ");
                message_length = 8000;
                @display("p=76,52;b=40,24,rect");
        };
        tcp: TcpModule {
            parameters:
                TCPDelayedAck = true;
                TCPFastRetrans = false;
                TCPFastRecovery = false;
                TCPNewReno = false;
                @display("p=79,120");
        };
        tcpipinterface: TcpIpInterface {
            parameters:
                @display("p=81,202");
        };
    connections:
        application.out --> tcp.from_appl;
        application.in <-- tcp.to_appl;
        tcp.to_ip --> tcpipinterface.from_tcp;
        tcp.from_ip <-- tcpipinterface.to_tcp;
        tcpipinterface.to_ip --> out;
        tcpipinterface.from_ip <-- in;
}

