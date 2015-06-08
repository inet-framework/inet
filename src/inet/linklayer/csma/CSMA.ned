//***************************************************************************
//* file:        CSMA.ned
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

package inet.linklayer.csma;

import inet.linklayer.base.MACProtocolBase;
import inet.linklayer.contract.IMACProtocol;

//
// Generic CSMA protocol supporting Mac-ACKs as well as
// constant, linear or exponential backoff times.
//
simple CSMA extends MACProtocolBase like IMACProtocol
{
    parameters:
        string address = default("auto"); // MAC address as hex string (12 hex digits), or
                                          // "auto". "auto" values will be replaced by
                                          // a generated MAC address in init stage 0.

        // length of MAC header
        int headerLength @unit(bit) = default(72 bit);
        int mtu @unit("B") = default(0B);
        // size of the MAC queue (maximum number of packets in Tx buffer)
        int queueLength = default(100);
        // bit rate
        double bitrate @unit(bps) = default(250000 bps);

        // Clear Channel Assessment detection time
        double ccaDetectionTime @unit(s) = default(0.000128 s); // 8 symbols
        // Time to setup radio to reception state
        double rxSetupTime @unit(s) = default(0 s);
        // Time to switch radio from Rx to Tx state
        double aTurnaroundTime @unit(s) = default(0.000192 s);    // 12 symbols

        // Send/Expect MAC acks for unicast traffic?
        bool useMACAcks;
        // Maximum number of frame retransmission,
        // only used when usage of MAC acks is enabled.
        int macMaxFrameRetries = default(3);
        // Time to wait for an acknowledgement after transmitting a unicast frame.
        // Only used when usage of MAC acks is enabled.
        // Value is calculated from 1+12+10+12 symbols, which is defined for nonbeacon-enabled PAN.
        // In the non-beacon-enabled case, the receiver responds at aTurnaroundTime
        // (i.e. the time for the sender and receiver to both be guaranteed to have
        // switched from Tx to Rx and vice verse).  This gives the value 192us +
        // 352us = 544us (there's been some discussion about the "extra" 1 symbol == 16us)
        // [section 7.5.6.4.2 of the specification].
        double macAckWaitDuration @unit(s) = default(0.00056 s);
        // Complete MAC ack message length (in bits)
        // (! headerLength is not added to this),
        // only used when usage of MAC acks is enabled.
        double ackLength @unit(bit) = default(40 bit);
        // Simple interframe space (12 symbols). Time to wait between receiving a frame and acknowledging it.
        // Should be bigger than the maximum time for switching between Tx and Rx at the receiver.
        // Only used when usage of MAC acks is enabled.
        double sifs @unit(s) = default(0.000192 s);

        //Backoff method to use: constant, linear or exponential
        string backoffMethod = default("linear");
        // maximum number of extra backoffs (excluding the first unconditional one) before frame drop
        int macMaxCSMABackoffs = default(5);
        // base unit for all backoff calculations
        double aUnitBackoffPeriod @unit(s) = default(0.00032 s);
        // # of backoff periods of the initial contention window
        // (for linear and constant backoff method only)
        int contentionWindow = default(2);
        // minimum backoff exponent (for exponential backoff method only)
        double macMinBE = default(3);
        // maximum backoff exponent (for exponential backoff method only)
        double macMaxBE = default(8);

        string radioModule = default("^.radio");   // The path to the Radio module  //FIXME remove default value

        @class(CSMA);
        @signal[NF_LINK_BREAK](type=CSMAFrame);
}
