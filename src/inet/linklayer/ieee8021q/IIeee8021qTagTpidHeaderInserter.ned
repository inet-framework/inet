//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.protocolelement.contract.IProtocolHeaderInserter;

moduleinterface IIeee8021qTagTpidHeaderInserter extends IProtocolHeaderInserter
{
    parameters:
        string vlanTagType @enum("c","s");
        @display("i=block/inserter");
}

