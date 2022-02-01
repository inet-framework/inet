//
// Copyright (C) 2005 Andras Babos
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.rip;

import inet.node.inet.Router;

//
// An RIPv2 router.
//
module RipRouter extends Router
{
    parameters:
        hasRip = true;
}

