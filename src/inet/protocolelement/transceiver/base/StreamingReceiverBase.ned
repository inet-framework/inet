//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.transceiver.base;

//
// Extends the PacketReceiverBase with streaming capabilities. Monitors the
// transmission channel for changes and handles dynamic channel modifications
// during simulation. Designed for continuous reception of signals over
// potentially changing channel conditions.
//
simple StreamingReceiverBase extends PacketReceiverBase
{
    parameters:
        @class(StreamingReceiverBase);
}
