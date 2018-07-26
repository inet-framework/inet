//
// Copyright (C) 2004 Andras Varga
//               2010 Zoltan Bojthe
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

package inet.emulation.linklayer.ethernet;

import inet.linklayer.contract.IExternalInterface;

module ExtLowerEthernetInterface like IExternalInterface
{
    parameters:
        string interfaceTableModule;
        int mtu @unit(B) = default(4470B);
        @class(ExtInterface);
        @display("i=block/ifcard");
    gates:
        input upperLayerIn;
        output upperLayerOut;
    submodules:
        socket: ExtEthernetDeviceSocket {
            parameters:
                @display("p=100,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> socket.upperLayerIn;
        upperLayerOut <-- { @display("m=n"); } <-- socket.upperLayerOut;
}
