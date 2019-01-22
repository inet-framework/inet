//
// Copyright (C) 2006 Andras Varga
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

package inet.linklayer.common;

import inet.linklayer.contract.IMacProtocol;
import inet.linklayer.contract.IWirelessInterface;
import inet.networklayer.common.InterfaceEntry;
import inet.physicallayer.contract.packetlevel.IRadio;

module WirelessInterface extends InterfaceEntry like IWirelessInterface
{
    parameters:
        @class(InterfaceEntry);
        string interfaceTableModule;
        string energySourceModule;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.energySourceModule = default(absPath(energySourceModule));
        @display("i=block/ifcard");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input radioIn @labels(ISignal);
    submodules:
        classifier: <default("")> like IIeee8021dQosClassifier if typename != "" {
            @display("p=250,100");
        }
        mac: <> like IMacProtocol {
            @display("p=100,200");
        }
        radio: <> like IRadio if typename != "" {
            @display("p=100,350");
        }
    connections allowunconnected:
        radioIn --> { @display("m=s"); } --> radio.radioIn if exists(radio);
        radio.upperLayerIn <-- mac.lowerLayerOut if exists(radio);
        radio.upperLayerOut --> mac.lowerLayerIn if exists(radio);

        mac.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
        mac.upperLayerIn <-- { @display("m=n"); } <-- upperLayerIn if !exists(classifier);
        mac.upperLayerIn <-- { @display("m=n"); } <-- classifier.out if exists(classifier);
        classifier.in <-- { @display("m=n"); } <-- upperLayerIn if exists(classifier);
}

