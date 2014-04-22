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
module TCPServerTunnel like ITCPApp
{
    parameters:
        string dataTransferMode @enum("bytecount","object","bytestream") = "bytestream";
        string localAddress = default(""); // local address or empty ("")
        int localPort; // port number to listen on
        string tunnelAddress;  // real computer IP address
        int tunnelPort; // port number on real computer
        string serverThreadClass = default("inet.applications.extapp.TCPActiveTunnelThread");
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
        listener: TCPListenerThread
        {
            localAddress = localAddress;
            localPort = localPort;
            dataTransferMode = dataTransferMode;
        }
    connections allowunconnected:
}

