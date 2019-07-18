//***************************************************************************
//* file:        Ieee802154Mac.ned
//*
//* author:      Jerome Rousselot, Marc Loebbers
//*
//* copyright:   (C) 2007-2009 CSEM SA, Neuchatel, Switzerland.
//*                  (C) 2004 Telecommunication Networks Group (TKN) at
//*              Technische Universitaet Berlin, Germany.
//*
//*              This program is free software; you can redistribute it
//*              and/or modify it under the terms of the GNU General Public
//*              License as published by the Free Software Foundation; either
//*              version 2 of the License, or (at your option) any later
//*              version.
//*              For further information see file COPYING
//*              in the top level directory
//*
//* Funding: This work was partially financed by the European Commission under the
//* Framework 6 IST Project "Wirelessly Accessible Sensor Populations"
//* (WASP) under contract IST-034963.
//***************************************************************************
//* part of:    Modifications to the MF-2 framework by CSEM
//**************************************************************************/

package inet.linklayer.ieee802154Loss;

import inet.common.queueing.contract.IPacketQueue;
import inet.linklayer.base.MacProtocolBase;
import inet.linklayer.contract.IMacProtocol;
import inet.linklayer.ieee802154.Ieee802154Mac;


//
// Generic CSMA protocol supporting Mac-ACKs as well as
// constant, linear or exponential backoff times.
//
module Ieee802154MacLoss extends Ieee802154Mac
{
    parameters:
 
        double probabilityCompleteFailureLink = default(0); // (between 0..1) percentage of links that will fail in every node        
        double probabilityLink = default(0); // (between 0..1) percentage of links that will lost packet in every node, after set the failure links
        double probabilityFailurePacket = default(0); // (between 0..1) probability of loss in link with loss.

        @class(Ieee802154MacLoss);
        @signal[linkBroken](type=inet::Packet);
        @statistic[linkBroken](title="link break"; source=linkBroken; record=count; interpolationmode=none);
        @statistic[packetDropNotAddressToUs](title="packet drop: not addressed to us"; source=packetDropReasonIsNotAddressedToUs(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropIncorrectlyReceived](title="packet drop: incorrectly received"; source=packetDropReasonIsIncorrectlyReceived(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropQueueOverlow](title="packet drop: queue overflow"; source=packetDropReasonIsQueueOverflow(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropRetryLimitReached](title="packet drop: retry limit reached"; source=packetDropReasonIsRetryLimitReached(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropBackoffLimitReached](title="packet drop: backoff limit reached"; source=packetDropReasonIsCongestion(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
    connections allowunconnected:
}

