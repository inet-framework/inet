//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.inet;

module MulticastRouter extends Router
{
    parameters:
        hasPim = default(true);
        multicastForwarding = true;
}

