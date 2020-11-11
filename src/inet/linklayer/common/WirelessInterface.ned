//
// Copyright (C) 2006 OpenSim Ltd.
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

package inet.linklayer.common;

import inet.linklayer.contract.IMacProtocol;
import inet.linklayer.contract.IWirelessInterface;
import inet.networklayer.common.NetworkInterface;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadio;

//
// This module implements a generic wireless network interface.
//
module WirelessInterface extends NetworkInterface like IWirelessInterface
{
    parameters:
        string interfaceTableModule;
        string energySourceModule;
        string protocol = default("");
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.energySourceModule = default(absPath(energySourceModule));
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input radioIn @labels(IWirelessSignal);
    submodules:
        classifier: <default("")> like IIeee8021dQosClassifier if typename != "" {
            parameters:
	            @display("p=500,100");
        }
        mac: <> like IMacProtocol {
            parameters:
	            @display("p=300,200");
        }
        radio: <default("")> like IRadio if typename != "" {
            parameters:
	            @display("p=300,300");
        }
    connections allowunconnected:
        upperLayerIn --> { @display("m=n"); } --> mac.upperLayerIn if !exists(classifier);
        upperLayerIn --> { @display("m=n"); } --> classifier.in if exists(classifier);
        classifier.out --> mac.upperLayerIn if exists(classifier);
        mac.lowerLayerOut --> radio.upperLayerIn if exists(radio);

        radioIn --> { @display("m=s"); } --> radio.radioIn if exists(radio);
        radio.upperLayerOut --> mac.lowerLayerIn if exists(radio);
        mac.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
}

