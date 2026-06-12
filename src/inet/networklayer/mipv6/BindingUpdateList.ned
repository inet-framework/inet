//
// Copyright (C) 2007
// Faqir Zarrar Yousaf
// Communication Networks Institute, Dortmund University of Technology (TU Dortmund), Germany.
// Christian Bauer
// Institute of Communications and Navigation, German Aerospace Center (DLR)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.networklayer.mipv6;

import inet.common.SimpleModule;

//
// Holds the Binding Update List of a mobile node (RFC 3775): one entry per home
// agent and correspondent node the node has sent a Binding Update to, recording
// the care-of address registered, sequence numbers, binding lifetimes, the
// return-routability state (home/care-of keygen tokens and nonce indices), and
// retransmission bookkeeping.
//
// This is a passive data-store module (no gates); it is read and updated by the
// ~Mipv6 protocol engine through C++ method calls.
//
// @see ~Mipv6, ~BindingCache
//
simple BindingUpdateList extends SimpleModule
{
    parameters:
        @class(BindingUpdateList);
        @display("i=old/harddisk");
}

