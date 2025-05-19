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
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.applications.quicapp;

import inet.common.SimpleModule;
import inet.applications.contract.IApp;

simple QuicZeroRttClient extends SimpleModule like IApp
{
    parameters:
        @class(QuicZeroRttClient);
        string localAddress = default(""); // comma separated list of addresses
        int localPort = default(0); // port number to listen on
        string connectAddress;  // server address (may be symbolic)
        int connectPort; // port number to connect to
        bool invalidClientTokenString = default(false); // app provides a client token with an invalid format (empty string)
        bool sendInvalidToken = default(false); // app provides a client token with a valid format but a wrong token number
    gates:
        input socketIn;
        output socketOut;
}
