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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.linklayer.bmac;

//***************************************************************************
//* file:        BMac.ned
//*
//* author:      Anna Foerster
//* 
//* copyright:   (C) 22008- 2010, NetLab, SUPSI, Switzerland.
//*
//*              This program is free software; you can redistribute it
//*              and/or modify it under the terms of the GNU General Public
//*              License as published by the Free Software Foundation; either
//*              version 2 of the License, or (at your option) any later
//*              version.
//*              For further information see file COPYING
//*              in the top level directory
//*
//**************************************************************************/

import inet.queueing.contract.IPacketQueue;
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
        string address = default("auto"); // MAC address as hex string (12 hex digits), or
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
        // maximum number of frame retransmission
        // only used when usage of MAC acks is enabled
        int macMaxFrameRetries = default(3);

        string radioModule = default("^.radio");   // The path to the Radio module  //FIXME remove default value

        int maxTxAttempts = default(2);     //  Maximum transmission attempts per data packet, when ACKs are used

        @class(BMac);
        @signal[linkBroken](type=inet::Packet);
        @statistic[linkBroken](title="link break"; source=linkBroken; record=count; interpolationmode=none);
        @statistic[packetDropIncorrectlyReceived](title="packet drop: incorrectly received"; source=packetDropReasonIsIncorrectlyReceived(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropNotAddressedToUs](title="packet drop: not addressed to us"; source=packetDropReasonIsNotAddressedToUs(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropQueueOverflow](title="packet drop: queue overflow"; source=packetDropReasonIsQueueOverflow(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);

        @selfMessageKinds(inet::BMacType);
    submodules:
        queue: <default("DropTailQueue")> like IPacketQueue {
            parameters:
                packetCapacity = default(20);
                @display("p=100,100;q=l2queue");
        }
}

