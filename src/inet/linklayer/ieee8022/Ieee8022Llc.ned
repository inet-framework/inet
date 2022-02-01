//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

simple Ieee8022Llc like IIeee8022Llc
{
    parameters:
        bool registerProtocol = default(false);     //FIXME //KLUDGE should redesign place of EthernetEncapsulation and LLC modules
        @display("i=block/layer");
        @signal[packetDropped](type=inet::Packet);
    gates:
        input upperLayerIn; // TODO accepts packets and socket commands with  @messageKinds(inet::SocketCommandCode), too
        output upperLayerOut; // TODO sends packets and socket indications with @messageKinds(inet::SocketStatusInd), too
        input lowerLayerIn;
        output lowerLayerOut;
}

