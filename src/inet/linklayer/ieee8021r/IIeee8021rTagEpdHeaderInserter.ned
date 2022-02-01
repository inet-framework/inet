//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021r;

import inet.protocolelement.contract.IProtocolHeaderInserter;

moduleinterface IIeee8021rTagEpdHeaderInserter extends IProtocolHeaderInserter
{
    parameters:
        @display("i=block/inserter");
}

