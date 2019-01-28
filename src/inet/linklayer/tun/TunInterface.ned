//
// Copyright (C) 2015 Irene Ruengeler
// Copyright (C) 2018 OpenSim Ltd.
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

import inet.linklayer.contract.ITunnelInterface;
import inet.networklayer.common.InterfaceEntry;

module TunInterface extends InterfaceEntry like ITunnelInterface
{
    parameters:
        @class(InterfaceEntry);
        string interfaceTableModule;    // The path to the InterfaceTable module
        int mtu @unit(B) = default(1500B);
        @display("i=block/tunnel");
        *.interfaceTableModule = default(absPath(interfaceTableModule));
    gates:
        input upperLayerIn;    // from upper layers
        output upperLayerOut;  // to upper layers
    submodules:
        tun: Tun {
            @display("p=100,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> tun.upperLayerIn;
        upperLayerOut <-- { @display("m=n"); } <-- tun.upperLayerOut;
}

