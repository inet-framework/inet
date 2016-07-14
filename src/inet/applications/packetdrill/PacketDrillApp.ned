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

package inet.applications.packetdrill;

import inet.applications.contract.IApp;


simple PacketDrillApp like IApp
{
    parameters:
        string scriptFile;
        double startTime @unit(s) = default(0s);
        int localPort; // port number of the initiator
        int remotePort;
        string localAddress;
        string remoteAddress;
        int inboundStreams = default(17);
        int outboundStreams = default(1);
        string dataTransferMode @enum("bytecount","object","bytestream") = default("bytecount");
        double latency @unit(s) = default(0s);
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
}

