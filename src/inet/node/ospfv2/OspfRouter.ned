//
// Copyright (C) 2005 Andras Babos
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.ospfv2;

import inet.node.inet.Router;

//
// An OSPFv2 router.
//
module OspfRouter extends Router
{
    parameters:
        hasOspf = true;
}

