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


module MyTCPUpperLayers
{
    parameters:
        string mytcpApplType;
        string local_addr;
        volatile double start_time;
        string server_addr;
        @display("p=10,10;b=150,236,rect");
    gates:
        input in;
        output out;
    submodules:
        application: <mytcpApplType> like MyTCPApp {
            parameters:
                local_addr = local_addr;
                server_addr = server_addr;
                timeout = 30000s;
                data_interarrival_time = 10s;
                start_time = start_time;
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

