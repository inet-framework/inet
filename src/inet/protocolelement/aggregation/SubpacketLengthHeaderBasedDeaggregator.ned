//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.aggregation;

import inet.protocolelement.aggregation.base.DeaggregatorBase;
import inet.protocolelement.aggregation.contract.IPacketDeaggregator;

simple SubpacketLengthHeaderBasedDeaggregator extends DeaggregatorBase like IPacketDeaggregator
{
    parameters:
        @class(SubpacketLengthHeaderBasedDeaggregator);
}
