//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for IEEE 802.11 contention-based channel access mechanisms. Implements 
// the core CSMA/CA algorithm with exponential backoff. Listens for DIFS/AIFS period 
// followed by a random backoff before transmission, defers when the channel is busy, 
// and uses EIFS after detecting corrupted frames. Provides the fundamental collision 
// avoidance functionality required for DCF and EDCA operation.
//
// @see ~Contention, ~Dcaf, ~Edcaf
//
moduleinterface IContention
{
    parameters:
        @display("i=block/timer");
}
