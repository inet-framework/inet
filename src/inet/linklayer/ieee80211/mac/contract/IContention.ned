//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for modules that implement contention-based channel access.
// For each frame, Contention listens on the channel for a DIFS (AIFS) period
// then for a random backoff period before transitting the frame, and defers when
// busy channel is sensed. After receiving a corrupted frame, EIFS is used instead
// of the original DIFS (AIFS).
//
moduleinterface IContention
{
    parameters:
        @display("i=block/timer");
}

