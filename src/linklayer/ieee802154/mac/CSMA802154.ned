//***************************************************************************
//* file:        CSMA802154.ned
//*
//* author:      Jerome Rousselot, Marc Loebbers
//* 
//* copyright:   (C) 2007-2009 CSEM SA, Neuchatel, Switzerland.
//*		 		 (C) 2004 Telecommunication Networks Group (TKN) at
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
package inet.underTest.wpan.linklayer.ieee802154.mac;

//
// IEEE 802.15.4-2006 non-beacon enabled CSMA protocol
//
simple CSMA802154
{
    parameters:
        @class(csma802154);
        bool debug = default(false);
        bool stats = default(true);
        bool trace = default(false);
        
        string address=default ("auto");
        string queueModule;				// name of optional external queue module
        bool useMACAcks = default(true);
        int queueLength = default(100);
        // simple interframe space. Time to wait between receiving a frame and acknowledging it.
        // Should be bigger than the maximum time for switching between Tx and Rx at the receiver.      
        double sifs @unit(s) = default(0.000192 s); // 12 symbols


        double macMaxCSMABackoffs = default(5);
        // maximum number of frame retransmission
        double macMaxFrameRetries = default(3);        
        // time to wait for an acknowledgement after transmitting a unicast frame.
        double macAckWaitDuration @unit(s) = default(0.00056 s);  // 1+12+10+12 symbols
        // Clear Channel Assessment detection time		
        double ccaDetectionTime @unit(s) = default(0.000128 s); // 8 symbols
        // Time to setup radio to reception state
        double rxSetupTime @unit(s);
        // Time to switch radio from Rx to Tx state
        double aTurnaroundTime @unit(s) = default(0.000192 s);    // 12 symbols
        double aUnitBackoffPeriod @unit(s) = default(0.00032 s);
        // complete MAC ack message length (in bits) 
        // (! headerLength is not added to this)
        double ackLength @unit(bit) = default(40 bit);        

        //backoff method to use. constant, linear or exponential
        string backoffMethod = default("exponential");
        // base unit for all backoff calculations
        // # of backoff periods of the initial contention window
        // (for linear and constant backoff method only)
        int contentionWindow = default(2);
        // minimum backoff exponent (for exponential backoff method only)
        double macMinBE = default(3);
        // maximum backoff exponent (for exponential backoff method only)
        double macMaxBE = default(8);
     gates:
        input uppergateIn; 			// from the upper layer
        output uppergateOut; 		// to the upper layer
        input lowergateIn; 			// from the physical layer
        output lowergateOut; 			// to the physical layer
        
}
