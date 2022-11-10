//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.filter;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// This packet filter module drops packets that don't match the given packet
// label filter criteria. The criteria can use wildcards and boolean operators.
//
simple LabelFilter extends PacketFilterBase like IPacketFilter
{
    parameters:
        string labelFilter @mutable; // the label filter as a match expression (e.g. green or red)
        @class(LabelFilter);
}
