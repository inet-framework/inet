//
// Copyright (C) 2004 Andras Varga
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
    "Blackboard",
    "TCPApp.ned",
    "TCPMain.ned",
    "UDPProcessing",
    "UDPApp",
    "NetworkLayer",
    "PingApp";



//
// IP host with TCP, UDP layers and applications.
//
module StandardHost2
    parameters:
        numTcpApps : numeric const,
        numUdpApps : numeric const,
        tcpAppType: string,
        udpAppType: string,
        IPForward : bool,
        routingFile : string;
    gates:
        in: in[];
        out: out[];
    submodules:
        blackboard: Blackboard;
            display: "i=proc2;p=56,78";
        tcpApp: tcpAppType[numTcpApps] like TCPApp;
            display: "p=125,58;i=app";
        tcp: TCPMain;
            display: "p=125,138;i=prot2";
        udpApp: udpAppType[numUdpApps] like UDPApp;
            display: "i=app;p=234,58";
        udp: UDPProcessing;
            display: "p=234,138;i=prot1";
        pingApp: PingApp;
            display: "i=app;p=291,193";
        networkLayer: NetworkLayer;
            parameters:
                IPForward = IPForward,
                routingFile = routingFile;
            gatesizes:
                physIn[sizeof(out)],
                physOut[sizeof(out)];
            display: "p=212,221;i=fork";
        ppp: PPPInterface[sizeof(out)];
            display: "p=170,303,row,90;q=queue;i=iface";
    connections nocheck:
        for i=0..numTcpApps-1 do
            tcpApp[i].tcpOut --> tcp.from_appl++;
            tcpApp[i].tcpIn <-- tcp.to_appl++;
        endfor

        tcp.to_ip --> networkLayer.TCPIn;
        tcp.from_ip <-- networkLayer.TCPOut;

        for i=0..numUdpApps-1 do
            udpApp[i].to_udp --> udp.from_application++;
            udpApp[i].from_udp <-- udp.to_application++;
        endfor

        udp.to_ip --> networkLayer.UDPIn;
        udp.from_ip <-- networkLayer.UDPOut;

        networkLayer.pingOut --> pingApp.in;
        networkLayer.pingIn <-- pingApp.out;

        // connections to network outside
        for i=0..sizeof(out)-1 do
            in[i] --> ppp[i].physIn;
            out[i] <-- ppp[i].physOut;
            ppp[i].netwOut --> networkLayer.physIn[i];
            ppp[i].netwIn <-- networkLayer.physOut[i];
        endfor;
endmodule

