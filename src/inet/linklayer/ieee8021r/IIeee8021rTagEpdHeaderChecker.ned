//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021r;

import inet.protocolelement.contract.IProtocolHeaderChecker;

moduleinterface IIeee8021rTagEpdHeaderChecker extends IProtocolHeaderChecker
{
    parameters:
        @display("i=block/checker");
}

