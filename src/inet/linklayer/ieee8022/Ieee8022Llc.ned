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
        @display("i=block/layer");
        @signal[packetDropped](type=inet::Packet);
    gates:
        input upperLayerIn @interface[inet::IIeee8022Llc] @interface[inet::queueing::IPassivePacketSink](protocol=ieee8022llc; service=request); // TODO accepts packets and socket commands with  @messageKinds(inet::SocketCommandCode) as well
        output upperLayerOut; // TODO sends packets and socket indications with @messageKinds(inet::SocketStatusInd) as well
        input lowerLayerIn @interface[inet::queueing::IPassivePacketSink](protocol=ieee8022llc; service=indication);
        output lowerLayerOut;
}

