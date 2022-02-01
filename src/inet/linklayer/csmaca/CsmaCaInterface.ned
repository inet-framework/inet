//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.csmaca;

import inet.linklayer.common.WirelessInterface;

//
// Module interface for CSMA/MA network interfaces.
//
module CsmaCaInterface extends WirelessInterface
{
    parameters:
        radio.typename = default("ApskScalarRadio");
        mac.typename = default("CsmaCaMac");
        queue.typename = default("DropTailQueue");
        queue.packetCapacity = default(-1);
}

