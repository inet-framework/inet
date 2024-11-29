//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.authentication;

import inet.protocolelement.contract.IProtocolHeaderChecker;
import inet.queueing.base.PacketFilterBase;

simple MessageAuthenticationCodeChecker extends PacketFilterBase like IProtocolHeaderChecker
{
    parameters:
        int headerLength @unit(b) = default(4B);
        @class(MessageAuthenticationCodeChecker);
        @display("i=block/checker");
}
