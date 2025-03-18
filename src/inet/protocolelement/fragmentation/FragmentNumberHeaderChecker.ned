//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.fragmentation;

import inet.queueing.base.PacketFilterBase;

//
// TODO
//
simple FragmentNumberHeaderChecker extends PacketFilterBase
{
    parameters:
        string headerPosition @enum("front","back") = default("front");
        @class(FragmentNumberHeaderChecker);
        @display("i=block/checker");
}
