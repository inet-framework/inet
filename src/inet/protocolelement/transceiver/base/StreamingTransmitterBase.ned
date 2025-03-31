//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.transceiver.base;

//
// Extends the PacketTransmitterBase with streaming capabilities. Monitors the
// transmission channel for changes and handles dynamic channel modifications
// during simulation. Supports continuous transmission of signals with the ability
// to adapt to channel state changes, including disabling/enabling and cutting
// connections.
//
simple StreamingTransmitterBase extends PacketTransmitterBase
{
    parameters:
        @class(StreamingTransmitterBase);
}
