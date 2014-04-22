//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


package inet.applications.extapp;


import inet.applications.ITCPApp;


//
// TODO
//
module TCPClientTunnel like ITCPApp
{
    parameters:
        @class(TCPClientTunnel);
        string dataTransferMode @enum("bytecount","object","bytestream") = "bytestream";
        string localAddress = default(""); // local address or empty ("")
        int localPort = default(-1); // local port number
        string connectAddress;  // server address (may be symbolic)
        int connectPort; // port number to connect to (port on the server)
        int tunnelPort; // local port number on the real computer for listening for connect a real TCP client
        string threadClass = default("inet.applications.extapp.TCPActiveTunnelThread");
        @display("i=block/telnet");
        @signal[sentPk](type=cPacket);
        @signal[rcvdPk](type=cPacket);
        @signal[connect](type=long);  // 1 for open, -1 for close
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[sentPk](title="packets sent"; source=sentPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="messageAge(rcvdPk)"; unit=s; record=histogram,vector; interpolationmode=none);

    gates:
        input tcpIn @labels(TCPCommand/up);
        output tcpOut @labels(TCPCommand/down);

    submodules:
        ctrl: TCPMultiThreadCtrl {
            @display("p=72,167");
        }
        listener: TCPRealListenerThread {
            parameters:
                tunnelPort = tunnelPort;
                @display("p=72,61");
        }
    connections allowunconnected:
        tcpIn --> ctrl.tcpIn;
        ctrl.tcpOut --> tcpOut;

        ctrl.threadOut++ --> listener.tcpIn;
        listener.tcpOut --> ctrl.threadIn++;
}

