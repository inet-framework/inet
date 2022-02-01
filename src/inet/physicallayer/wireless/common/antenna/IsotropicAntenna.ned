//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.antenna;

import inet.physicallayer.wireless.common.base.packetlevel.AntennaBase;

//
// This antenna model describes the theoretical point source which radiates the
// same intensity of radiation in all directions.
//
module IsotropicAntenna extends AntennaBase
{
    parameters:
        @class(IsotropicAntenna);
}
