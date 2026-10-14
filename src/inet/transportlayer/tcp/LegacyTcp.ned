//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.tcp;

//
// The ~Tcp module with the defaults that ~Tcp had before it took the
// defaults of the RFCs. Select it to keep the defaults of older simulations:
//
//   **.tcp.typename = "LegacyTcp"
//
// It sets each parameter whose default ~Tcp changed since then; a change of
// a default of ~Tcp adds the old value here. It does not restore the behavior
// that INET repaired, so the results of an older simulation can still differ.
// The TCP module tests select it, because their packet dumps were written for
// these defaults.
//
// @see ~Tcp, ~LinuxTcp
//
simple LegacyTcp extends Tcp
{
    parameters:
        @class(Tcp);
        sackSupport = default(false);
        timestampSupport = default(false);
        windowScalingSupport = default(false);
        delayedAcksEnabled = default(false);
        limitedTransmitEnabled = default(false);
        advertisedWindow = default(14 * this.mss);
        mss = default(536);
        initialWindow = default("rfc2001");
        tcpAlgorithmClass = default("TcpReno");
        hystartEnabled = default(true);
        initialRto = default(3s);
        lossDetectionMode = default("dupthresh");
}
