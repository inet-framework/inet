//
// Copyright (C) 2004 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.test.moduletest.lib;

import inet.applications.contract.IApp;

//
// \TCP client application for testing the TCP model.
//
simple TcpTestClient like IApp
{
    parameters:
        string localAddress;
        int localPort;
        bool active;
        string connectAddress;
        int connectPort;
        string dataTransferMode @enum("bytecount","object","bytestream") = default("bytecount");
        double tOpen @unit(s);
        double tSend @unit(s);
        int sendBytes @unit(B);
        string sendScript;
        double tClose @unit(s);
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
}

