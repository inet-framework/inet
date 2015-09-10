//
// Copyright (C) 2015 Irene Ruengeler
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.linklayer.tun;

import inet.linklayer.contract.ITunNic;

//  Implements a Tun interface that creates a shortcut between the linklayer and
//  the application.

simple TunInterface like ITunNic
{
    parameters:
        string interfaceTableModule;    // The path to the InterfaceTable module
        @display("i=block/tunnel");

        @signal[packetSentToLower](type=cPacket);
        @signal[packetReceivedFromLower](type=cPacket);
        @signal[packetSentToUpper](type=cPacket);
        @signal[packetReceivedFromUpper](type=cPacket);

    gates:
        input upperLayerIn;    // from upper layers
        output upperLayerOut;  // to upper layers
        input appIn;    // from application
        output appOut;  // to application
}
