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

import inet.linklayer.common.MACProtocolBase;
import inet.linklayer.contract.IMACProtocol;


//
// Implementation of B-MAC (called also Berkeley MAC, Low Power Listening or 
// LPL).
// See C++ documentation for details.
//
simple BMacLayer extends MACProtocolBase like IMACProtocol
{
    parameters:
        string address = default("auto"); // MAC address as hex string (12 hex digits), or
                                          // "auto". "auto" values will be replaced by
                                          // a generated MAC address in init stage 0.
        // BMAC specific parameters
        // how long is one slot?
        double slotDuration @unit(s) = default(0.1s);

        int headerLength @unit(B) = default(10B);
        int mtu @unit("B") = default(0B);

        // how long is the check interval (CCA)?
        double checkInterval @unit(s) = default(0.01s);

		// size of the MAC queue (maximum number of packets in Tx buffer)
        int queueLength = default(20);

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

        @class(BMacLayer);
}
