//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.linklayer.ieee802;

moduleinterface IIeee802Epd
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}

simple Ieee802Epd like IIeee802Epd
{
    parameters:
        bool registerProtocol = default(false);     //FIXME //KLUDGE should redesign place of EthernetEncapsulation and LLC modules
        @display("i=block/layer");
        @signal[packetDropped](type=Packet);
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}

