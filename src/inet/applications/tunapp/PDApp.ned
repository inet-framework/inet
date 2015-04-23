//
// Copyright (C) 2015 Irene Ruengeler
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

package inet.applications.tunapp;

//import inet.applications.contract.IPacketdrillApp;

simple PDApp //like IPacketdrillApp
{
    parameters:
        string scriptFile;
        double startTime @unit(s) = default(1s);
        int localPort = default(1000); // port number of the initiator
        int remotePort = default(2000);
        string localAddress = default("192.168.0.1");
        string remoteAddress = default("192.168.0.2");
        string dataTransferMode @enum("bytecount","object","bytestream") = default("bytecount");
        double latency @unit(s) = default(0.1s);
        @display("i=block/app");
        @class(PDApp);
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
        input tunIn @labels(TunInterface/up);
        output tunOut @labels(TunInterface/down);
}
