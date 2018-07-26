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

import inet.emulation.common.tap.TapBridge;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.contract.IExternalInterface;
import inet.physicallayer.ethernet.EtherPhy;

module ExtUpperEthernetInterface like IExternalInterface, IEthernetInterface
{
    parameters:
        string interfaceTableModule;
        int mtu @unit(B) = default(4470B);
        @class(ExtInterface);
        @display("i=block/ifcard");
    gates:
        input upperLayerIn @loose;
        output upperLayerOut @loose;
        inout phys @labels(EtherFrame);
    submodules:
        tapBridge: TapBridge {
            parameters:
                @display("p=100,100");
        }
        phy: EtherPhy {
            parameters:
                @display("p=100,300");
        }
    connections allowunconnected:
        tapBridge.lowerLayerOut --> { @display("m=s"); } --> phy.upperLayerIn;
        tapBridge.lowerLayerIn <-- { @display("m=s"); } <-- phy.upperLayerOut;
        phy.phys <--> { @display("m=s"); } <--> phys;
}
