//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for processes that unconditionally transmit a frame
// after waiting for a specified inter-frame space (usually SIFS). Such
// processes can be used to transmit frames where no contention is needed,
// e.g. ACK, CTS, or the second and further frames of a TXOP.
//
moduleinterface ITx
{
    parameters:
        @display("i=block/tx");
}

