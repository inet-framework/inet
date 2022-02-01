//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.common;

import inet.linklayer.contract.IMacProtocol;
import inet.linklayer.contract.IWirelessInterface;
import inet.networklayer.common.NetworkInterface;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadio;
import inet.queueing.contract.IPacketQueue;


//
// This module implements a generic wireless network interface.
//
module WirelessInterface extends NetworkInterface like IWirelessInterface
{
    parameters:
        string interfaceTableModule;
        string energySourceModule;
        string protocol = default("");
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        *.energySourceModule = default(absPath(this.energySourceModule));
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input radioIn @labels(IWirelessSignal);
    submodules:
        classifier: <default("OmittedIeee8021dQosClassifier")> like IIeee8021dQosClassifier {
            parameters:
                @display("p=200,100");
        }
        queue: <default("OmittedPacketQueue")> like IPacketQueue {
            parameters:
                @display("p=200,200;q=l2queue");
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
        upperLayerIn --> { @display("m=n"); } --> classifier.in;
        classifier.out --> queue.in;
        queue.out --> mac.upperLayerIn;
        mac.lowerLayerOut --> radio.upperLayerIn if exists(radio);

        radioIn --> { @display("m=s"); } --> radio.radioIn if exists(radio);
        radio.upperLayerOut --> mac.lowerLayerIn if exists(radio);
        mac.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
}

