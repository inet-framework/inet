//
// Copyright (C) 2008-2010, Anna Foerster, NetLab, SUPSI, Switzerland.
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.bmac;

import inet.linklayer.base.MacProtocolBase;
import inet.linklayer.contract.IMacProtocol;


//
// Implementation of B-MAC (called also Berkeley MAC, Low Power Listening or
// LPL).
// See C++ documentation for details.
//
module BMac extends MacProtocolBase like IMacProtocol
{
    parameters:
        string address @mutable = default("auto"); // MAC address as hex string (12 hex digits), or
                                                   // "auto". "auto" values will be replaced by
                                                   // a generated MAC address in init stage 0.
        // BMAC specific parameters
        // how long is one slot?
        double slotDuration @unit(s) = default(0.1s);

        int headerLength @unit(b) = default(10b);
        int ctrlFrameLength @unit(b) = default(headerLength);
        int mtu @unit(B) = default(0B);

        // how long is the check interval (CCA)?
        double checkInterval @unit(s) = default(0.01s);

        // should we animate the nodes with colors depending on their state or
        // not?
        bool animation = default(true);

        // bit rate
        double bitrate @unit(bps) = default(19200 bps);

        //should mac send and expect acknowledgments?
        bool useMACAcks = default(false);

        string radioModule = default("^.radio");   // The path to the Radio module  //FIXME remove default value

        int maxTxAttempts = default(2);     //  Maximum transmission attempts per data packet, when ACKs are used

        @class(BMac);
        @signal[linkBroken](type=inet::Packet);
        @statistic[linkBroken](title="link break"; source=linkBroken; record=count; interpolationmode=none);
        @statistic[packetDropIncorrectlyReceived](title="packet drop: incorrectly received"; source=packetDropReasonIsIncorrectlyReceived(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropNotAddressedToUs](title="packet drop: not addressed to us"; source=packetDropReasonIsNotAddressedToUs(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropQueueOverflow](title="packet drop: queue overflow"; source=packetDropReasonIsQueueOverflow(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);

        @selfMessageKinds(inet::BMacType);
}

