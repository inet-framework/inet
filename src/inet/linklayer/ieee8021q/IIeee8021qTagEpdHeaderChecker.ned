//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.protocolelement.contract.IProtocolHeaderChecker;

moduleinterface IIeee8021qTagEpdHeaderChecker extends IProtocolHeaderChecker
{
    parameters:
        string vlanTagType @enum("c","s");
        @display("i=block/checker");
}

