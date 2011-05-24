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

package inet.underTest.wpan.linklayer.bmac;

//***************************************************************************
//* file:        BMacLayer.ned
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



simple BMacLayer
{
    parameters:
		@class(BMacLayer);
        // debug switch
        bool debug = default(false);
        bool stats = default(true);

		// BMAC specific parameters
		// how long is one slot?
		double slotDuration = default(1);
		
		// how long is the check interval (CCA)?
		double checkInterval = default(0.01);
		
		// size of the MAC queue (maximum number of packets in Tx buffer)
        double queueLength = default(20);
        
        // should we animate the nodes with colors depending on their state or not?
        bool animation = default(true);
        
        // should show bubbles with the status 
        bool animationBubble = default(true);
        // bit rate 
        double bitrate @unit(bps) = default(19200 bps);
        
        //tx power [mW]
        double txPower @unit(mW) = default(50 mW);
          
        bool useMACAcks = default(false);
        // maximum number of frame retransmission
        // only used when usage of MAC acks is enabled
        string address = default("auto");
        string queueModule= default("");				// name of optional external queue module
        
        double macMaxFrameRetries = default(3);
        
        bool useIeee802Ctrl=default(true);
    gates:  
        input uppergateIn; // from upper layer
        output uppergateOut; // to uppe layer
        input lowergateIn; // from lower layer
        output lowergateOut; // to lower layer    
}

