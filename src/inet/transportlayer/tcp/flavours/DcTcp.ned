//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.tcp.flavours;

//
// DCTCP (RFC 8257): the window follows the share of the ECN-marked bytes.
//
simple DcTcp extends TcpAlgorithm
{
    parameters:
        @class(DcTcp);
        string slowStartExit @enum("none", "hystart", "hystartpp") = default("none"); // the end of the first slow start: "none" = at ssthresh or at a congestion signal (RFC 5681); "hystart" = the hybrid slow start of Linux tcp_cubic, which only ~TcpCubic runs, so here the same as "none", as Linux reno (~LinuxTcp and ~LegacyTcp set it); "hystartpp" = HyStart++ (RFC 9406)
        double g = default(0.0625); // the estimation gain g of the fraction of marked bytes, alpha (RFC 8257 section 4.2)
}
