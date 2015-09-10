//***************************************************************************
// * file:        Flood.ned
// *
// * author:      Daniel Willkomm
// *		  
// *
// * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
// *              Technische Universitaet Berlin, Germany.
// *
// *              This program is free software; you can redistribute it 
// *              and/or modify it under the terms of the GNU General Public 
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later 
// *              version.
// *              For further information see file COPYING 
// *              in the top level directory
// *
// ***************************************************************************
// * part of:     framework implementation developed by tkn
// * description: a simple flooding protocol
// *              the user can decide whether to use plain flooding or not
// ***************************************************************************

package inet.networklayer.flood;

import inet.networklayer.base.NetworkProtocolBase;
import inet.networklayer.contract.INetworkProtocol;

//
// A simple flooding protocol for network-level broadcast.
//
// This implementation uses plain flooding, i.e. it "remembers"
// (stores) already broadcasted messages in a list and does not
// rebroadcast them again, if it gets another copy of that message.
//
// The maximum number of entries for that list can be defined in the
// .ini file (@ref bcMaxEntries) as well as the time after which an entry
// is deleted (@ref bcDelTime).
//
// If you prefere a memory-less version you should set to false the 
// <verbatim>plainFlooding</verbatim> bool parameter.
//
// @author Daniel Willkomm
//
// ported to Mixim 2.0 by Theodoros Kapourniotis
//
simple Flood extends NetworkProtocolBase like INetworkProtocol
{
    parameters:
        @display("i=block/fork");
        int headerLength @unit(bit) = default(100b); // length of the network packet header (in bits)
        bool plainFlooding = default(true); // flag whether to use plain flooding
        // Max number of entries in the list of already broadcasted messages
        double bcMaxEntries = default(100);
        // Time after which an entry for an already broadcasted msg can be deleted
        double bcDelTime @unit(s) = default(100 s);
        double defaultTtl = default(10); // Default time-to-live (ttl) used for this module expressed in  number of hops.

        @class(Flood);
}
