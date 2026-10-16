//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.tcp.flavours;

//
// NewReno (RFC 6582), with the SACK recovery of RFC 6675 when SACK is on.
//
simple TcpNewReno extends TcpAlgorithm
{
    parameters:
        @class(TcpNewReno);
        string slowStartExit @enum("none", "hystart") = default("none"); // the end of the first slow start: "none" = at ssthresh or at a congestion signal (RFC 5681); "hystart" = the hybrid slow start of Linux tcp_cubic, which only ~TcpCubic runs, so here the same as "none", as Linux reno (~LinuxTcp and ~LegacyTcp set it)
}
