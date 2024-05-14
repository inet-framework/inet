//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

import inet.linklayer.contract.IIeee8022Llc;

simple Ieee8022Llc like IIeee8022Llc
{
    parameters:
        bool registerProtocol = default(false);     //FIXME //KLUDGE should redesign the placement of EthernetEncapsulation and LLC modules
        @display("i=block/layer");
        @signal[packetDropped](type=inet::Packet);
    gates:
        input upperLayerIn; // TODO accepts packets and socket commands with  @messageKinds(inet::SocketCommandCode) as well
        output upperLayerOut; // TODO sends packets and socket indications with @messageKinds(inet::SocketStatusInd) as well
        input lowerLayerIn;
        output lowerLayerOut;
}

