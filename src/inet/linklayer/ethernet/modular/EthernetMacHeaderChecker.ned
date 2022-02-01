//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.protocolelement.contract.IProtocolHeaderChecker;
import inet.queueing.base.PacketFilterBase;

simple EthernetMacHeaderChecker extends PacketFilterBase like IProtocolHeaderChecker
{
    parameters:
        string interfaceTableModule;
        bool promiscuous = default(false);
        @class(EthernetMacHeaderChecker);
        @display("i=block/checker");
}
