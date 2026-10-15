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
}
