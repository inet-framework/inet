//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.packetlevel;

//
// Module interface for caching various limits and constraints of the radio
// medium to optimize performance. Maintains information about maximum
// transmission power, minimum interference and reception power, maximum antenna
// gain, communication and interference ranges, and other parameters.
//
moduleinterface IMediumLimitCache
{
    parameters:
        @display("i=block/table2");
}

