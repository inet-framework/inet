//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.prototype.cellular;

import inet.common.Module;
import inet.physicallayer.wireless.common.contract.packetlevel.IAntenna;

//
// Stands in a radio's antenna slot for an antenna that several radios share, for example the
// panel that all component carriers of one band use. Every query goes to that antenna, so the
// radios share its position, its orientation and its beams. It works around the lack of shared
// antennas in ~Radio (change C3 of the cellular architecture study).
//
module SharedAntennaRef extends Module like IAntenna
{
    parameters:
        string antennaModule; // Path of the shared antenna, relative to this module
        @class(SharedAntennaRef);
        @display("i=device/antennatower;is=s");
}

