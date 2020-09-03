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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

package inet.linklayer.tun;

import inet.linklayer.contract.ITunnelInterface;
import inet.networklayer.common.NetworkInterface;

//
// This module implements a TUN network interface.
//
module TunInterface extends NetworkInterface like ITunnelInterface
{
    parameters:
        string interfaceTableModule;
        int mtu @unit(B) = default(1500B);
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        @display("i=block/tunnel");
    gates:
        input upperLayerIn;
        output upperLayerOut;
    submodules:
        tun: Tun {
            parameters:
	            @display("p=300,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> tun.upperLayerIn;
        tun.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
}

