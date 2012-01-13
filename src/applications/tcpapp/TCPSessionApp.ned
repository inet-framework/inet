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


package inet.applications.tcpapp;

//
// Single-connection TCP application: it opens a connection, sends
// the given number of bytes, and closes. Sending may be one-off,
// or may be controlled by a "script" which is a series of
// (time, number of bytes) pairs. May act either as client or as server,
// and works with TCPVirtualBytesSendQueue/RcvQueue as sendQueue/receiveQueue
// setting for TCP.
// Compatible with both IPv4 and IPv6.
//
// <b>Opening the connection</b>
//
// Regarding the type of opening the connection, the application may
// be either a client or a server. When active=false, the application
// will listen on the given local port, and wait for an incoming connection.
// When active=true, the application will bind to given local address:port,
// and connect to the connectAddress:connectPort. To use an ephemeral port
// as local port, set the port parameter to -1.
//
// Even when in server mode (active=false), the application will only
// serve one incoming connection. Further connect attempts will be
// refused by TCP (it will send RST) for lack of LISTENing connections.
//
// The time of opening the connection is in the tOpen parameter.
//
// <b>Sending data</b>
//
// Regardless of the type of OPEN, the application can be made to send
// data. One way of specifying sending is via the tSend, sendBytes
// parameters, the other way is sendScript. With the former, sendBytes
// bytes will be sent at tSend. With sendScript, the format is
// "<time> <numBytes>;<time> <numBytes>;..."
//
// <b>Closing the connection</b>
//
// The application will issue a \TCP CLOSE at time tClose. If tClose=-1, no
// CLOSE will be issued.
//
simple TCPSessionApp like TCPApp
{
    parameters:
        string address = default("");
        int port;   // FIXME change to listenPort = default(1000)
        bool active = default(true);
        string connectAddress;
        int connectPort = default(1000);
        double tOpen @unit("s") = default(1s);
        double tSend @unit("s") = default(1s);
        int sendBytes @unit("B") = default(1MiB);
        string sendScript = default("");
        double tClose @unit("s") = default(2s);
        @display("i=block/app");
    gates:
        input tcpIn @labels(TCPCommand/up);
        output tcpOut @labels(TCPCommand/down);
}

