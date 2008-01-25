//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//




//
// FIXME: add documentation here
//
module TCP2LDP
{
    parameters:
        string local_addr;
        volatile double timeout;
        volatile double keepAliveTime;
        @display("p=10,10;b=150,236,rect");
    gates:
        input from_tcp;
        input from_ldp;
        output to_tcp;
        output to_ldp;
    submodules:
        application: LDPInterface {
            parameters:
                local_addr = local_addr;
                timeout = timeout;
                keepAliveTime = keepAliveTime;

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
        application.to_tcp --> tcp.from_appl;
        application.from_tcp <-- tcp.to_appl;
        application.to_ldp --> to_ldp;
        application.from_ldp <-- from_ldp;
        tcp.to_ip --> tcpipinterface.from_tcp;
        tcp.from_ip <-- tcpipinterface.to_tcp;
        tcpipinterface.to_ip --> to_tcp;
        tcpipinterface.from_ip <-- from_tcp;
}

