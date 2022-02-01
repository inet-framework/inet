//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.packetlevel;

//
// The antenna model describes the physical device (a part of the radio) which
// converts electric signals into radio waves, and vice versa.
//
moduleinterface IAntenna
{
    parameters:
        @display("i=device/antennatower");
}

