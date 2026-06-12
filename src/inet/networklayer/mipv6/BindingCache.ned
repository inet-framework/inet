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
// Holds the Binding Cache of a home agent or correspondent node (RFC 3775): the
// bindings registered by mobile nodes, mapping each mobile node's home address to
// its current care-of address, together with the sequence number and binding
// lifetime. A home agent uses it to intercept and tunnel traffic to a mobile node
// that is away from home; a correspondent node uses it for route optimization
// (sending directly to the care-of address). It also keeps the return-routability
// nonces used to validate Binding Updates.
//
// This is a passive data-store module (no gates); it is read and updated by the
// ~Mipv6 protocol engine through C++ method calls.
//
// @see ~Mipv6, ~BindingUpdateList
//
simple BindingCache extends SimpleModule
{
    parameters:
        @class(BindingCache);
        @display("i=old/harddisk");
}

