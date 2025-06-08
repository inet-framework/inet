//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021as;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// This module classifies packets based on the domain number in the gPTP header
//
simple GptpDomainClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        @class(GptpDomainClassifier);
}
