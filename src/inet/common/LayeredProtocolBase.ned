//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.common;

import inet.common.SimpleModule;

//
// Module base for different layered protocols.
//
simple LayeredProtocolBase extends SimpleModule
{
    parameters:
        @class(abstract);   //# set to 'abstract' to prevent unintentional instantiation and also suppress(missing-cpp-class) in linter
        @signal[packetSentToUpper](type=cPacket);
        @signal[packetReceivedFromUpper](type=cPacket);
        @signal[packetSentToLower](type=cPacket);
        @signal[packetReceivedFromLower](type=cPacket);
        @signal[packetDropped](type=cPacket);
}

