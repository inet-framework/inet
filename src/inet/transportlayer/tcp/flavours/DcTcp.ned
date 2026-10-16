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
        double g = default(0.0625); // the estimation gain g of the fraction of marked bytes, alpha (RFC 8257 section 4.2)
}
