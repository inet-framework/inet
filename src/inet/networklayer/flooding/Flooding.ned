//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//***************************************************************************
// * file:        Flooding.ned
// *
// * author:      Daniel Willkomm
// *
// *
// * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
// *              Technische Universitaet Berlin, Germany.
// *
// *
// ***************************************************************************
// * part of:     framework implementation developed by tkn
// * description: a simple flooding protocol
// *              the user can decide whether to use plain flooding or not
// ***************************************************************************

package inet.networklayer.flooding;

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
// .ini file (bcMaxEntries parameter) as well as the time after which an entry
// is deleted (bcDelTime parameter).
//
// If you prefere a memory-less version you should set to false the
// plainFlooding bool parameter.
//
// @author Daniel Willkomm
//
// ported to Mixim 2.0 by Theodoros Kapourniotis
//
simple Flooding extends NetworkProtocolBase like INetworkProtocol
{
    parameters:
        @display("i=block/fork");
        int headerLength @unit(b) = default(96b); // length of the network packet header (in bits)
        bool plainFlooding = default(true); // flag whether to use plain flooding
        // Max number of entries in the list of already broadcasted messages
        int bcMaxEntries = default(100);
        // Time after which an entry for an already broadcasted msg can be deleted
        double bcDelTime @unit(s) = default(100 s);
        int defaultTtl = default(10); // Default time-to-live (ttl) used for this module expressed in  number of hops.
        @class(Flooding);
}

