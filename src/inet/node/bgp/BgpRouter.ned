//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.bgp;

import inet.node.inet.Router;

//
// IP router with BGPv4 and OSPFv4 support.
//
module BgpRouter extends Router
{
    parameters:
        hasBgp = true;
        hasOspf = true;
}

