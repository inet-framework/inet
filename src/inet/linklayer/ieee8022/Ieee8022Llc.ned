//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8022;

import inet.common.SimpleModule;
import inet.linklayer.contract.IIeee8022Llc;

//
// Implements the IEEE 802.2 Logical Link Control (LLC) protocol. Provides
// protocol multiplexing at the link layer through Service Access Points (SAPs).
// Supports both standard LLC headers with DSAP/SSAP fields and SNAP (SubNetwork
// Access Protocol) headers for EtherType encapsulation. Enables direct socket-based
// access to LLC services for applications and handles protocol registration for
// higher layer protocols.
//
simple Ieee8022Llc extends SimpleModule like IIeee8022Llc
{
    parameters:
        @class(Ieee8022Llc);
        bool registerProtocol = default(false);     //FIXME //KLUDGE should redesign the placement of EthernetEncapsulation and LLC modules
        @display("i=block/layer");
        @signal[packetDropped](type=inet::Packet);
    gates:
        input upperLayerIn; // TODO accepts packets and socket commands with  @messageKinds(inet::SocketCommandCode) as well
        output upperLayerOut; // TODO sends packets and socket indications with @messageKinds(inet::SocketStatusInd) as well
        input lowerLayerIn;
        output lowerLayerOut;
}

