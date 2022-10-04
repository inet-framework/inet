//
//  LMac.ned
//  LMAC for MF 2.02, omnetpp 4
//
//  Created by Anna Foerster on 10/10/08.
//  Copyright 2008 Universita della Svizzera Italiana. All rights reserved.
//
//  Coverted to OMNeT++ 4 by Rudolf Hornig
//
//  Converted to MiXiM by Kapourniotis Theodoros
//

package inet.linklayer.lmac;

import inet.linklayer.base.MacProtocolBase;
import inet.linklayer.contract.IMacProtocol;


//
// Implementation of L-MAC (Lightweight Medium Access Protocol for
// Wireless Sensor Networks).
//
// See C++ documentation for details.
//
module LMac extends MacProtocolBase like IMacProtocol
{
    parameters:
        string address @mutable = default("auto"); // MAC address as hex string (12 hex digits), or
                                                   // "auto". "auto" values will be replaced by
                                                   // a generated MAC address in init stage 0.
        // how long is a slot?
        double slotDuration @unit(s) = default(100ms);
        int headerLength @unit(b) = default(10b);
        int ctrlFrameLength @unit(b) = default(headerLength);
        int mtu @unit(B) = default(0B);
        // default channel
        double defaultChannel = default(0);
        // bit rate
        double bitrate @unit(bps) = default(19200bps);
        // level at which we consider medium busy FIXME not used currently
//        double busyRSSI;

        int reservedMobileSlots = default(2);
        int numSlots = default(64);
        string radioModule = default("^.radio");   // The path to the Radio module  //FIXME remove default value

        @class(LMac);
        @statistic[packetDropIncorrectlyReceived](title="packet drop: incorrectly received"; source=packetDropReasonIsIncorrectlyReceived(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropNotAddressedToUs](title="packet drop: not addressed to us"; source=packetDropReasonIsNotAddressedToUs(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @selfMessageKinds(inet::LMacType);
}

