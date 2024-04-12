//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.ieee802;

import inet.common.MessageDispatcher;
import inet.linklayer.contract.IEthernetLayer;
import inet.linklayer.contract.IIeee8021qLayer;
import inet.linklayer.contract.IIeee8021rLayer;
import inet.linklayer.contract.IIeee8022Llc;
import inet.linklayer.ieee80211.IIeee80211Mac;
import inet.protocolelement.contract.IProtocolLayer;

module Ieee802Layer like IProtocolLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        md1: MessageDispatcher {
            @display("p=300,100;b=400,5");
        }
        ieee8022: <default(Ieee8022Llc)> like IIeee8022Llc {
            @display("p=200,200");
        }
        md2: MessageDispatcher {
            @display("p=300,300;b=400,5");
        }
        ieee8021d: <default(BridgingLayer)> like IProtocolLayer {
            @display("p=300,400");
        }
        md3: MessageDispatcher {
            @display("p=300,500;b=400,5");
        }
        ieee8021q: <default(Ieee8021qLayer)> like IIeee8021qLayer {
            @display("p=200,600");
        }
        ieee8021r: <default(Ieee8021rLayer)> like IIeee8021rLayer {
            @display("p=400,600");
        }
        md4: MessageDispatcher {
            @display("p=300,700;b=400,5");
        }
        ieee8023: <default(EthernetLayer)> like IEthernetLayer {
            @display("p=200,800");
        }
        ieee80211: <default(Ieee80211Mac)> like IIeee80211Mac {
            @display("p=400,800");
        }
        md5: MessageDispatcher {
            @display("p=300,900;b=400,5");
        }
    connections:
        upperLayerIn --> md1.in++;
        md1.out++ --> upperLayerOut;

        md1.out++ --> ieee8022.upperLayerIn;
        ieee8022.upperLayerOut --> md1.in++;

        ieee8022.lowerLayerOut --> md2.in++;
        md2.out++ --> ieee8022.lowerLayerIn;

        md1.out++ --> md2.in++;
        md2.out++ --> md1.in++;

        md2.out++ --> ieee8021d.upperLayerIn;
        ieee8021d.upperLayerOut --> md2.in++;

        ieee8021d.lowerLayerOut --> md3.in++;
        md3.out++ --> ieee8021d.lowerLayerIn;

        md3.out++ --> md4.in++;
        md4.out++ --> md3.in++;

        md3.out++ --> ieee8021q.upperLayerIn;
        ieee8021q.upperLayerOut --> md3.in++;

        md3.out++ --> ieee8021r.upperLayerIn;
        ieee8021r.upperLayerOut --> md3.in++;

        ieee8021q.lowerLayerOut --> md4.in++;
        md4.out++ --> ieee8021q.lowerLayerIn;

        ieee8021r.lowerLayerOut --> md4.in++;
        md4.out++ --> ieee8021r.lowerLayerIn;

        md4.out++ --> ieee8023.upperLayerIn;
        ieee8023.upperLayerOut --> md4.in++;

        md4.out++ --> ieee80211.upperLayerIn;
        ieee80211.lowerLayerOut --> md4.in++;

        ieee8023.lowerLayerOut --> md5.in++;
        md5.out++ --> ieee8023.lowerLayerIn;

        ieee80211.lowerLayerOut --> md5.in++;
        md5.out++ --> ieee80211.lowerLayerIn;

        md5.out++ --> lowerLayerOut;
        lowerLayerIn --> md5.in++;
}
